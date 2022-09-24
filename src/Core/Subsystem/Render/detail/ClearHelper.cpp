/**
* @file
* @date 2022/8/1
* @author 9chu
* 此文件为 LuaSTGPlus 项目的一部分，版权与许可声明详见 COPYRIGHT.txt。
*/
#include "ClearHelper.hpp"

#include <MapHelper.hpp>
#include <RenderDevice.h>
#include <SwapChain.h>

using namespace std;
using namespace lstg;
using namespace lstg::Subsystem::Render::detail;

using namespace Diligent;

struct QuadVert
{
    float x, y, z;
    uint8_t rgba[4];
};
static_assert(sizeof(QuadVert) == 3 * 4 + 4);

static const char* kViewportClearVertexShaderHLSL = R"(
    struct VSInput
    {
        float3 pos : ATTRIB0;
        float4 col : ATTRIB1;
    };

    struct PSInput
    {
        float4 pos : SV_POSITION;
        float4 col : COLOR;
    };

    void main(in VSInput VSIn, out PSInput PSIn)
    {
        PSIn.pos = float4(VSIn.pos.xyz, 1.0);
        PSIn.col = VSIn.col;
    }
)";

static const char* kViewportClearPixelShaderHLSL = R"(
    struct PSInput
    {
        float4 pos : SV_POSITION;
        float4 col : COLOR;
    };

    float4 main(in PSInput PSIn) : SV_Target
    {
        return PSIn.col;
    }
)";

