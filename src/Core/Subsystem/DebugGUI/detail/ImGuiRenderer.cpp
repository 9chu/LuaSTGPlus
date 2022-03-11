/**
 * @file
 * @date 2022/3/9
 * @author 9chu
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#include "ImGuiRenderer.hpp"

#include <BasicMath.hpp>
#include <SwapChain.h>
#include <MapHelper.hpp>
#include <lstg/Core/Logging.hpp>

using namespace std;
using namespace lstg;
using namespace lstg::Subsystem::DebugGUI::detail;
using namespace Diligent;

LSTG_DEF_LOG_CATEGORY(ImGuiRenderer);

namespace
{
    /**
     * 计算不小于 n 的最近二次幂
     */
    constexpr unsigned int NextPowerOf2(unsigned int n) noexcept
    {
        n--;
        n |= n >> 1;
        n |= n >> 2;
        n |= n >> 4;
        n |= n >> 8;
        n |= n >> 16;
        n++;
        return n;
    }

    float4 TransformClipRect(SURFACE_TRANSFORM preTransform, const ImVec2& displaySize, const float4& rect) noexcept
    {
        switch (preTransform)
        {
            case SURFACE_TRANSFORM_IDENTITY:
                return rect;
            case SURFACE_TRANSFORM_ROTATE_90:
                {
                    // The image content is rotated 90 degrees clockwise. The origin is in the left-top corner.
                    //
                    //                                                             DsplSz.y
                    //                a.x                                            -a.y     a.y     Old origin
                    //              0---->|                                       0------->|<------| /
                    //           0__|_____|____________________                0__|________|_______|/
                    //            | |     '                    |                | |        '       |
                    //        a.y | |     '                    |            a.x | |        '       |
                    //           _V_|_ _ _a____b               |               _V_|_ _d'___a'      |
                    //            A |     |    |               |                  |   |    |       |
                    //  DsplSz.y  | |     |____|               |                  |   |____|       |
                    //    -a.y    | |     d    c               |                  |   c'   b'      |
                    //           _|_|__________________________|                  |                |
                    //              A                                             |                |
                    //              |-----> Y'                                    |                |
                    //         New Origin                                         |________________|
                    //
                    float2 a {rect.x, rect.y};
                    float2 c {rect.z, rect.w};
                    return float4 {
                        displaySize.y - c.y, // min_x = c'.x
                        a.x,                 // min_y = a'.y
                        displaySize.y - a.y, // max_x = a'.x
                        c.x                  // max_y = c'.y
                    };
                }
            case SURFACE_TRANSFORM_ROTATE_180:
                {
                    // The image content is rotated 180 degrees clockwise. The origin is in the left-top corner.
                    //
                    //                a.x                                               DsplSz.x - a.x
                    //              0---->|                                         0------------------>|
                    //           0__|_____|____________________                 0_ _|___________________|______
                    //            | |     '                    |                  | |                   '      |
                    //        a.y | |     '                    |        DsplSz.y  | |              c'___d'     |
                    //           _V_|_ _ _a____b               |          -a.y    | |              |    |      |
                    //              |     |    |               |                 _V_|_ _ _ _ _ _ _ |____|      |
                    //              |     |____|               |                    |              b'   a'     |
                    //              |     d    c               |                    |                          |
                    //              |__________________________|                    |__________________________|
                    //                                         A                                               A
                    //                                         |                                               |
                    //                                     New Origin                                      Old Origin
                    float2 a{rect.x, rect.y};
                    float2 c{rect.z, rect.w};
                    return float4 {
                        displaySize.x - c.x, // min_x = c'.x
                        displaySize.y - c.y, // min_y = c'.y
                        displaySize.x - a.x, // max_x = a'.x
                        displaySize.y - a.y  // max_y = a'.y
                    };
                }
            case SURFACE_TRANSFORM_ROTATE_270:
                {
                    // The image content is rotated 270 degrees clockwise. The origin is in the left-top corner.
                    //
                    //              0  a.x     DsplSz.x-a.x   New Origin              a.y
                    //              |---->|<-------------------|                    0----->|
                    //          0_ _|_____|____________________V                 0 _|______|_________
                    //            | |     '                    |                  | |      '         |
                    //            | |     '                    |                  | |      '         |
                    //        a.y_V_|_ _ _a____b               |        DsplSz.x  | |      '         |
                    //              |     |    |               |          -a.x    | |      '         |
                    //              |     |____|               |                  | |      b'___c'   |
                    //              |     d    c               |                  | |      |    |    |
                    //  DsplSz.y _ _|__________________________|                 _V_|_ _ _ |____|    |
                    //                                                              |      a'   d'   |
                    //                                                              |                |
                    //                                                              |________________|
                    //                                                              A
                    //                                                              |
                    //                                                            Old origin
                    float2 a{rect.x, rect.y};
                    float2 c{rect.z, rect.w};
                    return float4 {
                        a.y,                 // min_x = a'.x
                        displaySize.x - c.x, // min_y = c'.y
                        c.y,                 // max_x = c'.x
                        displaySize.x - a.x  // max_y = a'.y
                    };
                }
            case SURFACE_TRANSFORM_OPTIMAL:
                LSTG_LOG_ERROR_CAT(ImGuiRenderer, "SURFACE_TRANSFORM_OPTIMAL is invalid");
                assert(false);
                return rect;
            case SURFACE_TRANSFORM_HORIZONTAL_MIRROR:
            case SURFACE_TRANSFORM_HORIZONTAL_MIRROR_ROTATE_90:
            case SURFACE_TRANSFORM_HORIZONTAL_MIRROR_ROTATE_180:
            case SURFACE_TRANSFORM_HORIZONTAL_MIRROR_ROTATE_270:
                LSTG_LOG_ERROR_CAT(ImGuiRenderer, "Mirror transforms are not supported");
                return rect;
            default:
                assert(false);
                return rect;
        }
    }

    // <editor-fold desc="HLSL">

    const char* kImGuiVertexShaderHLSL = R"(
