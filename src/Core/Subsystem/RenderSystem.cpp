/**
 * @file
 * @date 2022/3/6
 * @author 9chu
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#include <lstg/Core/Subsystem/RenderSystem.hpp>

#include <vector>
#include <SDL.h>
#include <lstg/Core/Pal.hpp>
#include <lstg/Core/Logging.hpp>
#include <lstg/Core/Subsystem/SubsystemContainer.hpp>
#include <lstg/Core/Subsystem/Render/GraphicsDefinitionCache.hpp>
#include "Render/GraphDef/detail/ToDiligent.hpp"
#include "Render/detail/RenderDevice/RenderDeviceGL.hpp"
#include "Render/detail/RenderDevice/RenderDeviceVulkan.hpp"

using namespace std;
using namespace lstg;
using namespace lstg::Subsystem;

LSTG_DEF_LOG_CATEGORY(RenderSystem);

namespace
{
    /**
     * Diligent 调试日志转发
     */
    void DiligentDebugOutput(Diligent::DEBUG_MESSAGE_SEVERITY severity, const char* message, const char* function, const char* file,
        int line) noexcept
    {
        LogLevel level;
        switch (severity)
        {
            case Diligent::DEBUG_MESSAGE_SEVERITY_INFO:
                level = LogLevel::Info;
                break;
            case Diligent::DEBUG_MESSAGE_SEVERITY_WARNING:
                level = LogLevel::Warn;
                break;
            case Diligent::DEBUG_MESSAGE_SEVERITY_ERROR:
                level = LogLevel::Error;
                break;
            case Diligent::DEBUG_MESSAGE_SEVERITY_FATAL_ERROR:
                level = LogLevel::Critical;
                break;
            default:
                level = LogLevel::Debug;
                break;
        }

        Logging::GetInstance().Log(kLogCategoryRenderSystem.Name, level, lstg::detail::GetLogCurrentTime(), {file, function, line}, "{}",
            message);
    }

    using RenderDeviceConstructor = std::function<Render::RenderDevicePtr(WindowSystem*)>;

    /**
     * 获取支持的渲染设备
     * @param out 输出设备列表
     */
    void GetSupportedRenderDevice(std::vector<std::tuple<const char*, RenderDeviceConstructor>>& out)
    {
#if VULKAN_SUPPORTED == 1
        out.emplace_back("Vulkan", [](WindowSystem* windowSystem) -> Render::RenderDevicePtr {
            return make_shared<Render::detail::RenderDevice::RenderDeviceVulkan>(windowSystem);
        });
#endif
#if GL_SUPPORTED == 1 || GLES_SUPPORTED == 1
        out.emplace_back("OpenGL", [](WindowSystem* windowSystem) -> Render::RenderDevicePtr {
            return make_shared<Render::detail::RenderDevice::RenderDeviceGL>(windowSystem);
        });
#endif
    }

    struct CameraState
    {
        glm::mat4x4 ViewMatrix {};
        glm::mat4x4 ProjectMatrix {};
        glm::mat4x4 ProjectViewMatrix {};
        float ViewportLeft = 0.f;
        float ViewportTop = 0.f;
        float ViewportWidth = 0.f;
        float ViewportHeight = 0.f;
    };

    Render::ConstantBufferPtr CreateCameraStateConstantBuffer(Render::RenderDevice& device)
    {
        using namespace Render::GraphDef;

        ConstantBufferDefinition desc("_CameraState");
        // float4x4 _CameraViewMatrix
        desc.DefineField("_CameraViewMatrix", ConstantBufferValueType{ConstantBufferValueType::ScalarTypes::Float, 4, 4});
        // float4x4 _CameraProjectMatrix
        desc.DefineField("_CameraProjectMatrix", ConstantBufferValueType{ConstantBufferValueType::ScalarTypes::Float, 4, 4});
        // float4x4 _CameraProjectViewMatrix
        desc.DefineField("_CameraProjectViewMatrix", ConstantBufferValueType{ConstantBufferValueType::ScalarTypes::Float, 4, 4});
        // float _CameraViewportLeft
        desc.DefineField("_CameraViewportLeft", ConstantBufferValueType{ConstantBufferValueType::ScalarTypes::Float});
        // float _CameraViewportTop
        desc.DefineField("_CameraViewportTop", ConstantBufferValueType{ConstantBufferValueType::ScalarTypes::Float});
        // float _CameraViewportWidth
        desc.DefineField("_CameraViewportWidth", ConstantBufferValueType{ConstantBufferValueType::ScalarTypes::Float});
        // float _CameraViewportHeight
        desc.DefineField("_CameraViewportHeight", ConstantBufferValueType{ConstantBufferValueType::ScalarTypes::Float});

        auto def = make_shared<const ConstantBufferDefinition>(desc);
        auto ret = make_shared<Render::ConstantBuffer>(device, def, Render::ConstantBuffer::Usage::Dynamic);
        assert(ret->GetSize() == sizeof(CameraState));
        assert(def->GetFields()[0].Size == sizeof(CameraState::ViewMatrix));
        assert(def->GetFields()[0].Offset == offsetof(CameraState, ViewMatrix));
        assert(def->GetFields()[1].Size == sizeof(CameraState::ProjectMatrix));
        assert(def->GetFields()[1].Offset == offsetof(CameraState, ProjectMatrix));
        assert(def->GetFields()[2].Size == sizeof(CameraState::ProjectViewMatrix));
        assert(def->GetFields()[2].Offset == offsetof(CameraState, ProjectViewMatrix));
        assert(def->GetFields()[3].Size == sizeof(CameraState::ViewportLeft));
        assert(def->GetFields()[3].Offset == offsetof(CameraState, ViewportLeft));
        assert(def->GetFields()[4].Size == sizeof(CameraState::ViewportTop));
        assert(def->GetFields()[4].Offset == offsetof(CameraState, ViewportTop));
        assert(def->GetFields()[5].Size == sizeof(CameraState::ViewportWidth));
        assert(def->GetFields()[5].Offset == offsetof(CameraState, ViewportWidth));
        assert(def->GetFields()[6].Size == sizeof(CameraState::ViewportHeight));
        assert(def->GetFields()[6].Offset == offsetof(CameraState, ViewportHeight));
        return ret;
    }
}

