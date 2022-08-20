/**
 * @file
 * @date 2022/4/16
 * @author 9chu
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#include <lstg/Core/Subsystem/Render/Drawing2D/CommandExecutor.hpp>

#include <lstg/Core/Logging.hpp>
#include <lstg/Core/Subsystem/RenderSystem.hpp>

using namespace std;
using namespace lstg;
using namespace lstg::Subsystem::Render::Drawing2D;

static const char* kDefault2DEffect = R"EFFECT(
local vs, ps
do
    -- External CBuffer
    local cbCameraState = importConstantBuffer "_CameraState"

    -- CBuffer
    local cbFogState = constantBuffer "FogState"
        :scalar("FogType", ScalarTypes.UINT)
        :scalar("FogColorRGBA32", ScalarTypes.UINT)
        :scalar("FogArg1", ScalarTypes.FLOAT)
        :scalar("FogArg2", ScalarTypes.FLOAT)
        :build()

    -- Vertex Layout
    local vl2d = vertexLayout()
        :slot(0, SemanticNames.POSITION, 0)  -- vec3
        :slot(1, SemanticNames.TEXCOORD, 0)  -- vec2
        :slot(2, SemanticNames.COLOR, 0, true)  -- uint8_t[4] -> vec4
        :slot(3, SemanticNames.COLOR, 1, true)  -- uint8_t[4] -> vec4
        :build()

    -- Texture
    local texMainTexture = texture2d "MainTexture"
        :build()

    -- Vertex Shader
    vs = vertexShader [=[
        struct VSInput
        {
            float3 Position: ATTRIB0;  // POSITION0
            float2 UV: ATTRIB1;  // TEXCOORD0
            float4 AdditiveColor: ATTRIB2;  // COLOR0
            float4 MultiplierColor: ATTRIB3;  // COLOR1
        };

        struct VSOutput
        {
            float4 Position: SV_POSITION;
            float2 UV: TEXCOORD0;
            float FogDepth: FOGDISTANCE;
            float4 AdditiveColor: COLOR0;
            float4 MultiplierColor: COLOR1;
        };

        void main(in VSInput inVert, out VSOutput outVert)
        {
            float4 cameraViewPos = mul(_CameraViewMatrix, float4(inVert.Position.xyz, 1.0));
            outVert.Position = mul(_CameraProjectViewMatrix, float4(inVert.Position.xyz, 1.0));
            outVert.UV = inVert.UV;
            outVert.FogDepth = cameraViewPos.z;
            outVert.AdditiveColor = inVert.AdditiveColor;
            outVert.MultiplierColor = inVert.MultiplierColor;
        }
    ]=]
        :name("Default 2D Vertex Shader")
        :entry("main")
        :use(cbCameraState)
        :vertexLayout(vl2d)
        :build()

    -- Pixel Shader
    ps = pixelShader [=[
        #define FOG_DISABLED 0x00000000u
        #define FOG_LINEAR   0x00000001u
        #define FOG_EXP1     0x00000002u
        #define FOG_EXP2     0x00000003u

        struct PSInput
        {
            float4 Position: SV_POSITION;
            float2 UV: TEXCOORD0;
            float FogDepth: FOGDISTANCE;
            float4 AdditiveColor: COLOR0;
            float4 MultiplierColor: COLOR1;
        };

        float4 UnpackRGBA32(uint rgba32)
        {
            return float4(
                float((rgba32 & 0xFF000000u) >> 24u) / 255.f,
                float((rgba32 & 0x00FF0000u) >> 16u) / 255.f,
                float((rgba32 & 0x0000FF00u) >> 8u) / 255.f,
                float(rgba32 & 0x000000FFu) / 255.f);
        }

        float4 main(in PSInput outVert) : SV_Target
        {
            // 过去实现中，顶点颜色的 Alpha 通道可以被用于控制物体的透明度，并不参与乘算和加算的表现
            // 在当前可编程管线上，我们总是将两种颜色乘起来，使得原有的逻辑保持一致
            float4 texRGBA = MainTexture.Sample(MainTextureSampler, outVert.UV);
            float3 blendColor = min(texRGBA.xyz * outVert.MultiplierColor.xyz + outVert.AdditiveColor.xyz, float3(1, 1, 1));
            float blendAlpha = texRGBA.w * outVert.MultiplierColor.w * outVert.AdditiveColor.w;

            // 模拟 D3D9 固管的逐像素雾
            // https://docs.microsoft.com/en-us/windows/win32/direct3d9/fog-formulas
            // https://github.com/walbourn/directx-sdk-samples/blob/main/FixedFuncEMUFX11/FixedFuncEMU.fx
            float f = 1.f;
            float4 fogRGBA = UnpackRGBA32(FogColorRGBA32);
            if (FogType == FOG_LINEAR)
                f = (FogArg2 - outVert.FogDepth) / (FogArg2 - FogArg1);
            else if (FogType == FOG_EXP1)
                f = 1.f / exp(outVert.FogDepth * FogArg1);
            else if (FogType == FOG_EXP2)
                f = 1.f / exp(pow(outVert.FogDepth * FogArg1, 2.f));
            f = clamp(f, 0.f, 1.f);

            // 最终颜色
            float3 finalColor = f * blendColor + (1.f - f) * fogRGBA.xyz;
            return float4(finalColor, blendAlpha);
        }
    ]=]
        :name("Default 2D Pixel Shader")
        :entry("main")
        :use(cbFogState)
        :use(texMainTexture)
        :build()
end

-- PassGroup: Alpha Blend
local passGroupAlphaBlend
do
    local pass0 = pass "pass0"
        :blendState(BlendStates.ENABLE, 1)
        :blendState(BlendStates.SOURCE_BLEND, BlendFactors.SOURCE_ALPHA)
        :blendState(BlendStates.DEST_BLEND, BlendFactors.INVERT_SOURCE_ALPHA)
        :blendState(BlendStates.BLEND_OPERATION, BlendOperations.ADD)
        :blendState(BlendStates.SOURCE_ALPHA_BLEND, BlendFactors.SOURCE_ALPHA)
        :blendState(BlendStates.DEST_ALPHA_BLEND, BlendFactors.INVERT_SOURCE_ALPHA)
        :blendState(BlendStates.ALPHA_BLEND_OPERATION, BlendOperations.ADD)
        :rasterizerState(RasterizerStates.CULL_MODE, CullModes.NONE)
        :depthStencilState(DepthStencilStates.DEPTH_ENABLE, 1)
        :depthStencilState(DepthStencilStates.DEPTH_WRITE_ENABLE, 1)
        :depthStencilState(DepthStencilStates.DEPTH_FUNCTION, ComparisionFunctions.LESS_EQUAL)
        :vertexShader(vs)
        :pixelShader(ps)
        :build()
    passGroupAlphaBlend = passGroup "AlphaBlend"
        :tag("Blend", "Alpha")
        :pass(pass0)
        :build()
end

-- PassGroup: Add Blend
local passGroupAddBlend
do
    local pass0 = pass "pass0"
        :blendState(BlendStates.ENABLE, 1)
        :blendState(BlendStates.SOURCE_BLEND, BlendFactors.SOURCE_ALPHA)
        :blendState(BlendStates.DEST_BLEND, BlendFactors.ONE)
        :blendState(BlendStates.BLEND_OPERATION, BlendOperations.ADD)
        :blendState(BlendStates.SOURCE_ALPHA_BLEND, BlendFactors.SOURCE_ALPHA)
        :blendState(BlendStates.DEST_ALPHA_BLEND, BlendFactors.INVERT_SOURCE_ALPHA)
        :blendState(BlendStates.ALPHA_BLEND_OPERATION, BlendOperations.ADD)
        :rasterizerState(RasterizerStates.CULL_MODE, CullModes.NONE)
        :depthStencilState(DepthStencilStates.DEPTH_ENABLE, 1)
        :depthStencilState(DepthStencilStates.DEPTH_WRITE_ENABLE, 1)
        :depthStencilState(DepthStencilStates.DEPTH_FUNCTION, ComparisionFunctions.LESS_EQUAL)
        :vertexShader(vs)
        :pixelShader(ps)
        :build()
    passGroupAddBlend = passGroup "AddBlend"
        :tag("Blend", "Add")
        :pass(pass0)
        :build()
end

-- PassGroup: Subtract Blend
local passGroupSubtractBlend
do
    local pass0 = pass "pass0"
        :blendState(BlendStates.ENABLE, 1)
        :blendState(BlendStates.SOURCE_BLEND, BlendFactors.SOURCE_ALPHA)
        :blendState(BlendStates.DEST_BLEND, BlendFactors.ONE)
        :blendState(BlendStates.BLEND_OPERATION, BlendOperations.SUBTRACT)
        :blendState(BlendStates.SOURCE_ALPHA_BLEND, BlendFactors.SOURCE_ALPHA)
        :blendState(BlendStates.DEST_ALPHA_BLEND, BlendFactors.INVERT_SOURCE_ALPHA)
        :blendState(BlendStates.ALPHA_BLEND_OPERATION, BlendOperations.ADD)
        :rasterizerState(RasterizerStates.CULL_MODE, CullModes.NONE)
        :depthStencilState(DepthStencilStates.DEPTH_ENABLE, 1)
        :depthStencilState(DepthStencilStates.DEPTH_WRITE_ENABLE, 1)
        :depthStencilState(DepthStencilStates.DEPTH_FUNCTION, ComparisionFunctions.LESS_EQUAL)
        :vertexShader(vs)
        :pixelShader(ps)
        :build()
    passGroupSubtractBlend = passGroup "SubtractBlend"
        :tag("Blend", "Subtract")
        :pass(pass0)
        :build()
end

-- PassGroup: Revert Subtract Blend
local passGroupRevertSubtractBlend
do
    local pass0 = pass "pass0"
        :blendState(BlendStates.ENABLE, 1)
        :blendState(BlendStates.SOURCE_BLEND, BlendFactors.SOURCE_ALPHA)
        :blendState(BlendStates.DEST_BLEND, BlendFactors.ONE)
        :blendState(BlendStates.BLEND_OPERATION, BlendOperations.REVERT_SUBTRACT)
        :blendState(BlendStates.SOURCE_ALPHA_BLEND, BlendFactors.SOURCE_ALPHA)
        :blendState(BlendStates.DEST_ALPHA_BLEND, BlendFactors.INVERT_SOURCE_ALPHA)
        :blendState(BlendStates.ALPHA_BLEND_OPERATION, BlendOperations.ADD)
        :rasterizerState(RasterizerStates.CULL_MODE, CullModes.NONE)
        :depthStencilState(DepthStencilStates.DEPTH_ENABLE, 1)
        :depthStencilState(DepthStencilStates.DEPTH_WRITE_ENABLE, 1)
        :depthStencilState(DepthStencilStates.DEPTH_FUNCTION, ComparisionFunctions.LESS_EQUAL)
        :vertexShader(vs)
        :pixelShader(ps)
        :build()
    passGroupRevertSubtractBlend = passGroup "RevertSubtractBlend"
        :tag("Blend", "RevertSubtract")
        :pass(pass0)
        :build()
end

-- PassGroup: Alpha Blend No Depth
local passGroupAlphaBlendNoDepth
do
    local pass0 = pass "pass0"
        :blendState(BlendStates.ENABLE, 1)
        :blendState(BlendStates.SOURCE_BLEND, BlendFactors.SOURCE_ALPHA)
        :blendState(BlendStates.DEST_BLEND, BlendFactors.INVERT_SOURCE_ALPHA)
        :blendState(BlendStates.BLEND_OPERATION, BlendOperations.ADD)
        :blendState(BlendStates.SOURCE_ALPHA_BLEND, BlendFactors.SOURCE_ALPHA)
        :blendState(BlendStates.DEST_ALPHA_BLEND, BlendFactors.INVERT_SOURCE_ALPHA)
        :blendState(BlendStates.ALPHA_BLEND_OPERATION, BlendOperations.ADD)
        :rasterizerState(RasterizerStates.CULL_MODE, CullModes.NONE)
        :depthStencilState(DepthStencilStates.DEPTH_ENABLE, 0)
        :vertexShader(vs)
        :pixelShader(ps)
        :build()
    passGroupAlphaBlendNoDepth = passGroup "AlphaBlendNoDepth"
        :tag("Blend", "Alpha")
        :tag("DepthDisabled", "1")
        :pass(pass0)
        :build()
end

-- PassGroup: Add Blend No Depth
local passGroupAddBlendNoDepth
do
    local pass0 = pass "pass0"
        :blendState(BlendStates.ENABLE, 1)
        :blendState(BlendStates.SOURCE_BLEND, BlendFactors.SOURCE_ALPHA)
        :blendState(BlendStates.DEST_BLEND, BlendFactors.ONE)
        :blendState(BlendStates.BLEND_OPERATION, BlendOperations.ADD)
        :blendState(BlendStates.SOURCE_ALPHA_BLEND, BlendFactors.SOURCE_ALPHA)
        :blendState(BlendStates.DEST_ALPHA_BLEND, BlendFactors.INVERT_SOURCE_ALPHA)
        :blendState(BlendStates.ALPHA_BLEND_OPERATION, BlendOperations.ADD)
        :rasterizerState(RasterizerStates.CULL_MODE, CullModes.NONE)
        :depthStencilState(DepthStencilStates.DEPTH_ENABLE, 0)
        :vertexShader(vs)
        :pixelShader(ps)
        :build()
    passGroupAddBlendNoDepth = passGroup "AddBlendNoDepth"
        :tag("Blend", "Add")
        :tag("DepthDisabled", "1")
        :pass(pass0)
        :build()
end

-- PassGroup: Subtract Blend No Depth
local passGroupSubtractBlendNoDepth
do
    local pass0 = pass "pass0"
        :blendState(BlendStates.ENABLE, 1)
        :blendState(BlendStates.SOURCE_BLEND, BlendFactors.SOURCE_ALPHA)
        :blendState(BlendStates.DEST_BLEND, BlendFactors.ONE)
        :blendState(BlendStates.BLEND_OPERATION, BlendOperations.SUBTRACT)
        :blendState(BlendStates.SOURCE_ALPHA_BLEND, BlendFactors.SOURCE_ALPHA)
        :blendState(BlendStates.DEST_ALPHA_BLEND, BlendFactors.INVERT_SOURCE_ALPHA)
        :blendState(BlendStates.ALPHA_BLEND_OPERATION, BlendOperations.ADD)
        :rasterizerState(RasterizerStates.CULL_MODE, CullModes.NONE)
        :depthStencilState(DepthStencilStates.DEPTH_ENABLE, 0)
        :vertexShader(vs)
        :pixelShader(ps)
        :build()
    passGroupSubtractBlendNoDepth = passGroup "SubtractBlendNoDepth"
        :tag("Blend", "Subtract")
        :tag("DepthDisabled", "1")
        :pass(pass0)
        :build()
end

-- PassGroup: Revert Subtract Blend No Depth
local passGroupRevertSubtractBlendNoDepth
do
    local pass0 = pass "pass0"
        :blendState(BlendStates.ENABLE, 1)
        :blendState(BlendStates.SOURCE_BLEND, BlendFactors.SOURCE_ALPHA)
        :blendState(BlendStates.DEST_BLEND, BlendFactors.ONE)
        :blendState(BlendStates.BLEND_OPERATION, BlendOperations.REVERT_SUBTRACT)
        :blendState(BlendStates.SOURCE_ALPHA_BLEND, BlendFactors.SOURCE_ALPHA)
        :blendState(BlendStates.DEST_ALPHA_BLEND, BlendFactors.INVERT_SOURCE_ALPHA)
        :blendState(BlendStates.ALPHA_BLEND_OPERATION, BlendOperations.ADD)
        :rasterizerState(RasterizerStates.CULL_MODE, CullModes.NONE)
        :depthStencilState(DepthStencilStates.DEPTH_ENABLE, 0)
        :vertexShader(vs)
        :pixelShader(ps)
        :build()
    passGroupRevertSubtractBlendNoDepth = passGroup "RevertSubtractBlendNoDepth"
        :tag("Blend", "RevertSubtract")
        :tag("DepthDisabled", "1")
        :pass(pass0)
        :build()
end

-- Effect
return effect()
    :passGroup(passGroupAlphaBlend)
    :passGroup(passGroupAddBlend)
    :passGroup(passGroupSubtractBlend)
    :passGroup(passGroupRevertSubtractBlend)
    :passGroup(passGroupAlphaBlendNoDepth)
    :passGroup(passGroupAddBlendNoDepth)
    :passGroup(passGroupSubtractBlendNoDepth)
    :passGroup(passGroupRevertSubtractBlendNoDepth)
    :build()
)EFFECT";

LSTG_DEF_LOG_CATEGORY(CommandExecutor);

static const char* kBlendTagName = "Blend";
static const char* kDepthDisabledTagName = "DepthDisabled";

CommandExecutor::CommandExecutor(RenderSystem& renderSystem)
    : m_stRenderSystem(renderSystem), m_pDefaultTexture(renderSystem.GetDefaultTexture2D())
{
    // 创建默认的 2D 渲染 Shader
    auto effect = renderSystem.GetEffectFactory()->CreateEffect(kDefault2DEffect);
    if (!effect)
    {
        LSTG_LOG_ERROR_CAT(CommandExecutor, "Create default 2d effect fail: {}", effect.GetError());
        effect.ThrowIfError();
    }

    // 创建默认 Material
    auto material = renderSystem.CreateMaterial(*effect);
    if (!material)
    {
        LSTG_LOG_ERROR_CAT(CommandExecutor, "Create default 2d material fail: {}", material.GetError());
        material.ThrowIfError();
    }
    m_pDefaultMaterial = std::move(*material);

    // 创建动态 Mesh
    Render::GraphDef::MeshDefinition meshDefinition;
    {
        using ScalarTypes = Render::GraphDef::MeshDefinition::VertexElementScalarTypes;
        using Components = Render::GraphDef::MeshDefinition::VertexElementComponents;
        using SemanticNames = Render::GraphDef::MeshDefinition::VertexElementSemanticNames;
        meshDefinition.SetVertexStride(sizeof(Vertex));
        meshDefinition.SetPrimitiveTopologyType(Render::GraphDef::MeshDefinition::PrimitiveTopologyTypes::TriangleList);
        meshDefinition.AddVertexElement(
            {ScalarTypes::Float, Components::Three},
            {SemanticNames::Position, 0},
            offsetof(Vertex, Position));
        meshDefinition.AddVertexElement(
            {ScalarTypes::Float, Components::Two},
            {SemanticNames::TextureCoord, 0},
            offsetof(Vertex, TexCoord));
        meshDefinition.AddVertexElement(
            {ScalarTypes::UInt8, Components::Four},
            {SemanticNames::Color, 0},
            offsetof(Vertex, Color0));
        meshDefinition.AddVertexElement(
            {ScalarTypes::UInt8, Components::Four},
            {SemanticNames::Color, 1},
            offsetof(Vertex, Color1));
    }
    auto mesh = renderSystem.CreateDynamicMesh(meshDefinition, false);
    if (!mesh)
    {
        LSTG_LOG_ERROR_CAT(CommandExecutor, "Create dynamic mesh fail: {}", mesh.GetError());
        mesh.ThrowIfError();
    }
    m_pMesh = std::move(*mesh);
}

Result<void> CommandExecutor::Execute(CommandBuffer::DrawData& drawData) noexcept
{
    // 先保存当前状态
    Render::CameraPtr oldCamera;
    Render::MaterialPtr oldMaterial;
    RenderSystem::EffectGroupSelectCallback oldSelector;
    try
    {
        oldCamera = m_stRenderSystem.GetCamera();
        oldMaterial = m_stRenderSystem.GetMaterial();
        oldSelector = m_stRenderSystem.GetEffectGroupSelectCallback();
        m_stOldBlendTag = string{ m_stRenderSystem.GetRenderTag(kBlendTagName) };
        m_stOldDepthDisabledTag = string{ m_stRenderSystem.GetRenderTag(kDepthDisabledTagName) };

        // 设置新的 Selector
        m_stRenderSystem.SetEffectGroupSelectCallback([this](const GraphDef::EffectDefinition* effect,
            const std::map<std::string, std::string, std::less<>>& tags) {
            return OnSelectEffectGroup(effect, tags);
        });
    }
    catch (...)
    {
        return make_error_code(errc::not_enough_memory);
    }

    // 准备 Mesh
    auto ret = m_pMesh->Commit(
        {reinterpret_cast<const uint8_t*>(drawData.VertexBuffer.data()), drawData.VertexBuffer.size() * sizeof(drawData.VertexBuffer[0])},
        {reinterpret_cast<const uint8_t*>(drawData.IndexBuffer.data()), drawData.IndexBuffer.size() * sizeof(drawData.IndexBuffer[0])}
    );
    if (!ret)
    {
        LSTG_LOG_ERROR_CAT(CommandExecutor, "Commit mesh data fail: {}", ret.GetError());
        return ret.GetError();
    }

    // 遍历命令
    m_uDrawCalls = 0;
    for (const auto& group : drawData.CommandGroup)
        OnDrawGroup(drawData, *group.get());

    // 恢复状态
    m_stRenderSystem.SetCamera(oldCamera);
    m_stRenderSystem.SetMaterial(oldMaterial);
    m_stRenderSystem.SetEffectGroupSelectCallback(std::move(oldSelector));
    m_stRenderSystem.SetRenderTag(kBlendTagName, std::move(m_stOldBlendTag));
    m_stRenderSystem.SetRenderTag(kDepthDisabledTagName, std::move(m_stOldDepthDisabledTag));
    return {};
}

const Subsystem::Render::GraphDef::EffectPassGroupDefinition* CommandExecutor::OnSelectEffectGroup(const GraphDef::EffectDefinition* effect,
    const std::map<std::string, std::string, std::less<>>& tags) noexcept
{
    // 选择 Effect 组
    SelectableEffectPassGroups* passGroups = nullptr;
    if (m_stEffectGroupSelector.Contains(effect))
    {
        passGroups = &m_stEffectGroupSelector[effect];
        assert(passGroups);
    }
    else
    {
        assert(effect);

        // 构造 Effect 组并缓存
        SelectableEffectPassGroups cacheGroup;
        for (const auto& group : effect->GetGroups())
        {
            auto blendTag = group->GetTag(kBlendTagName);
            auto depthDisabledTag = group->GetTag(kDepthDisabledTagName) == "1" ? 1 : 0;
            if (blendTag == "Alpha")
                cacheGroup.AlphaBlendGroup[depthDisabledTag] = group.get();
            else if (blendTag == "Add")
                cacheGroup.AddBlendGroup[depthDisabledTag] = group.get();
            else if (blendTag == "Subtract")
                cacheGroup.SubtractBlendGroup[depthDisabledTag] = group.get();
            else if (blendTag == "ReverseSubtract")
                cacheGroup.ReverseSubtractBlendGroup[depthDisabledTag] = group.get();
        }

        try
        {
            passGroups = m_stEffectGroupSelector.Emplace(effect, cacheGroup);
        }
        catch (...)
        {
        }
    }

    if (!passGroups)
    {
        assert(false);  // 当且仅当内存分配失败会走这个路径
        return nullptr;
    }

    // 获取当前渲染标签
    ColorBlendMode targetBlend = ColorBlendMode::Alpha;
    int depthDisabled = 0;
    {
        auto it = tags.find(kBlendTagName);
        if (it != tags.end())
        {
            if (it->second == "Alpha")
                targetBlend = ColorBlendMode::Alpha;
            else if (it->second == "Add")
                targetBlend = ColorBlendMode::Add;
            else if (it->second == "Subtract")
                targetBlend = ColorBlendMode::Subtract;
            else if (it->second == "ReverseSubtract")
                targetBlend = ColorBlendMode::ReverseSubtract;
        }

        it = tags.find(kDepthDisabledTagName);
        depthDisabled = (it != tags.end() && it->second == "1") ? 1 : 0;
    }

    // 取缓存的组
    switch (targetBlend)
    {
        case ColorBlendMode::Alpha:
            return passGroups->AlphaBlendGroup[depthDisabled];
        case ColorBlendMode::Add:
            return passGroups->AddBlendGroup[depthDisabled];
        case ColorBlendMode::Subtract:
            return passGroups->SubtractBlendGroup[depthDisabled];
        case ColorBlendMode::ReverseSubtract:
            return passGroups->ReverseSubtractBlendGroup[depthDisabled];
        default:
            return nullptr;
    }
}

void CommandExecutor::OnDrawGroup(CommandBuffer::DrawData& drawData, CommandBuffer::CommandGroup& groupData) noexcept
{
    for (const auto& q : groupData.Queue)
        OnDrawQueue(drawData, *q.get());
}

void CommandExecutor::OnDrawQueue(CommandBuffer::DrawData& drawData, CommandBuffer::CommandQueue& queueData) noexcept
{
    // 获取相机
    assert(queueData.CameraId < drawData.CameraList.size());
    const auto& camera = drawData.CameraList[queueData.CameraId];
    assert(camera.get());
    m_stRenderSystem.SetCamera(*camera.get());

    // 清理
    optional<Render::ColorRGBA32> clearColor;
    optional<float> clearDepth;
    if (queueData.ClearFlag & ClearFlags::Color)
        clearColor = queueData.ClearColor;
    if (queueData.ClearFlag & ClearFlags::Depth)
        clearDepth = 1.f;
    if (clearColor || clearDepth)
        m_stRenderSystem.Clear(clearColor, clearDepth);

    // 绘制
    for (const auto& cmd : queueData.Commands)
    {
        // 获取纹理
        assert(cmd.TextureId < drawData.TextureList.size());
        auto tex2d = drawData.TextureList[cmd.TextureId];
        if (!tex2d) // 空白纹理时 fallback 到默认纹理
            tex2d = m_pDefaultTexture;

        // 获取材质
        assert(cmd.MaterialId < drawData.MaterialList.size());
        auto mat = drawData.MaterialList[cmd.MaterialId];
        if (!mat) // 没有指定材质时，fallback 到默认材质
            mat = m_pDefaultMaterial;

        // 设置混合状态
        switch (cmd.ColorBlend)
        {
            case ColorBlendMode::Alpha:
                m_stRenderSystem.SetRenderTag(kBlendTagName, "Alpha");
                break;
            case ColorBlendMode::Add:
                m_stRenderSystem.SetRenderTag(kBlendTagName, "Add");
                break;
            case ColorBlendMode::Subtract:
                m_stRenderSystem.SetRenderTag(kBlendTagName, "Subtract");
                break;
            case ColorBlendMode::ReverseSubtract:
                m_stRenderSystem.SetRenderTag(kBlendTagName, "ReverseSubtract");
                break;
        }

        // 设置深度状态
        m_stRenderSystem.SetRenderTag(kDepthDisabledTagName, cmd.NoDepth ? "1" : "0");

        // 设置材质
        assert(mat);
        m_stRenderSystem.SetMaterial(mat);

#define SET_UNIFORM_WITH_LOG(TYPE, NAME, VALUE) \
        do \
        { \
            if (!(ret = mat->SetUniform<TYPE>(#NAME, VALUE))) \
            { \
                if (ret.GetError() != make_error_code(GraphDef::DefinitionError::SymbolNotFound)) \
                    LSTG_LOG_ERROR_CAT(CommandExecutor, "Set uniform '" #NAME "' fail: {}", ret.GetError()); \
            } \
        } while (false)

        Result<void> ret;

        // 设置材质参数
        SET_UNIFORM_WITH_LOG(uint32_t, FogType, (static_cast<uint32_t>(cmd.FogType)));
        SET_UNIFORM_WITH_LOG(uint32_t, FogColorRGBA32, (cmd.FogColor.rgba32()));
        SET_UNIFORM_WITH_LOG(float, FogArg1, (cmd.FogArg1));
        SET_UNIFORM_WITH_LOG(float, FogArg2, (cmd.FogArg2));

        // 设置主纹理
        assert(tex2d);
        if (!(ret = mat->SetTexture("MainTexture", tex2d)))
            LSTG_LOG_ERROR_CAT(CommandExecutor, "Set MainTexture fail: {}", ret.GetError());

#undef SET_UNIFORM_WITH_LOG

        // 绘制
        ret = m_stRenderSystem.Draw(m_pMesh.get(), cmd.IndexCount, cmd.BaseVertexIndex, cmd.IndexStart);
        if (!ret)
            LSTG_LOG_ERROR_CAT(CommandExecutor, "Draw index={}, start={} fail: {}", cmd.IndexCount, cmd.IndexStart, ret.GetError());
        ++m_uDrawCalls;
    }
}