ClearHelper::ClearHelper(RenderDevice* device)
    : m_pDevice(device)
{
    auto renderDevice = m_pDevice->GetDevice();
    auto swapChain = m_pDevice->GetSwapChain();

    // 创建 Shader
    RefCntAutoPtr<IShader> vertexShader;
    {
        ShaderCreateInfo shaderCreateInfo;
        shaderCreateInfo.UseCombinedTextureSamplers = true;
        shaderCreateInfo.SourceLanguage = SHADER_SOURCE_LANGUAGE_HLSL;
        shaderCreateInfo.Desc.ShaderType = SHADER_TYPE_VERTEX;
        shaderCreateInfo.Desc.Name = "Viewport Clear VS";
        shaderCreateInfo.Source = kViewportClearVertexShaderHLSL;
        renderDevice->CreateShader(shaderCreateInfo, &vertexShader);
        if (!vertexShader)
            LSTG_THROW(ClearHelperInitializeFailedException, "Create vertex shader fail");
    }

    RefCntAutoPtr<IShader> pixelShader;
    {
        ShaderCreateInfo shaderCreateInfo;
        shaderCreateInfo.UseCombinedTextureSamplers = true;
        shaderCreateInfo.SourceLanguage = SHADER_SOURCE_LANGUAGE_HLSL;
        shaderCreateInfo.Desc.ShaderType = SHADER_TYPE_PIXEL;
        shaderCreateInfo.Desc.Name = "Viewport Clear PS";
        shaderCreateInfo.Source = kViewportClearPixelShaderHLSL;
        renderDevice->CreateShader(shaderCreateInfo, &pixelShader);
        if (!pixelShader)
            LSTG_THROW(ClearHelperInitializeFailedException, "Create pixel shader fail");
    }

    // 创建 Pipeline
    {
        GraphicsPipelineStateCreateInfo pipelineCreateInfo;
        pipelineCreateInfo.PSODesc.Name = "Viewport Clear PSO";

        // 光栅化参数
        auto& graphicsPipeline = pipelineCreateInfo.GraphicsPipeline;
        graphicsPipeline.NumRenderTargets = 1;
        graphicsPipeline.RTVFormats[0] = swapChain->GetDesc().ColorBufferFormat;
        graphicsPipeline.DSVFormat = swapChain->GetDesc().DepthBufferFormat;
        graphicsPipeline.PrimitiveTopology = PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        graphicsPipeline.RasterizerDesc.CullMode = CULL_MODE_NONE;
        graphicsPipeline.RasterizerDesc.ScissorEnable = true;
        graphicsPipeline.DepthStencilDesc.DepthEnable = true;
        graphicsPipeline.DepthStencilDesc.DepthFunc = COMPARISON_FUNC_ALWAYS;

        // 混合模式
        auto& blendDesc = graphicsPipeline.BlendDesc.RenderTargets[0];
        blendDesc.BlendEnable = false;
        blendDesc.RenderTargetWriteMask = COLOR_MASK_ALL;

        // 顶点模式
        LayoutElement vertexInputLayout[] {
            {0, 0, 3, VT_FLOAT32},  // pos
            {1, 0, 4, VT_UINT8, true}  // col
        };
        graphicsPipeline.InputLayout.NumElements = std::extent_v<decltype(vertexInputLayout)>;
        graphicsPipeline.InputLayout.LayoutElements = vertexInputLayout;

        // Shader
        pipelineCreateInfo.pVS = vertexShader;
        pipelineCreateInfo.pPS = pixelShader;

        renderDevice->CreateGraphicsPipelineState(pipelineCreateInfo, &m_pColorDepthClearPSO);
        if (!m_pColorDepthClearPSO)
            LSTG_THROW(ClearHelperInitializeFailedException, "Create pipeline state fail");
    }
    {
        GraphicsPipelineStateCreateInfo pipelineCreateInfo;
        pipelineCreateInfo.PSODesc.Name = "Viewport Clear Color PSO";

        // 光栅化参数
        auto& graphicsPipeline = pipelineCreateInfo.GraphicsPipeline;
        graphicsPipeline.NumRenderTargets = 1;
        graphicsPipeline.RTVFormats[0] = swapChain->GetDesc().ColorBufferFormat;
        graphicsPipeline.DSVFormat = swapChain->GetDesc().DepthBufferFormat;
        graphicsPipeline.PrimitiveTopology = PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        graphicsPipeline.RasterizerDesc.CullMode = CULL_MODE_NONE;
        graphicsPipeline.RasterizerDesc.ScissorEnable = true;
        graphicsPipeline.DepthStencilDesc.DepthEnable = false;
        graphicsPipeline.DepthStencilDesc.DepthWriteEnable = false;

        // 混合模式
        auto& blendDesc = graphicsPipeline.BlendDesc.RenderTargets[0];
        blendDesc.BlendEnable = false;
        blendDesc.RenderTargetWriteMask = COLOR_MASK_ALL;

        // 顶点模式
        LayoutElement vertexInputLayout[] {
            {0, 0, 3, VT_FLOAT32},  // pos
            {1, 0, 4, VT_UINT8, true}  // col
        };
        graphicsPipeline.InputLayout.NumElements = std::extent_v<decltype(vertexInputLayout)>;
        graphicsPipeline.InputLayout.LayoutElements = vertexInputLayout;

        // Shader
        pipelineCreateInfo.pVS = vertexShader;
        pipelineCreateInfo.pPS = pixelShader;

        renderDevice->CreateGraphicsPipelineState(pipelineCreateInfo, &m_pColorOnlyClearPSO);
        if (!m_pColorOnlyClearPSO)
            LSTG_THROW(ClearHelperInitializeFailedException, "Create pipeline state fail");
    }
    {
        GraphicsPipelineStateCreateInfo pipelineCreateInfo;
        pipelineCreateInfo.PSODesc.Name = "Viewport Clear Depth PSO";

        // 光栅化参数
        auto& graphicsPipeline = pipelineCreateInfo.GraphicsPipeline;
        graphicsPipeline.NumRenderTargets = 1;
        graphicsPipeline.RTVFormats[0] = swapChain->GetDesc().ColorBufferFormat;
        graphicsPipeline.DSVFormat = swapChain->GetDesc().DepthBufferFormat;
        graphicsPipeline.PrimitiveTopology = PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        graphicsPipeline.RasterizerDesc.CullMode = CULL_MODE_NONE;
        graphicsPipeline.RasterizerDesc.ScissorEnable = true;
        graphicsPipeline.DepthStencilDesc.DepthEnable = true;
        graphicsPipeline.DepthStencilDesc.DepthFunc = COMPARISON_FUNC_ALWAYS;

        // 混合模式
        auto& blendDesc = graphicsPipeline.BlendDesc.RenderTargets[0];
        blendDesc.BlendEnable = false;
        blendDesc.RenderTargetWriteMask = COLOR_MASK_NONE;

        // 顶点模式
        LayoutElement vertexInputLayout[] {
            {0, 0, 3, VT_FLOAT32},  // pos
            {1, 0, 4, VT_UINT8, true}  // col
        };
        graphicsPipeline.InputLayout.NumElements = std::extent_v<decltype(vertexInputLayout)>;
        graphicsPipeline.InputLayout.LayoutElements = vertexInputLayout;

        // Shader
        pipelineCreateInfo.pVS = vertexShader;
        pipelineCreateInfo.pPS = pixelShader;

        renderDevice->CreateGraphicsPipelineState(pipelineCreateInfo, &m_pDepthOnlyClearPSO);
        if (!m_pDepthOnlyClearPSO)
            LSTG_THROW(ClearHelperInitializeFailedException, "Create pipeline state fail");
    }

    // 创建 VB&IB
    {
        BufferDesc vertexBufferDesc;
        vertexBufferDesc.Name = "Viewport Clear VB";
        vertexBufferDesc.BindFlags = BIND_VERTEX_BUFFER;
        vertexBufferDesc.Size = 4 * sizeof(QuadVert);
        vertexBufferDesc.Usage = USAGE_DYNAMIC;
        vertexBufferDesc.CPUAccessFlags = CPU_ACCESS_WRITE;
        renderDevice->CreateBuffer(vertexBufferDesc, nullptr, &m_pVertexBuffer);
        if (!m_pVertexBuffer)
            LSTG_THROW(ClearHelperInitializeFailedException, "Create VB fail");
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
        indexBufferDesc.Name = "Viewport Clear IB";
        indexBufferDesc.BindFlags = BIND_INDEX_BUFFER;
        indexBufferDesc.Size = 6 * sizeof(uint16_t);
        indexBufferDesc.Usage = USAGE_IMMUTABLE;
        BufferData data;
        data.pData = kFixedIndex;
        data.DataSize = sizeof(kFixedIndex);
        data.pContext = m_pDevice->GetImmediateContext();
        renderDevice->CreateBuffer(indexBufferDesc, &data, &m_pIndexBuffer);
        if (!m_pIndexBuffer)
            LSTG_THROW(ClearHelperInitializeFailedException, "Create IB fail");
    }
}