cbuffer Constants
{
    float4x4 ProjectionMatrix;
}

struct VSInput
{
    float2 pos : ATTRIB0;
    float2 uv  : ATTRIB1;
    float4 col : ATTRIB2;
};

struct PSInput
{
    float4 pos : SV_POSITION;
    float4 col : COLOR;
    float2 uv  : TEXCOORD;
};

void main(in VSInput VSIn, out PSInput PSIn)
{
    PSIn.pos = mul(ProjectionMatrix, float4(VSIn.pos.xy, 0.0, 1.0));
    PSIn.col = VSIn.col;
    PSIn.uv  = VSIn.uv;
}
)";

    const char* kImGuiPixelShaderHLSL = R"(
struct PSInput
{
    float4 pos : SV_POSITION;
    float4 col : COLOR;
    float2 uv  : TEXCOORD;
};

Texture2D    Texture;
SamplerState Texture_sampler;

float4 main(in PSInput PSIn) : SV_Target
{
    return PSIn.col * Texture.Sample(Texture_sampler, PSIn.uv);
}
)";

    // </editor-fold>
    // <editor-fold desc="GLSL">

    const char* kImGuiVertexShaderGLSL = R"(
#ifdef VULKAN
#   define BINDING(X) layout(binding=X)
#   define OUT_LOCATION(X) layout(location=X) // Requires separable programs
#else
#   define BINDING(X)
#   define OUT_LOCATION(X)
#endif
BINDING(0) uniform Constants
{
    mat4 ProjectionMatrix;
};

layout(location = 0) in vec2 in_pos;
layout(location = 1) in vec2 in_uv;
layout(location = 2) in vec4 in_col;

OUT_LOCATION(0) out vec4 vsout_col;
OUT_LOCATION(1) out vec2 vsout_uv;

#ifndef GL_ES
out gl_PerVertex
{
    vec4 gl_Position;
};
#endif

void main()
{
    gl_Position = ProjectionMatrix * vec4(in_pos.xy, 0.0, 1.0);
    vsout_col = in_col;
    vsout_uv  = in_uv;
}
)";

    const char* kImGuiPixelShaderGLSL = R"(
#ifdef VULKAN
#   define BINDING(X) layout(binding=X)
#   define IN_LOCATION(X) layout(location=X) // Requires separable programs
#else
#   define BINDING(X)
#   define IN_LOCATION(X)
#endif
BINDING(0) uniform sampler2D Texture;

IN_LOCATION(0) in vec4 vsout_col;
IN_LOCATION(1) in vec2 vsout_uv;

layout(location = 0) out vec4 psout_col;