RenderSystem::RenderSystem(SubsystemContainer& container)
    : m_pWindowSystem(container.Get<WindowSystem>()), m_pVirtualFileSystem(container.Get<VirtualFileSystem>())
{
    assert(m_pWindowSystem);

    // 设置 Debug Output
    Diligent::SetDebugMessageCallback(DiligentDebugOutput);

    // 初始化渲染设备
    vector<tuple<const char*, RenderDeviceConstructor>> supportedDevices;
    GetSupportedRenderDevice(supportedDevices);
    if (!supportedDevices.empty())
    {
        for (const auto& builder : supportedDevices)
        {
            try
            {
                LSTG_LOG_INFO_CAT(RenderSystem, "Try initializing render backend {}", std::get<0>(builder));
                m_pRenderDevice = std::get<1>(builder)(m_pWindowSystem.get());
                break;
            }
            catch (const std::exception& ex)
            {
                LSTG_LOG_ERROR_CAT(RenderSystem, "Initializing render backend {} fail: {}", std::get<0>(builder), ex.what());
            }
        }
    }
    if (!m_pRenderDevice)
        LSTG_THROW(Render::RenderDeviceInitializeFailedException, "No available render device");

    // 初始化效果工厂
    m_pEffectFactory = make_shared<Render::EffectFactory>(*m_pVirtualFileSystem, *m_pRenderDevice);

    // 创建内建 CBuffer
    m_pCameraStateCBuffer = CreateCameraStateConstantBuffer(*GetRenderDevice());
    auto ret = m_pEffectFactory->RegisterGlobalConstantBuffer(m_pCameraStateCBuffer);
    assert(ret);
    static_cast<void>(ret);
}

// <editor-fold desc="资源分配">

Result<Render::CameraPtr> RenderSystem::CreateCamera() noexcept
{
    try
    {
        return make_shared<Render::Camera>();
    }
    catch (...)  // bad_alloc
    {
        return make_error_code(errc::not_enough_memory);
    }
}