void ClearHelper::ClearColor(ColorRGBA32 color) noexcept
{
    auto renderContext = m_pDevice->GetImmediateContext();

    // 准备顶点
    {
        MapHelper<QuadVert> vertices(renderContext, m_pVertexBuffer, MAP_WRITE, MAP_FLAG_DISCARD);
        QuadVert* vertexDest = vertices;
        vertexDest[0].x = -1.f; vertexDest[0].y = -1.f; vertexDest[0].z = 0.5f;
        vertexDest[1].x = 1.f; vertexDest[1].y = -1.f; vertexDest[1].z = 0.5f;
        vertexDest[2].x = -1.f; vertexDest[2].y = 1.f; vertexDest[2].z = 0.5f;
        vertexDest[3].x = 1.f; vertexDest[3].y = 1.f; vertexDest[3].z = 0.5f;
        for (int i = 0; i < 4; ++i)
        {
            vertexDest[i].rgba[0] = color.r();
            vertexDest[i].rgba[1] = color.g();
            vertexDest[i].rgba[2] = color.b();
            vertexDest[i].rgba[3] = color.a();
        }
    }

    // 设置 Shader 和 Buffer
    IBuffer* vertexBuffers[] = { m_pVertexBuffer };
    renderContext->SetVertexBuffers(0, 1, vertexBuffers, nullptr, RESOURCE_STATE_TRANSITION_MODE_TRANSITION, SET_VERTEX_BUFFERS_FLAG_RESET);
    renderContext->SetIndexBuffer(m_pIndexBuffer, 0, RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
    renderContext->SetPipelineState(m_pColorOnlyClearPSO);
    DrawIndexedAttribs drawAttrs { 6, VT_UINT16, DRAW_FLAG_VERIFY_STATES };
    renderContext->DrawIndexed(drawAttrs);
}

void ClearHelper::ClearDepth(float depth) noexcept
{
    auto renderContext = m_pDevice->GetImmediateContext();

    // 准备顶点
    {
        MapHelper<QuadVert> vertices(renderContext, m_pVertexBuffer, MAP_WRITE, MAP_FLAG_DISCARD);
        QuadVert* vertexDest = vertices;
        vertexDest[0].x = -1.f; vertexDest[0].y = -1.f; vertexDest[0].z = depth;
        vertexDest[1].x = 1.f; vertexDest[1].y = -1.f; vertexDest[1].z = depth;
        vertexDest[2].x = -1.f; vertexDest[2].y = 1.f; vertexDest[2].z = depth;
        vertexDest[3].x = 1.f; vertexDest[3].y = 1.f; vertexDest[3].z = depth;
        for (int i = 0; i < 4; ++i)
        {
            vertexDest[i].rgba[0] = 0;
            vertexDest[i].rgba[1] = 0;
            vertexDest[i].rgba[2] = 0;
            vertexDest[i].rgba[3] = 0;
        }
    }

    // 设置 Shader 和 Buffer
    IBuffer* vertexBuffers[] = { m_pVertexBuffer };
    renderContext->SetVertexBuffers(0, 1, vertexBuffers, nullptr, RESOURCE_STATE_TRANSITION_MODE_TRANSITION, SET_VERTEX_BUFFERS_FLAG_RESET);
    renderContext->SetIndexBuffer(m_pIndexBuffer, 0, RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
    renderContext->SetPipelineState(m_pColorOnlyClearPSO);
    DrawIndexedAttribs drawAttrs { 6, VT_UINT16, DRAW_FLAG_VERIFY_STATES };
    renderContext->DrawIndexed(drawAttrs);
}

void ClearHelper::ClearDepthColor(ColorRGBA32 color, float depth) noexcept
{
    auto renderContext = m_pDevice->GetImmediateContext();

    // 准备顶点
    {
        MapHelper<QuadVert> vertices(renderContext, m_pVertexBuffer, MAP_WRITE, MAP_FLAG_DISCARD);
        QuadVert* vertexDest = vertices;
        vertexDest[0].x = -1.f; vertexDest[0].y = -1.f; vertexDest[0].z = depth;
        vertexDest[1].x = 1.f; vertexDest[1].y = -1.f; vertexDest[1].z = depth;
        vertexDest[2].x = -1.f; vertexDest[2].y = 1.f; vertexDest[2].z = depth;
        vertexDest[3].x = 1.f; vertexDest[3].y = 1.f; vertexDest[3].z = depth;
        for (int i = 0; i < 4; ++i)
        {
            vertexDest[i].rgba[0] = color.r();
            vertexDest[i].rgba[1] = color.g();
            vertexDest[i].rgba[2] = color.b();
            vertexDest[i].rgba[3] = color.a();
        }
    }

    // 设置 Shader 和 Buffer
    IBuffer* vertexBuffers[] = { m_pVertexBuffer };
    renderContext->SetVertexBuffers(0, 1, vertexBuffers, nullptr, RESOURCE_STATE_TRANSITION_MODE_TRANSITION, SET_VERTEX_BUFFERS_FLAG_RESET);
    renderContext->SetIndexBuffer(m_pIndexBuffer, 0, RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
    renderContext->SetPipelineState(m_pColorDepthClearPSO);
    DrawIndexedAttribs drawAttrs { 6, VT_UINT16, DRAW_FLAG_VERIFY_STATES };
    renderContext->DrawIndexed(drawAttrs);
}