void main()
{
    psout_col = vsout_col * texture(Texture, vsout_uv);
}
)";

    // </editor-fold>
    // <editor-fold desc="SPIRV">

    // glslangValidator.exe -V -e main --vn VertexShader_SPIRV ImGUI.vert

    constexpr uint32_t kImGuiVertexShaderSPIRV[] = {
        0x07230203,0x00010000,0x0008000a,0x00000028,0x00000000,0x00020011,0x00000001,0x0006000b,
        0x00000001,0x4c534c47,0x6474732e,0x3035342e,0x00000000,0x0003000e,0x00000000,0x00000001,
        0x000b000f,0x00000000,0x00000004,0x6e69616d,0x00000000,0x0000000a,0x00000016,0x00000020,
        0x00000022,0x00000025,0x00000026,0x00030003,0x00000002,0x000001a4,0x00040005,0x00000004,
        0x6e69616d,0x00000000,0x00060005,0x00000008,0x505f6c67,0x65567265,0x78657472,0x00000000,
        0x00060006,0x00000008,0x00000000,0x505f6c67,0x7469736f,0x006e6f69,0x00030005,0x0000000a,
        0x00000000,0x00050005,0x0000000e,0x736e6f43,0x746e6174,0x00000073,0x00080006,0x0000000e,
        0x00000000,0x6a6f7250,0x69746365,0x614d6e6f,0x78697274,0x00000000,0x00030005,0x00000010,
        0x00000000,0x00040005,0x00000016,0x705f6e69,0x0000736f,0x00050005,0x00000020,0x756f7376,
        0x6f635f74,0x0000006c,0x00040005,0x00000022,0x635f6e69,0x00006c6f,0x00050005,0x00000025,
        0x756f7376,0x76755f74,0x00000000,0x00040005,0x00000026,0x755f6e69,0x00000076,0x00050048,
        0x00000008,0x00000000,0x0000000b,0x00000000,0x00030047,0x00000008,0x00000002,0x00040048,
        0x0000000e,0x00000000,0x00000005,0x00050048,0x0000000e,0x00000000,0x00000023,0x00000000,
        0x00050048,0x0000000e,0x00000000,0x00000007,0x00000010,0x00030047,0x0000000e,0x00000002,
        0x00040047,0x00000010,0x00000022,0x00000000,0x00040047,0x00000010,0x00000021,0x00000000,
        0x00040047,0x00000016,0x0000001e,0x00000000,0x00040047,0x00000020,0x0000001e,0x00000000,
        0x00040047,0x00000022,0x0000001e,0x00000002,0x00040047,0x00000025,0x0000001e,0x00000001,
        0x00040047,0x00000026,0x0000001e,0x00000001,0x00020013,0x00000002,0x00030021,0x00000003,
        0x00000002,0x00030016,0x00000006,0x00000020,0x00040017,0x00000007,0x00000006,0x00000004,
        0x0003001e,0x00000008,0x00000007,0x00040020,0x00000009,0x00000003,0x00000008,0x0004003b,
        0x00000009,0x0000000a,0x00000003,0x00040015,0x0000000b,0x00000020,0x00000001,0x0004002b,
        0x0000000b,0x0000000c,0x00000000,0x00040018,0x0000000d,0x00000007,0x00000004,0x0003001e,
        0x0000000e,0x0000000d,0x00040020,0x0000000f,0x00000002,0x0000000e,0x0004003b,0x0000000f,
        0x00000010,0x00000002,0x00040020,0x00000011,0x00000002,0x0000000d,0x00040017,0x00000014,
        0x00000006,0x00000002,0x00040020,0x00000015,0x00000001,0x00000014,0x0004003b,0x00000015,
        0x00000016,0x00000001,0x0004002b,0x00000006,0x00000018,0x00000000,0x0004002b,0x00000006,
        0x00000019,0x3f800000,0x00040020,0x0000001e,0x00000003,0x00000007,0x0004003b,0x0000001e,
        0x00000020,0x00000003,0x00040020,0x00000021,0x00000001,0x00000007,0x0004003b,0x00000021,
        0x00000022,0x00000001,0x00040020,0x00000024,0x00000003,0x00000014,0x0004003b,0x00000024,
        0x00000025,0x00000003,0x0004003b,0x00000015,0x00000026,0x00000001,0x00050036,0x00000002,
        0x00000004,0x00000000,0x00000003,0x000200f8,0x00000005,0x00050041,0x00000011,0x00000012,
        0x00000010,0x0000000c,0x0004003d,0x0000000d,0x00000013,0x00000012,0x0004003d,0x00000014,
        0x00000017,0x00000016,0x00050051,0x00000006,0x0000001a,0x00000017,0x00000000,0x00050051,
        0x00000006,0x0000001b,0x00000017,0x00000001,0x00070050,0x00000007,0x0000001c,0x0000001a,
        0x0000001b,0x00000018,0x00000019,0x00050091,0x00000007,0x0000001d,0x00000013,0x0000001c,
        0x00050041,0x0000001e,0x0000001f,0x0000000a,0x0000000c,0x0003003e,0x0000001f,0x0000001d,
        0x0004003d,0x00000007,0x00000023,0x00000022,0x0003003e,0x00000020,0x00000023,0x0004003d,
        0x00000014,0x00000027,0x00000026,0x0003003e,0x00000025,0x00000027,0x000100fd,0x00010038
    };

    constexpr uint32_t kImGuiFragmentShaderSPIRV[] = {
        0x07230203,0x00010000,0x0008000a,0x00000018,0x00000000,0x00020011,0x00000001,0x0006000b,
        0x00000001,0x4c534c47,0x6474732e,0x3035342e,0x00000000,0x0003000e,0x00000000,0x00000001,
        0x0008000f,0x00000004,0x00000004,0x6e69616d,0x00000000,0x00000009,0x0000000b,0x00000014,
        0x00030010,0x00000004,0x00000007,0x00030003,0x00000002,0x000001a4,0x00040005,0x00000004,
        0x6e69616d,0x00000000,0x00050005,0x00000009,0x756f7370,0x6f635f74,0x0000006c,0x00050005,
        0x0000000b,0x756f7376,0x6f635f74,0x0000006c,0x00040005,0x00000010,0x74786554,0x00657275,
        0x00050005,0x00000014,0x756f7376,0x76755f74,0x00000000,0x00040047,0x00000009,0x0000001e,
        0x00000000,0x00040047,0x0000000b,0x0000001e,0x00000000,0x00040047,0x00000010,0x00000022,
        0x00000000,0x00040047,0x00000010,0x00000021,0x00000000,0x00040047,0x00000014,0x0000001e,
        0x00000001,0x00020013,0x00000002,0x00030021,0x00000003,0x00000002,0x00030016,0x00000006,
        0x00000020,0x00040017,0x00000007,0x00000006,0x00000004,0x00040020,0x00000008,0x00000003,
        0x00000007,0x0004003b,0x00000008,0x00000009,0x00000003,0x00040020,0x0000000a,0x00000001,
        0x00000007,0x0004003b,0x0000000a,0x0000000b,0x00000001,0x00090019,0x0000000d,0x00000006,
        0x00000001,0x00000000,0x00000000,0x00000000,0x00000001,0x00000000,0x0003001b,0x0000000e,
        0x0000000d,0x00040020,0x0000000f,0x00000000,0x0000000e,0x0004003b,0x0000000f,0x00000010,
        0x00000000,0x00040017,0x00000012,0x00000006,0x00000002,0x00040020,0x00000013,0x00000001,
        0x00000012,0x0004003b,0x00000013,0x00000014,0x00000001,0x00050036,0x00000002,0x00000004,
        0x00000000,0x00000003,0x000200f8,0x00000005,0x0004003d,0x00000007,0x0000000c,0x0000000b,
        0x0004003d,0x0000000e,0x00000011,0x00000010,0x0004003d,0x00000012,0x00000015,0x00000014,
        0x00050057,0x00000007,0x00000016,0x00000011,0x00000015,0x00050085,0x00000007,0x00000017,
        0x0000000c,0x00000016,0x0003003e,0x00000009,0x00000017,0x000100fd,0x00010038
    };

    // </editor-fold>
    // <editor-fold desc="Metal">

    const char* kImGuiShadersMSL = R"(