Result<Render::MaterialPtr> RenderSystem::CreateMaterial(const Render::GraphDef::ImmutableEffectDefinitionPtr& effect) noexcept
{
    try
    {
        return make_shared<Render::Material>(*GetRenderDevice(), effect);
    }
    catch (const system_error& ex)
    {
        return ex.code();
    }
    catch (...)  // bad_alloc
    {
        return make_error_code(errc::not_enough_memory);
    }
}

Result<Render::MeshPtr> RenderSystem::CreateStaticMesh(const Render::GraphDef::MeshDefinition& def, Span<const uint8_t> vertexData,
    Span<const uint8_t> indexData, bool use32BitIndex) noexcept
{
    if (def.GetVertexElements().empty() || def.GetVertexStride() == 0)
        return make_error_code(errc::invalid_argument);

    // 检查 VertexData 和 IndexData
    if (vertexData.size() % def.GetVertexStride() != 0)
        return make_error_code(errc::invalid_argument);
    if (indexData.size() == 0)
        return make_error_code(errc::invalid_argument);
    if ((use32BitIndex && (indexData.size() % 4 != 0)) || indexData.size() % 2 != 0)
        return make_error_code(errc::invalid_argument);

    try
    {
        // 生成定义
        // MeshDefinition 必须 Cache，以获取全局唯一实例，用于加速查询 PSO Cache
        auto sharedDef = m_stMeshDefCache.CreateDefinition(def);

        // 创建 VertexBuffer
        Diligent::RefCntAutoPtr<Diligent::IBuffer> vertexBuffer;
        {
            Diligent::BufferDesc vertexBufferDesc;
            vertexBufferDesc.BindFlags = Diligent::BIND_VERTEX_BUFFER;
            vertexBufferDesc.Size = vertexData.size();
            vertexBufferDesc.Usage = Diligent::USAGE_IMMUTABLE;
            vertexBufferDesc.CPUAccessFlags = Diligent::CPU_ACCESS_NONE;
            Diligent::BufferData vertexDataDesc;
            vertexDataDesc.DataSize = vertexData.size();
            vertexDataDesc.pContext = m_pRenderDevice->GetImmediateContext();
            vertexDataDesc.pData = vertexData.data();
            m_pRenderDevice->GetDevice()->CreateBuffer(vertexBufferDesc, &vertexDataDesc, &vertexBuffer);
            if (!vertexBuffer)
                return make_error_code(errc::not_enough_memory);
        }

        // 创建 IndexBuffer
        Diligent::RefCntAutoPtr<Diligent::IBuffer> indexBuffer;
        {
            Diligent::BufferDesc indexBufferDesc;
            indexBufferDesc.BindFlags = Diligent::BIND_INDEX_BUFFER;
            indexBufferDesc.Size = indexData.size();
            indexBufferDesc.Usage = Diligent::USAGE_IMMUTABLE;
            indexBufferDesc.CPUAccessFlags = Diligent::CPU_ACCESS_NONE;
            Diligent::BufferData indexDataDesc;
            indexDataDesc.DataSize = indexData.size();
            indexDataDesc.pContext = m_pRenderDevice->GetImmediateContext();
            indexDataDesc.pData = indexData.data();
            m_pRenderDevice->GetDevice()->CreateBuffer(indexBufferDesc, &indexDataDesc, &indexBuffer);
            if (!indexBuffer)
                return make_error_code(errc::not_enough_memory);
        }

        // 创建 Mesh 对象
        return make_shared<Render::Mesh>(sharedDef, vertexBuffer, indexBuffer, use32BitIndex);
    }
    catch (...)  // bad_alloc
    {
        return make_error_code(errc::not_enough_memory);
    }
}

// </editor-fold>
// <editor-fold desc="渲染控制">

