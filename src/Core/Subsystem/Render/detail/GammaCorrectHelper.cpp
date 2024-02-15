/**
* @file
* @date 2022/8/19
* @author 9chu
* 此文件为 LuaSTGPlus 项目的一部分，版权与许可声明详见 COPYRIGHT.txt。
 */
#include "GammaCorrectHelper.hpp"

#include <MapHelper.hpp>
#include <RenderDevice.h>
#include <SwapChain.h>
#include <lstg/Core/Logging.hpp>

using namespace std;
using namespace lstg;
using namespace lstg::Subsystem::Render::detail;

using namespace Diligent;

LSTG_DEF_LOG_CATEGORY(GammaCorrectHelper);

struct UVQuadVert
{
    float x, y, z;
    float u, v;
};
static_assert(sizeof(UVQuadVert) == 3 * 4 + 2 * 4);

static const char* kGammaCorrectPostEffectVertexShaderHLSL = R"(
    struct VSInput
    {
        float3 pos : ATTRIB0;
        float2 uv  : ATTRIB1;
    };

    struct PSInput
    {
        float4 pos : SV_POSITION;
        float2 uv  : TEXCOORD;
    };

    void main(in VSInput VSIn, out PSInput PSIn)
    {
        PSIn.pos = float4(VSIn.pos.xyz, 1.0);
        PSIn.uv  = VSIn.uv;
    }
)";

static const char* kGammaCorrectPostEffectPixelShaderHLSL = R"(
    struct PSInput
    {
        float4 pos : SV_POSITION;
        float2 uv  : TEXCOORD;
    };

    Texture2D    Texture;
    SamplerState Texture_sampler;

    float4 main(in PSInput PSIn) : SV_Target
    {
        const float kGamma = 2.2f;
        float4 sampleColor = Texture.Sample(Texture_sampler, PSIn.uv);
        float3 correctColor = pow(sampleColor.rgb, float3(1.f / kGamma));
        return float4(correctColor, sampleColor.a);
    }
)";