#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

struct VSConstants
{
    float4x4 ProjectionMatrix;
};

struct VSIn
{
    float2 pos [[attribute(0)]];
    float2 uv  [[attribute(1)]];
    float4 col [[attribute(2)]];
};

struct VSOut
{
    float4 col [[user(locn0)]];
    float2 uv  [[user(locn1)]];
    float4 pos [[position]];
};

vertex VSOut vs_main(VSIn in [[stage_in]], constant VSConstants& Constants [[buffer(0)]])
{
    VSOut out = {};
    out.pos = Constants.ProjectionMatrix * float4(in.pos, 0.0, 1.0);
    out.col = in.col;
    out.uv  = in.uv;
    return out;
}

struct PSOut
{
    float4 col [[color(0)]];
};

fragment PSOut ps_main(VSOut in [[stage_in]],
                       texture2d<float> Texture [[texture(0)]],
                       sampler Texture_sampler  [[sampler(0)]])
{
    PSOut out = {};
    out.col = in.col * Texture.sample(Texture_sampler, in.uv);
    return out;
}
)";

    // </editor-fold>
}

ImGuiRenderer::ImGuiRenderer(Render::RenderDevice* deviceBridge, ImGuiIO& io)
    : m_pDeviceBridge(deviceBridge), m_stImGuiIO(io)
{
    auto renderDevice = m_pDeviceBridge->GetDevice();

    // 检查是否支持 Base Vertex Offset
    // https://www.khronos.org/registry/OpenGL/extensions/ARB/ARB_draw_elements_base_vertex.txt
    m_bBaseVertexSupported = renderDevice->GetAdapterInfo().DrawCommand.CapFlags & DRAW_COMMAND_CAP_FLAG_BASE_VERTEX;

    // 设置 ImGuiIO
    m_stImGuiIO.BackendRendererName = "LSTGPlus/Diligent";
    if (m_bBaseVertexSupported)
        m_stImGuiIO.BackendFlags |= ImGuiBackendFlags_RendererHasVtxOffset;

    CreateDeviceObjects();
}