Result<void> RenderSystem::BeginFrame() noexcept
{
    auto swapChain = m_pRenderDevice->GetSwapChain();
    auto context = m_pRenderDevice->GetImmediateContext();

    auto* renderTargetView = swapChain->GetCurrentBackBufferRTV();
    auto* depthStencilView = swapChain->GetDepthBufferDSV();

    // 在帧开始切换 RT 到 SwapChain 上的 Buffer
    {
        context->SetRenderTargets(1, &renderTargetView, depthStencilView, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
        m_stCurrentOutputViews = {};
    }

    // 切换 VP 到全屏
    {
        auto sz = GetCurrentOutputViewSize();
        Diligent::Viewport vp;
        vp.MinDepth = 0.0f;
        vp.MaxDepth = 1.0f;
        vp.Width = static_cast<float>(std::get<0>(sz));
        vp.Height = static_cast<float>(std::get<1>(sz));
        vp.TopLeftX = 0.f;
        vp.TopLeftY = 0.f;
        m_pRenderDevice->GetImmediateContext()->SetViewports(1, &vp, 0, 0);
        m_stCurrentViewport = { 0.f, 0.f, vp.Width, vp.Height };  // 这里必须强制写出大小
    }

    // 清空 RT
    {
        static const float kClearColor[4] = { 0.f, 0.f, 0.f, 0.f };
        context->ClearRenderTarget(renderTargetView, kClearColor, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
        context->ClearDepthStencil(depthStencilView, Diligent::CLEAR_DEPTH_FLAG /* | Diligent::CLEAR_STENCIL_FLAG */, 1.0f, 0,
            Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
    }
    return {};
}

void RenderSystem::EndFrame() noexcept
{
    auto swapChain = m_pRenderDevice->GetSwapChain();
    auto context = m_pRenderDevice->GetImmediateContext();

    auto* renderTargetView = swapChain->GetCurrentBackBufferRTV();
    auto* depthStencilView = swapChain->GetDepthBufferDSV();

    // 再次设置 RT，防止渲染过程中的变动
    {
        context->SetRenderTargets(1, &renderTargetView, depthStencilView, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
        m_stCurrentOutputViews = {};
    }

    // 执行 Present
    m_pRenderDevice->Present();
}

void RenderSystem::SetCamera(Render::CameraPtr camera) noexcept
{
    assert(camera);
    if (camera == m_pCurrentCamera)
        return;
    camera->m_bStateDirty = true;
    m_pCurrentCamera = std::move(camera);
}

void RenderSystem::SetMaterial(Render::MaterialPtr material) noexcept
{
    assert(material);
    if (material == m_pCurrentMaterial)
        return;
    m_pCurrentMaterial = std::move(material);
    m_pCurrentPassGroup = nullptr;
}

void RenderSystem::SetEffectPassGroupSelector(Render::EffectPassGroupSelectorPtr selector) noexcept
{
    assert(selector);
    if (selector == m_pCurrentSelector)
        return;
    m_pCurrentSelector = std::move(selector);
    m_pCurrentPassGroup = nullptr;
}

Result<void> RenderSystem::Draw(Render::Mesh* mesh, size_t indexCount, size_t vertexOffset, size_t indexOffset) noexcept
{
    if (!mesh)
        return make_error_code(errc::invalid_argument);
    if (!m_pCurrentCamera || !m_pCurrentMaterial)
        return make_error_code(errc::invalid_argument);

    auto context = m_pRenderDevice->GetImmediateContext();
    auto meshDef = mesh->GetDefinition();

    // 准备 Camera、Material 数据
    {
        auto ret = CommitCamera();
        if (!ret)
        {
            LSTG_LOG_ERROR_CAT(RenderSystem, "Commit camera state fail: {}", ret.GetError());
            return ret.GetError();
        }
        ret = CommitMaterial();
        if (!ret)
        {
            LSTG_LOG_ERROR_CAT(RenderSystem, "Commit material state fail: {}", ret.GetError());
            return ret.GetError();
        }
    }

    // 准备 VB,IB
    {
        assert(vertexOffset < mesh->GetVertexCount());
        assert(indexOffset < mesh->GetIndexCount());
        Diligent::IBuffer* vertexBuffers[] = {mesh->m_pVertexBuffer};
        Uint64 vertexOffsets[] = {mesh->GetDefinition()->GetVertexStride() * vertexOffset};
        context->SetIndexBuffer(mesh->m_pIndexBuffer, indexOffset * (mesh->Is32BitsIndex() ? 4 : 2),
            Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
        context->SetVertexBuffers(0, 1, vertexBuffers, vertexOffsets, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION,
            Diligent::SET_VERTEX_BUFFERS_FLAG_RESET);
    }

    // 依次渲染 Pass
    Diligent::DrawIndexedAttribs drawAttrs;
    drawAttrs.NumIndices = indexCount;
    drawAttrs.IndexType = mesh->Is32BitsIndex() ? Diligent::VT_UINT32 : Diligent::VT_UINT16;
    drawAttrs.Flags = Diligent::DRAW_FLAG_VERIFY_STATES;
    for (const auto& pass : m_pCurrentPassGroup->GetPasses())
    {
        // 找到实例
        auto it = m_pCurrentMaterial->m_stPassInstances.find(pass.get());
        assert(it != m_pCurrentMaterial->m_stPassInstances.end());

        // 准备管线
        auto ret = PreparePipeline(pass.get(), meshDef.get());
        if (!ret)
        {
            LSTG_LOG_ERROR_CAT(RenderSystem, "Prepare pipeline on pass {} fail", pass->GetName());
            return ret.GetError();
        }

        // 提交 SRB
        context->CommitShaderResources(it->second.ResourceBinding, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

        // 渲染
        context->DrawIndexed(drawAttrs);
    }
    return {};
}

std::tuple<uint32_t, uint32_t> RenderSystem::GetCurrentOutputViewSize() noexcept
{
    if (m_stCurrentOutputViews.ColorView == nullptr)
    {
        return { m_pRenderDevice->GetRenderOutputWidth(), m_pRenderDevice->GetRenderOutputHeight() };
    }
    else
    {
        auto texture = m_stCurrentOutputViews.ColorView->GetTexture();
        assert(texture);
        return { texture->GetDesc().Width, texture->GetDesc().Height };
    }
}

const Render::GraphDef::EffectPassGroupDefinition* RenderSystem::SelectPassGroup() noexcept
{
    assert(m_pCurrentMaterial);
    auto def = m_pCurrentMaterial->GetDefinition();
    const auto& groups = def->GetGroups();
    assert(!groups.empty());

    // 没有效果选择器，直接返回第一个
    if (!m_pCurrentSelector)
        return groups[0].get();

    // 通知选择器进行选择
    m_pCurrentSelector->Reset();
    for (const auto& g : groups)
        m_pCurrentSelector->AddCandidate(g.get());
    auto result = m_pCurrentSelector->GetSelectedPassGroup();

    // 没有选出来，则 fallback 到第一个结果
    if (!result)
        return groups[0].get();
    return result;
}

Result<void> RenderSystem::CommitCamera() noexcept
{
    assert(m_pCurrentCamera);
    auto swapChain = m_pRenderDevice->GetSwapChain();
    auto context = m_pRenderDevice->GetImmediateContext();

    const auto& outputViews = m_pCurrentCamera->GetOutputViews();
    const auto& viewport = m_pCurrentCamera->GetViewport();
    bool forceUpdateViewport = false;
    bool viewportChanged = false;

    // 提交 RT
    if (!(m_stCurrentOutputViews == outputViews))
    {
        Diligent::ITextureView* renderTargetView = outputViews.ColorView ? outputViews.ColorView : swapChain->GetCurrentBackBufferRTV();
        Diligent::ITextureView* depthStencilView = outputViews.DepthStencilView ? outputViews.DepthStencilView :
            swapChain->GetDepthBufferDSV();

        // 切换 RT
        context->SetRenderTargets(1, &renderTargetView, depthStencilView, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
        m_stCurrentOutputViews = outputViews;

        // 同时调整 Viewport
        forceUpdateViewport = true;
    }

    // 提交 Viewport
    if (!(m_stCurrentViewport == viewport) || forceUpdateViewport)
    {
        if (!viewport.IsAutoViewport())
        {
            Diligent::Viewport vp;
            vp.MinDepth = 0.0f;
            vp.MaxDepth = 1.0f;
            vp.Width = viewport.Width;
            vp.Height = viewport.Height;
            vp.TopLeftX = viewport.Left;
            vp.TopLeftY = viewport.Top;
            m_pRenderDevice->GetImmediateContext()->SetViewports(1, &vp, 0, 0);
            m_stCurrentViewport = viewport;
        }
        else
        {
            auto sz = GetCurrentOutputViewSize();
            Render::Camera::Viewport targetViewport = {0.f, 0.f, static_cast<float>(std::get<0>(sz)), static_cast<float>(std::get<1>(sz))};
            if (!(targetViewport == m_stCurrentViewport))
            {
                Diligent::Viewport vp;
                vp.MinDepth = 0.0f;
                vp.MaxDepth = 1.0f;
                vp.Width = targetViewport.Width;
                vp.Height = targetViewport.Height;
                vp.TopLeftX = targetViewport.Left;
                vp.TopLeftY = targetViewport.Top;
                m_pRenderDevice->GetImmediateContext()->SetViewports(1, &vp, 0, 0);
                m_stCurrentViewport = viewport;
            }
        }
        viewportChanged = true;
    }

    // 提交相机参数
    if (m_pCurrentCamera->m_bStateDirty || viewportChanged)
    {
        CameraState state = {
            m_pCurrentCamera->GetViewMatrix(),
            m_pCurrentCamera->GetProjectMatrix(),
            m_pCurrentCamera->GetProjectViewMatrix(),
            m_stCurrentViewport.Left,
            m_stCurrentViewport.Top,
            m_stCurrentViewport.Width,
            m_stCurrentViewport.Height,
        };

        // 直接整个刷新
        m_pCameraStateCBuffer->CopyFrom(&state, sizeof(state), 0);

        // 取消脏标记
        m_pCurrentCamera->m_bStateDirty = false;
    }

    // Dynamic CBuffer 每帧都会提交
    return m_pCameraStateCBuffer->Commit();
}

Result<void> RenderSystem::CommitMaterial() noexcept
{
    assert(m_pCurrentMaterial);

    // 检查是否有 PassGroup
    if (!m_pCurrentPassGroup)
        m_pCurrentPassGroup = SelectPassGroup();
    assert(m_pCurrentPassGroup);

    // 提交数据
    return m_pCurrentMaterial->Commit(m_pCurrentPassGroup);
}

Result<void> RenderSystem::PreparePipeline(const Render::GraphDef::EffectPassDefinition* pass,
    const Render::GraphDef::MeshDefinition* meshDef)
{
    assert(pass && meshDef);
    auto swapChain = m_pRenderDevice->GetSwapChain();

    // 选择 PSO
    Diligent::RefCntAutoPtr<Diligent::IPipelineState> pso;

    // 检查是否已经存在 PSO
    Render::GraphDef::EffectPassDefinition::PipelineCacheKey key;
    key.ColorBufferFormat = (m_stCurrentOutputViews.ColorView == nullptr ? swapChain->GetDesc().ColorBufferFormat :
        m_stCurrentOutputViews.ColorView->GetDesc().Format);
    key.DepthBufferFormat = (m_stCurrentOutputViews.DepthStencilView == nullptr ? swapChain->GetDesc().DepthBufferFormat :
        m_stCurrentOutputViews.DepthStencilView->GetDesc().Format);
    key.MeshDef = meshDef;
    auto it = pass->m_stPipelineStateCaches.find(key);
    if (it != pass->m_stPipelineStateCaches.end())
    {
        // 使用缓存的 PSO
        pso = it->second;
    }
    else
    {
        try
        {
            // 创建新的 PSO
            string name = fmt::format("PSO #{}", key.GetHashCode());

            Diligent::IPipelineResourceSignature* prs[] = { pass->m_pResourceSignature };
            assert(prs[0]);

            Diligent::GraphicsPipelineStateCreateInfo pipelineCreateInfo;
            pipelineCreateInfo.PSODesc.Name = name.c_str();

            // 绑定 PRS
            pipelineCreateInfo.ResourceSignaturesCount = 1;
            pipelineCreateInfo.ppResourceSignatures = prs;

            // 设置光栅化参数
            auto& graphicsPipeline = pipelineCreateInfo.GraphicsPipeline;
            graphicsPipeline.NumRenderTargets = 1;
            graphicsPipeline.RTVFormats[0] = static_cast<Diligent::TEXTURE_FORMAT>(key.ColorBufferFormat);
            graphicsPipeline.DSVFormat = static_cast<Diligent::TEXTURE_FORMAT>(key.DepthBufferFormat);
            graphicsPipeline.PrimitiveTopology = Render::GraphDef::detail::ToDilignet(meshDef->GetPrimitiveTopologyType());
            graphicsPipeline.RasterizerDesc = Render::GraphDef::detail::ToDiligent(pass->GetRasterizerState());
            graphicsPipeline.DepthStencilDesc = Render::GraphDef::detail::ToDiligent(pass->GetDepthStencilState());
            graphicsPipeline.BlendDesc.RenderTargets[0] = Render::GraphDef::detail::ToDiligent(pass->GetBlendState());

            // 顶点模式
            vector<Diligent::LayoutElement> vertexLayout;
            for (const auto& s : pass->GetVertexShader()->GetVertexLayout()->GetSlots())
            {
                // 从 Mesh 中找到对应语义的槽
                bool found = false;
                for (const auto& vm : meshDef->GetVertexElements())
                {
                    if (vm.Semantic == s.second.Semantic)
                    {
                        vertexLayout.emplace_back(Diligent::LayoutElement {
                            s.second.SlotIndex,  // _InputIndex
                            0,  // _BufferSlot
                            static_cast<unsigned>(std::get<1>(vm.Type)),  // _NumComponents
                            Render::GraphDef::detail::ToDiligent(std::get<0>(vm.Type)),  // _ValueType
                            s.second.Normalized,  // _IsNormalized
                            static_cast<unsigned>(vm.Offset),  // _RelativeOffset
                            static_cast<unsigned>(meshDef->GetVertexStride())  // _Stride
                        });
                        found = true;
                        break;
                    }
                }
                if (!found)
                {
                    LSTG_LOG_WARN_CAT(RenderSystem, "Pass \"{}\" vertex layout slot {} missing in mesh", pass->GetName(),
                        s.second.SlotIndex);
                }
            }
            graphicsPipeline.InputLayout.NumElements = vertexLayout.size();
            graphicsPipeline.InputLayout.LayoutElements = vertexLayout.data();

            // Shader
            pipelineCreateInfo.pVS = pass->GetVertexShader()->m_pCompiledShader;
            pipelineCreateInfo.pPS = pass->GetPixelShader()->m_pCompiledShader;
            assert(pipelineCreateInfo.pVS && pipelineCreateInfo.pPS);

            m_pRenderDevice->GetDevice()->CreateGraphicsPipelineState(pipelineCreateInfo, &pso);
            if (!pso)
            {
                LSTG_LOG_ERROR_CAT(RenderSystem, "Create pso fail, pass \"{}\", key #{}", pass->GetName(), key.GetHashCode());
                return make_error_code(errc::io_error);
            }

            // 记录
            LSTG_LOG_TRACE_CAT(RenderSystem, "PSO #{} created", key.GetHashCode());
            pass->m_stPipelineStateCaches.emplace(key, pso);
            pso->AddRef();
        }
        catch (...)  // bad_alloc
        {
            return make_error_code(errc::not_enough_memory);
        }
    }
    assert(pso);

    // 设置 PSO
    m_pRenderDevice->GetImmediateContext()->SetPipelineState(pso);
    return {};
}

void RenderSystem::OnEvent(SubsystemEvent& event) noexcept
{
    const auto& underlay = event.GetEvent();
    if (underlay.index() != 0)
        return;

    const SDL_Event* platformEvent = std::get<0>(underlay);

    if (platformEvent->window.event == SDL_WINDOWEVENT_RESIZED)
    {
        auto renderSize = m_pWindowSystem->GetRenderSize();
        auto newWidth = std::get<0>(renderSize);
        auto newHeight = std::get<1>(renderSize);
        if (newWidth > 0 && newHeight > 0)
        {
            auto swapChain = m_pRenderDevice->GetSwapChain();
            const auto& desc = swapChain->GetDesc();
            if (desc.Width != static_cast<uint32_t>(newWidth) || desc.Height != static_cast<uint32_t>(newHeight))
            {
                LSTG_LOG_INFO_CAT(RenderSystem, "Swap chain resize {}x{} -> {}x{}", desc.Width, desc.Height, newWidth, newHeight);
                swapChain->Resize(newWidth, newHeight);
            }
        }
    }
}