GammaCorrectHelper::GammaCorrectHelper(RenderDevice* device)
    : m_pDevice(device)
{
    auto renderDevice = m_pDevice->GetDevice();
    auto swapChain = m_pDevice->GetSwapChain();

    // 创建 Shader
    RefCntAutoPtr<IShader> vertexShader;
    {
        ShaderCreateInfo shaderCreateInfo;
        shaderCreateInfo.SourceLanguage = SHADER_SOURCE_LANGUAGE_HLSL;
        shaderCreateInfo.Desc.ShaderType = SHADER_TYPE_VERTEX;
        shaderCreateInfo.Desc.Name = "Gamma Correct VS";
        shaderCreateInfo.Desc.UseCombinedTextureSamplers = true;
        shaderCreateInfo.Source = kGammaCorrectPostEffectVertexShaderHLSL;
        renderDevice->CreateShader(shaderCreateInfo, &vertexShader);
        if (!vertexShader)
            LSTG_THROW(GammaCorrectHelperInitializeFailedException, "Create vertex shader fail");
    }

    RefCntAutoPtr<IShader> pixelShader;
    {
        ShaderCreateInfo shaderCreateInfo;
        shaderCreateInfo.SourceLanguage = SHADER_SOURCE_LANGUAGE_HLSL;
        shaderCreateInfo.Desc.ShaderType = SHADER_TYPE_PIXEL;
        shaderCreateInfo.Desc.Name = "Gamma Correct PS";
        shaderCreateInfo.Desc.UseCombinedTextureSamplers = true;
        shaderCreateInfo.Source = kGammaCorrectPostEffectPixelShaderHLSL;
        renderDevice->CreateShader(shaderCreateInfo, &pixelShader);
        if (!pixelShader)
            LSTG_THROW(GammaCorrectHelperInitializeFailedException, "Create pixel shader fail");
    }

    // 创建 Pipeline
    {
        GraphicsPipelineStateCreateInfo pipelineCreateInfo;
        pipelineCreateInfo.PSODesc.Name = "Gamma Correct PSO";

        // 光栅化参数
        auto& graphicsPipeline = pipelineCreateInfo.GraphicsPipeline;
        graphicsPipeline.NumRenderTargets = 1;
        graphicsPipeline.RTVFormats[0] = swapChain->GetDesc().ColorBufferFormat;
        graphicsPipeline.DSVFormat = swapChain->GetDesc().DepthBufferFormat;
        graphicsPipeline.PrimitiveTopology = PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        graphicsPipeline.RasterizerDesc.CullMode = CULL_MODE_NONE;
        graphicsPipeline.RasterizerDesc.ScissorEnable = false;
        graphicsPipeline.DepthStencilDesc.DepthEnable = false;
        graphicsPipeline.DepthStencilDesc.DepthWriteEnable = false;
        graphicsPipeline.DepthStencilDesc.StencilEnable = false;

        // 混合模式
        auto& blendDesc = graphicsPipeline.BlendDesc.RenderTargets[0];
        blendDesc.BlendEnable = false;
        blendDesc.RenderTargetWriteMask = COLOR_MASK_ALL;

        // 顶点模式
        LayoutElement vertexInputLayout[] {
            {0, 0, 3, VT_FLOAT32},  // pos
            {1, 0, 2, VT_FLOAT32},  // uv
        };
        graphicsPipeline.InputLayout.NumElements = std::extent_v<decltype(vertexInputLayout)>;
        graphicsPipeline.InputLayout.LayoutElements = vertexInputLayout;

        // Shader
        pipelineCreateInfo.pVS = vertexShader;
        pipelineCreateInfo.pPS = pixelShader;

        ShaderResourceVariableDesc shaderVariables[] = {
            {SHADER_TYPE_PIXEL, "Texture", SHADER_RESOURCE_VARIABLE_TYPE_DYNAMIC}
        };
        pipelineCreateInfo.PSODesc.ResourceLayout.Variables = shaderVariables;
        pipelineCreateInfo.PSODesc.ResourceLayout.NumVariables = std::extent_v<decltype(shaderVariables)>;

        SamplerDesc samplerDesc;
        samplerDesc.AddressU = TEXTURE_ADDRESS_WRAP;
        samplerDesc.AddressV = TEXTURE_ADDRESS_WRAP;
        samplerDesc.AddressW = TEXTURE_ADDRESS_WRAP;
        ImmutableSamplerDesc immutableSamplers[] = {
            {SHADER_TYPE_PIXEL, "Texture", samplerDesc}
        };
        pipelineCreateInfo.PSODesc.ResourceLayout.ImmutableSamplers = immutableSamplers;
        pipelineCreateInfo.PSODesc.ResourceLayout.NumImmutableSamplers = std::extent_v<decltype(immutableSamplers)>;

        renderDevice->CreateGraphicsPipelineState(pipelineCreateInfo, &m_pPostEffectPSO);
        if (!m_pPostEffectPSO)
            LSTG_THROW(GammaCorrectHelperInitializeFailedException, "Create pipeline state fail");
    }

    // 创建 RT
    {
        TextureDesc renderTargetDesc;
        renderTargetDesc.Name = "Gamma Correct Texture";
        renderTargetDesc.Type = Diligent::RESOURCE_DIM_TEX_2D;
        renderTargetDesc.Width = swapChain->GetDesc().Width;
        renderTargetDesc.Height = swapChain->GetDesc().Height;
        renderTargetDesc.MipLevels = 1;
        renderTargetDesc.BindFlags = Diligent::BIND_SHADER_RESOURCE | Diligent::BIND_RENDER_TARGET;
        renderTargetDesc.Usage = Diligent::USAGE_DEFAULT;
        renderTargetDesc.Format = swapChain->GetDesc().ColorBufferFormat;  // 需要和 SwapChain 一致
        renderTargetDesc.ClearValue.Format = renderTargetDesc.Format;
        renderTargetDesc.ClearValue.Color[0] = 0.f;
        renderTargetDesc.ClearValue.Color[1] = 0.f;
        renderTargetDesc.ClearValue.Color[2] = 0.f;
        renderTargetDesc.ClearValue.Color[3] = 0.f;

        RefCntAutoPtr<ITexture> renderTarget;
        renderDevice->CreateTexture(renderTargetDesc, nullptr, &renderTarget);
        if (!renderTarget)
            LSTG_THROW(GammaCorrectHelperInitializeFailedException, "Create render target fail");
        m_pColorFrameBuffer = renderTarget;
        m_pColorFrameBufferSRV = m_pColorFrameBuffer->GetDefaultView(TEXTURE_VIEW_SHADER_RESOURCE);
        m_pColorFrameBufferRTV = m_pColorFrameBuffer->GetDefaultView(TEXTURE_VIEW_RENDER_TARGET);

        m_pPostEffectPSO->CreateShaderResourceBinding(&m_pPostEffectSRB, true);
        if (!m_pPostEffectSRB)
            LSTG_THROW(GammaCorrectHelperInitializeFailedException, "Create shader resource binding fail");
        m_pColorFrameTextureVar = m_pPostEffectSRB->GetVariableByName(SHADER_TYPE_PIXEL, "Texture");
        assert(m_pColorFrameTextureVar);
    }

    // 创建 VB&IB
    {
        const UVQuadVert kFixedVert[4] = {
            { -1.f, -1.f, 0.5f, 0.f, 0.f },
            {  1.f, -1.f, 0.5f, 1.f, 0.f },
            { -1.f,  1.f, 0.5f, 0.f, 1.f },
            {  1.f,  1.f, 0.5f, 1.f, 1.f },
        };
        BufferDesc vertexBufferDesc;
        vertexBufferDesc.Name = "Gamma Correct VB";
        vertexBufferDesc.BindFlags = BIND_VERTEX_BUFFER;
        vertexBufferDesc.Size = 4 * sizeof(UVQuadVert);
        vertexBufferDesc.Usage = USAGE_IMMUTABLE;
        BufferData data;
        data.pData = kFixedVert;
        data.DataSize = sizeof(kFixedVert);
        data.pContext = m_pDevice->GetImmediateContext();
        renderDevice->CreateBuffer(vertexBufferDesc, &data, &m_pVertexBuffer);
        if (!m_pVertexBuffer)
            LSTG_THROW(GammaCorrectHelperInitializeFailedException, "Create VB fail");
    }
    {
        /**
         * 0 -- 1
         * | \  |
         * |  \ |
         * 2 -- 3
         */
        const uint16_t kFixedIndex[] = { 0, 1, 3, 0, 3, 2 };
        BufferDesc indexBufferDesc;
        indexBufferDesc.Name = "Gamma Correct IB";
        indexBufferDesc.BindFlags = BIND_INDEX_BUFFER;
        indexBufferDesc.Size = 6 * sizeof(uint16_t);
        indexBufferDesc.Usage = USAGE_IMMUTABLE;
        BufferData data;
        data.pData = kFixedIndex;
        data.DataSize = sizeof(kFixedIndex);
        data.pContext = m_pDevice->GetImmediateContext();
        renderDevice->CreateBuffer(indexBufferDesc, &data, &m_pIndexBuffer);
        if (!m_pIndexBuffer)
            LSTG_THROW(GammaCorrectHelperInitializeFailedException, "Create IB fail");
    }
}