void ImGuiRenderer::CreateDeviceObjects()
{
    auto renderDevice = m_pDeviceBridge->GetDevice();
    auto swapChain = m_pDeviceBridge->GetSwapChain();
    const auto deviceType = renderDevice->GetDeviceInfo().Type;

    // 创建 Shader
    RefCntAutoPtr<IShader> vertexShader;
    {
        ShaderCreateInfo shaderCreateInfo;
        shaderCreateInfo.UseCombinedTextureSamplers = true;
        shaderCreateInfo.SourceLanguage = SHADER_SOURCE_LANGUAGE_DEFAULT;
        shaderCreateInfo.Desc.ShaderType = SHADER_TYPE_VERTEX;
        shaderCreateInfo.Desc.Name = "ImGui VS";
        switch (deviceType)
        {
            case RENDER_DEVICE_TYPE_VULKAN:
                shaderCreateInfo.ByteCode = kImGuiVertexShaderSPIRV;
                shaderCreateInfo.ByteCodeSize = sizeof(kImGuiVertexShaderSPIRV);
                break;
            case RENDER_DEVICE_TYPE_D3D11:
            case RENDER_DEVICE_TYPE_D3D12:
                shaderCreateInfo.Source = kImGuiVertexShaderHLSL;
                break;
            case RENDER_DEVICE_TYPE_GL:
            case RENDER_DEVICE_TYPE_GLES:
                shaderCreateInfo.Source = kImGuiVertexShaderGLSL;
                break;
            case RENDER_DEVICE_TYPE_METAL:
                shaderCreateInfo.Source = kImGuiShadersMSL;
                shaderCreateInfo.EntryPoint = "vs_main";
                break;
            default:
                assert(false);
                LSTG_THROW(ImGuiRendererInitializeFailedException, "Unknown device {}", deviceType);
        }
        renderDevice->CreateShader(shaderCreateInfo, &vertexShader);
        if (!vertexShader)
            LSTG_THROW(ImGuiRendererInitializeFailedException, "Create vertex shader fail");
    }

    RefCntAutoPtr<IShader> pixelShader;
    {
        ShaderCreateInfo shaderCreateInfo;
        shaderCreateInfo.UseCombinedTextureSamplers = true;
        shaderCreateInfo.SourceLanguage = SHADER_SOURCE_LANGUAGE_DEFAULT;
        shaderCreateInfo.Desc.ShaderType = SHADER_TYPE_PIXEL;
        shaderCreateInfo.Desc.Name = "ImGui PS";
        switch (deviceType)
        {
            case RENDER_DEVICE_TYPE_VULKAN:
                shaderCreateInfo.ByteCode = kImGuiFragmentShaderSPIRV;
                shaderCreateInfo.ByteCodeSize = sizeof(kImGuiFragmentShaderSPIRV);
                break;
            case RENDER_DEVICE_TYPE_D3D11:
            case RENDER_DEVICE_TYPE_D3D12:
                shaderCreateInfo.Source = kImGuiPixelShaderHLSL;
                break;
            case RENDER_DEVICE_TYPE_GL:
            case RENDER_DEVICE_TYPE_GLES:
                shaderCreateInfo.Source = kImGuiPixelShaderGLSL;
                break;
            case RENDER_DEVICE_TYPE_METAL:
                shaderCreateInfo.Source = kImGuiShadersMSL;
                shaderCreateInfo.EntryPoint = "ps_main";
                break;
            default:
                assert(false);
                LSTG_THROW(ImGuiRendererInitializeFailedException, "Unknown device {}", deviceType);
        }
        renderDevice->CreateShader(shaderCreateInfo, &pixelShader);
        if (!pixelShader)
            LSTG_THROW(ImGuiRendererInitializeFailedException, "Create pixel shader fail");
    }

    // 创建 Pipeline
    {
        GraphicsPipelineStateCreateInfo pipelineCreateInfo;
        pipelineCreateInfo.PSODesc.Name = "ImGui PSO";

        // 光栅化参数
        auto& graphicsPipeline = pipelineCreateInfo.GraphicsPipeline;
        graphicsPipeline.NumRenderTargets = 1;
        graphicsPipeline.RTVFormats[0] = swapChain->GetDesc().ColorBufferFormat;
        graphicsPipeline.DSVFormat = swapChain->GetDesc().DepthBufferFormat;
        graphicsPipeline.PrimitiveTopology = PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        graphicsPipeline.RasterizerDesc.CullMode = CULL_MODE_NONE;
        graphicsPipeline.RasterizerDesc.ScissorEnable = true;
        graphicsPipeline.DepthStencilDesc.DepthEnable = false;

        // 混合模式
        auto& blendDesc = graphicsPipeline.BlendDesc.RenderTargets[0];
        blendDesc.BlendEnable = true;
        blendDesc.SrcBlend = BLEND_FACTOR_SRC_ALPHA;
        blendDesc.DestBlend = BLEND_FACTOR_INV_SRC_ALPHA;
        blendDesc.BlendOp = BLEND_OPERATION_ADD;
        blendDesc.SrcBlendAlpha = BLEND_FACTOR_INV_SRC_ALPHA;
        blendDesc.DestBlendAlpha = BLEND_FACTOR_ZERO;
        blendDesc.BlendOpAlpha = BLEND_OPERATION_ADD;
        blendDesc.RenderTargetWriteMask = COLOR_MASK_ALL;

        // 顶点模式
        LayoutElement vertexInputLayout[] {
            {0, 0, 2, VT_FLOAT32},  // pos
            {1, 0, 2, VT_FLOAT32},  // uv
            {2, 0, 4, VT_UINT8, true}  // col
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

        renderDevice->CreateGraphicsPipelineState(pipelineCreateInfo, &m_pPipelineState);
        if (!m_pPipelineState)
            LSTG_THROW(ImGuiRendererInitializeFailedException, "Create pipeline state fail");
    }

    // 创建 Shader Buffer
    {
        BufferDesc bufferDesc;
        bufferDesc.Size = sizeof(float4x4);
        bufferDesc.Usage = USAGE_DYNAMIC;
        bufferDesc.BindFlags = BIND_UNIFORM_BUFFER;
        bufferDesc.CPUAccessFlags = CPU_ACCESS_WRITE;
        renderDevice->CreateBuffer(bufferDesc, nullptr, &m_pVertexConstantBuffer);
        m_pPipelineState->GetStaticVariableByName(SHADER_TYPE_VERTEX, "Constants")->Set(m_pVertexConstantBuffer);
    }

    // 创建字体纹理
    {
        // 获取 ImGui 字体信息
        unsigned char* texData = nullptr;
        int texWidth = 0;
        int texHeight = 0;
        m_stImGuiIO.Fonts->GetTexDataAsRGBA32(&texData, &texWidth, &texHeight);

        TextureDesc fontTexDesc;
        fontTexDesc.Name = "ImGui Font Texture";
        fontTexDesc.Type = RESOURCE_DIM_TEX_2D;
        fontTexDesc.Width = static_cast<Uint32>(texWidth);
        fontTexDesc.Height = static_cast<Uint32>(texHeight);
        fontTexDesc.Format = TEX_FORMAT_RGBA8_UNORM;
        fontTexDesc.BindFlags = BIND_SHADER_RESOURCE;
        fontTexDesc.Usage = USAGE_IMMUTABLE;

        TextureSubResData mip0Data[] = {{texData, 4 * fontTexDesc.Width}};
        TextureData initData(mip0Data, std::extent_v<decltype(mip0Data)>);

        RefCntAutoPtr<ITexture> fontTexture;
        renderDevice->CreateTexture(fontTexDesc, &initData, &fontTexture);
        if (!fontTexture)
            LSTG_THROW(ImGuiRendererInitializeFailedException, "Create font texture fail");
        m_pFontTextureSRV = fontTexture->GetDefaultView(TEXTURE_VIEW_SHADER_RESOURCE);

        m_pPipelineState->CreateShaderResourceBinding(&m_pShaderResourceBinding, true);
        if (!m_pShaderResourceBinding)
            LSTG_THROW(ImGuiRendererInitializeFailedException, "Create shader resource binding fail");
        m_pTextureVar = m_pShaderResourceBinding->GetVariableByName(SHADER_TYPE_PIXEL, "Texture");
        assert(m_pTextureVar);

        m_stImGuiIO.Fonts->TexID = static_cast<ImTextureID>(m_pFontTextureSRV);
    }
}

void ImGuiRenderer::RenderDrawData(ImDrawData* drawData) noexcept
{
    auto renderDevice = m_pDeviceBridge->GetDevice();
    auto renderContext = m_pDeviceBridge->GetImmediateContext();
    auto swapChain = m_pDeviceBridge->GetSwapChain();
    auto surfaceWidth = swapChain->GetDesc().Width;
    auto surfaceHeight = swapChain->GetDesc().Height;
    auto preTransform = swapChain->GetDesc().PreTransform;

    // 最小化的时候不进行渲染
    if (drawData->DisplaySize.x <= 0.f || drawData->DisplaySize.y <= 0.f)
        return;

    // 创建 Vertex/Index 缓冲
    if (!m_pVertexBuffer || static_cast<int>(m_uVertexBufferSize) < drawData->TotalVtxCount)
    {
        m_pVertexBuffer.Release();

        // 计算需要的大小，总是取 2 的幂次
        auto desiredSize = ::max(16u, ::NextPowerOf2(drawData->TotalVtxCount));
        assert(!m_pVertexBuffer || desiredSize > m_uVertexBufferSize);
        m_uVertexBufferSize = desiredSize;

        BufferDesc vertexBufferDesc;
        vertexBufferDesc.Name = "ImGui vertex buffer";
        vertexBufferDesc.BindFlags = BIND_VERTEX_BUFFER;
        vertexBufferDesc.Size = m_uVertexBufferSize * sizeof(ImDrawVert);
        vertexBufferDesc.Usage = USAGE_DYNAMIC;
        vertexBufferDesc.CPUAccessFlags = CPU_ACCESS_WRITE;
        renderDevice->CreateBuffer(vertexBufferDesc, nullptr, &m_pVertexBuffer);
    }
    if (!m_pIndexBuffer || static_cast<int>(m_uIndexBufferSize) < drawData->TotalIdxCount)
    {
        m_pIndexBuffer.Release();

        // 计算需要的大小，总是取 2 的幂次
        auto desiredSize = ::max(16u, ::NextPowerOf2(drawData->TotalIdxCount));
        assert(!m_pIndexBuffer || desiredSize > m_uIndexBufferSize);
        m_uIndexBufferSize = desiredSize;

        BufferDesc indexBufferDesc;
        indexBufferDesc.Name = "ImGui index buffer";
        indexBufferDesc.BindFlags = BIND_INDEX_BUFFER;
        indexBufferDesc.Size = m_uIndexBufferSize * sizeof(ImDrawIdx);
        indexBufferDesc.Usage = USAGE_DYNAMIC;
        indexBufferDesc.CPUAccessFlags = CPU_ACCESS_WRITE;
        renderDevice->CreateBuffer(indexBufferDesc, nullptr, &m_pIndexBuffer);
    }
    if (!m_pVertexBuffer || !m_pIndexBuffer)
    {
        LSTG_LOG_ERROR_CAT(ImGuiRenderer, "Buffer not available");
        return;
    }

    // 复制顶点和索引数据
    {
        MapHelper<ImDrawVert> vertices(renderContext, m_pVertexBuffer, MAP_WRITE, MAP_FLAG_DISCARD);
        MapHelper<ImDrawIdx> indices(renderContext, m_pIndexBuffer, MAP_WRITE, MAP_FLAG_DISCARD);

        ImDrawVert* vertexDest = vertices;
        ImDrawIdx* indexDest = indices;
        for (int32_t cmdListId = 0; cmdListId < drawData->CmdListsCount; cmdListId++)
        {
            const auto* cmdList = drawData->CmdLists[cmdListId];
            ::memcpy(vertexDest, cmdList->VtxBuffer.Data, cmdList->VtxBuffer.Size * sizeof(ImDrawVert));
            ::memcpy(indexDest, cmdList->IdxBuffer.Data, cmdList->IdxBuffer.Size * sizeof(ImDrawIdx));
            vertexDest += cmdList->VtxBuffer.Size;
            indexDest += cmdList->IdxBuffer.Size;
        }
    }

    // 设置正交投影矩阵
    {
        float left = drawData->DisplayPos.x;
        float right = drawData->DisplayPos.x + drawData->DisplaySize.x;
        float top = drawData->DisplayPos.y;
        float bottom = drawData->DisplayPos.y + drawData->DisplaySize.y;

        float4x4 projection {
            2.0f / (right - left), 0.0f, 0.0f, 0.0f,
            0.0f, 2.0f / (top - bottom), 0.0f, 0.0f,
            0.0f, 0.0f, 0.5f, 0.0f,
            (right + left) / (left - right),  (top + bottom) / (bottom - top), 0.5f, 1.0f
        };

        // 处理屏幕旋转
        switch (preTransform)
        {
            case SURFACE_TRANSFORM_IDENTITY:
                break;
            case SURFACE_TRANSFORM_ROTATE_90:  // 90 度顺时针
                projection *= float4x4::RotationZ(-PI_F * 0.5f);
                break;
            case SURFACE_TRANSFORM_ROTATE_180:  // 180 度顺时针
                projection *= float4x4::RotationZ(-PI_F * 1.0f);
                break;
            case SURFACE_TRANSFORM_ROTATE_270:  // 270 度顺时针
                projection *= float4x4::RotationZ(-PI_F * 1.5f);
                break;
            case SURFACE_TRANSFORM_OPTIMAL:
                LSTG_LOG_ERROR_CAT(ImGuiRenderer, "SURFACE_TRANSFORM_OPTIMAL is invalid");
                assert(false);
                break;
            case SURFACE_TRANSFORM_HORIZONTAL_MIRROR:
            case SURFACE_TRANSFORM_HORIZONTAL_MIRROR_ROTATE_90:
            case SURFACE_TRANSFORM_HORIZONTAL_MIRROR_ROTATE_180:
            case SURFACE_TRANSFORM_HORIZONTAL_MIRROR_ROTATE_270:
                LSTG_LOG_ERROR_CAT(ImGuiRenderer, "Mirror transfroms are not supported");
                break;
            default:
                assert(false);
                break;
        }

        // 设置到 ConstantBuffer
        MapHelper<float4x4> constantBuffer(renderContext, m_pVertexConstantBuffer, MAP_WRITE, MAP_FLAG_DISCARD);
        *constantBuffer = projection;
    }

    // 设置渲染状态
    SetupRenderState(renderContext, surfaceWidth, surfaceHeight);

    // 渲染命令队列
    uint32_t globalIdxOffset = 0;
    uint32_t globalVtxOffset = 0;

    ITextureView* lastTextureView = nullptr;
    for (int32_t cmdListId = 0; cmdListId < drawData->CmdListsCount; cmdListId++)
    {
        const ImDrawList* cmdList = drawData->CmdLists[cmdListId];
        for (int32_t cmdId = 0; cmdId < cmdList->CmdBuffer.Size; cmdId++)
        {
            const ImDrawCmd* cmd = &cmdList->CmdBuffer[cmdId];
            if (cmd->UserCallback)
            {
                // 执行用户回调方法，见 ImDrawList::AddCallback
                // ImDrawCallback_ResetRenderState 需要特殊处理
                if (cmd->UserCallback == ImDrawCallback_ResetRenderState)
                    SetupRenderState(renderContext, surfaceWidth, surfaceHeight);
                else
                    cmd->UserCallback(cmdList, cmd);
            }
            else
            {
                // 计算裁切范围
                float4 clipRect {
                    (cmd->ClipRect.x - drawData->DisplayPos.x) * drawData->FramebufferScale.x,
                    (cmd->ClipRect.y - drawData->DisplayPos.y) * drawData->FramebufferScale.y,
                    (cmd->ClipRect.z - drawData->DisplayPos.x) * drawData->FramebufferScale.x,
                    (cmd->ClipRect.w - drawData->DisplayPos.y) * drawData->FramebufferScale.y
                };

                // 处理屏幕旋转
                clipRect = TransformClipRect(preTransform, drawData->DisplaySize, clipRect);

                // 设置裁切
                Rect scissor {
                    static_cast<Int32>(clipRect.x),
                    static_cast<Int32>(clipRect.y),
                    static_cast<Int32>(clipRect.z),
                    static_cast<Int32>(clipRect.w),
                };
                renderContext->SetScissorRects(1, &scissor, surfaceWidth, surfaceHeight);

                // 获取绑定的纹理
                auto* textureView = reinterpret_cast<ITextureView*>(cmd->TextureId);
                assert(textureView);
                if (textureView != lastTextureView)
                {
                    lastTextureView = textureView;
                    m_pTextureVar->Set(textureView);
                    renderContext->CommitShaderResources(m_pShaderResourceBinding, RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
                }

                DrawIndexedAttribs drawAttrs {
                    cmd->ElemCount,
                    sizeof(ImDrawIdx) == sizeof(Uint16) ? VT_UINT16 : VT_UINT32,
                    DRAW_FLAG_VERIFY_STATES
                };
                drawAttrs.FirstIndexLocation = cmd->IdxOffset + globalIdxOffset;
                if (m_bBaseVertexSupported)
                {
                    drawAttrs.BaseVertex = cmd->VtxOffset + globalVtxOffset;
                }
                else
                {
                    IBuffer* vertexBuffers[] = {m_pVertexBuffer};
                    Uint64 vertexOffsets[] = {sizeof(ImDrawVert) * (cmd->VtxOffset + globalVtxOffset)};
                    renderContext->SetVertexBuffers(0, 1, vertexBuffers, vertexOffsets, RESOURCE_STATE_TRANSITION_MODE_TRANSITION,
                        SET_VERTEX_BUFFERS_FLAG_NONE);
                }
                renderContext->DrawIndexed(drawAttrs);
            }
        }
        globalIdxOffset += cmdList->IdxBuffer.Size;
        globalVtxOffset += cmdList->VtxBuffer.Size;
    }
}

void ImGuiRenderer::SetupRenderState(Diligent::IDeviceContext* context, uint32_t surfaceWidth, uint32_t surfaceHeight) noexcept
{
    // 设置 Shader 和 Buffer
    IBuffer* vertexBuffers[] = { m_pVertexBuffer };
    context->SetVertexBuffers(0, 1, vertexBuffers, nullptr, RESOURCE_STATE_TRANSITION_MODE_TRANSITION, SET_VERTEX_BUFFERS_FLAG_RESET);
    context->SetIndexBuffer(m_pIndexBuffer, 0, RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
    context->SetPipelineState(m_pPipelineState);

    static const float kBlendFactor[4] = {0.f, 0.f, 0.f, 0.f};
    context->SetBlendFactors(kBlendFactor);

    // 总是使用整个屏幕
    Viewport vp;
    vp.Width = static_cast<float>(surfaceWidth);
    vp.Height = static_cast<float>(surfaceHeight);
    vp.MinDepth = 0.0f;
    vp.MaxDepth = 1.0f;
    vp.TopLeftX = vp.TopLeftY = 0;
    context->SetViewports(1, &vp, surfaceWidth, surfaceHeight);
}