void GammaCorrectHelper::ResizeFrameBuffer() noexcept
{
    auto renderDevice = m_pDevice->GetDevice();
    auto swapChain = m_pDevice->GetSwapChain();

    // 重新创建 RT
    TextureDesc renderTargetDesc;
    renderTargetDesc.Name = "Gamma Correct Texture";
    renderTargetDesc.Type = Diligent::RESOURCE_DIM_TEX_2D;
    renderTargetDesc.Width = swapChain->GetDesc().Width;
    renderTargetDesc.Height = swapChain->GetDesc().Height;
    renderTargetDesc.MipLevels = 1;
    renderTargetDesc.BindFlags = Diligent::BIND_SHADER_RESOURCE | Diligent::BIND_RENDER_TARGET;
    renderTargetDesc.Usage = Diligent::USAGE_DEFAULT;
    renderTargetDesc.Format = swapChain->GetDesc().ColorBufferFormat;  // 需要和 SwapChain 一致
    renderTargetDesc.ClearValue.Format = renderTargetDesc.Format;
    renderTargetDesc.ClearValue.Color[0] = 0.f;
    renderTargetDesc.ClearValue.Color[1] = 0.f;
    renderTargetDesc.ClearValue.Color[2] = 0.f;
    renderTargetDesc.ClearValue.Color[3] = 0.f;

    RefCntAutoPtr<ITexture> renderTarget;
    renderDevice->CreateTexture(renderTargetDesc, nullptr, &renderTarget);
    if (!renderTarget)
    {
        LSTG_LOG_ERROR_CAT(GammaCorrectHelper, "Create render target after resize fail");
        return;
    }
    m_pColorFrameBuffer = renderTarget;
    m_pColorFrameBufferSRV = m_pColorFrameBuffer->GetDefaultView(TEXTURE_VIEW_SHADER_RESOURCE);
    m_pColorFrameBufferRTV = m_pColorFrameBuffer->GetDefaultView(TEXTURE_VIEW_RENDER_TARGET);
}

void GammaCorrectHelper::DrawFrameBuffer() noexcept
{
    auto context = m_pDevice->GetImmediateContext();
    auto swapChain = m_pDevice->GetSwapChain();

    // 设置 Shader 和 Buffer
    IBuffer* vertexBuffers[] = { m_pVertexBuffer };
    context->SetVertexBuffers(0, 1, vertexBuffers, nullptr, RESOURCE_STATE_TRANSITION_MODE_TRANSITION, SET_VERTEX_BUFFERS_FLAG_RESET);
    context->SetIndexBuffer(m_pIndexBuffer, 0, RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
    context->SetPipelineState(m_pPostEffectPSO);

    // 提交 SRB
    m_pColorFrameTextureVar->Set(m_pColorFrameBufferSRV);
    context->CommitShaderResources(m_pPostEffectSRB, RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

    // 总是使用整个屏幕
    Viewport vp;
    vp.Width = static_cast<float>(swapChain->GetDesc().Width);
    vp.Height = static_cast<float>(swapChain->GetDesc().Height);
    vp.MinDepth = 0.0f;
    vp.MaxDepth = 1.0f;
    vp.TopLeftX = vp.TopLeftY = 0;
    context->SetViewports(1, &vp, swapChain->GetDesc().Width, swapChain->GetDesc().Height);

    DrawIndexedAttribs drawAttrs { 6, VT_UINT16, DRAW_FLAG_VERIFY_STATES };
    context->DrawIndexed(drawAttrs);
}
