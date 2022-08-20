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
#include "Render/detail/Texture2DDataImpl.hpp"
#include "Render/GraphDef/detail/ToDiligent.hpp"
#include "Render/detail/ClearHelper.hpp"
#include "Render/detail/GammaCorrectHelper.hpp"
#include "Render/detail/ScreenCaptureHelper.hpp"
#include "Render/detail/RenderDevice/RenderDeviceGL.hpp"
#include "Render/detail/RenderDevice/RenderDeviceVulkan.hpp"
#include "Render/detail/RenderDevice/RenderDeviceD3D11.hpp"
#include "Render/detail/RenderDevice/RenderDeviceD3D12.hpp"

using namespace std;
using namespace lstg;
using namespace lstg::Subsystem;

LSTG_DEF_LOG_CATEGORY(RenderSystem);

static const unsigned kDefaultTexture2DWidth = 16;
static const unsigned kDefaultTexture2DHeight = 16;

namespace
{
#ifdef LSTG_PLATFORM_EMSCRIPTEN
    extern "C" void glEnable(unsigned int cap);
    extern "C" unsigned int glGetError();

    /**
     * 检查是否可以使用 SRGB FrameBuffer
     */
    bool IsSRGBFrameBufferAvailable() noexcept
    {
        ::glEnable(/* GL_FRAMEBUFFER_SRGB */ 0x8DB9);
        if (::glGetError() != /* GL_NO_ERROR */ 0)
            return false;
        return true;
    }
#endif

    /**
     * Diligent 调试日志转发
     */
    void DiligentDebugOutput(Diligent::DEBUG_MESSAGE_SEVERITY severity, const char* message, const char* function, const char* file,
        int line) noexcept
    {
        LogLevel level;
        switch (severity)
        {
            case Diligent::DEBUG_MESSAGE_SEVERITY_WARNING:
                level = LogLevel::Warn;
                break;
            case Diligent::DEBUG_MESSAGE_SEVERITY_ERROR:
                level = LogLevel::Error;
                break;
            case Diligent::DEBUG_MESSAGE_SEVERITY_FATAL_ERROR:
                level = LogLevel::Critical;
                break;
            case Diligent::DEBUG_MESSAGE_SEVERITY_INFO:
            default:
                level = LogLevel::Trace;
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
#if D3D12_SUPPORTED == 1
        out.emplace_back("D3D12", [](WindowSystem* windowSystem) -> Render::RenderDevicePtr {
            return make_shared<Render::detail::RenderDevice::RenderDeviceD3D12>(windowSystem);
        });
#endif
#if D3D11_SUPPORTED == 1
        out.emplace_back("D3D11", [](WindowSystem* windowSystem) -> Render::RenderDevicePtr {
            return make_shared<Render::detail::RenderDevice::RenderDeviceD3D11>(windowSystem);
        });
#endif
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

    Render::TexturePtr GenerateDefaultTexture2D(RenderSystem* self)
    {
        static const unsigned kBlockSize = 4;
        Render::Texture2DData data(kDefaultTexture2DWidth, kDefaultTexture2DHeight, Render::Texture2DFormats::R8G8B8A8);
        assert(data.GetStride() == kDefaultTexture2DWidth * 4);
        auto buffer = data.GetBuffer();
        for (size_t i = 0; i < kDefaultTexture2DHeight / kBlockSize; ++i)
            for (size_t j = 0; j < kDefaultTexture2DWidth / kBlockSize; ++j)
            {
                auto* start = &buffer[i * kBlockSize * kDefaultTexture2DWidth * 4 + j * kBlockSize * 4];
                start[0] = start[1] = start[2] = 0; start[3] = 0xFF;
                start[4] = start[5] = start[6] = 0; start[7] = 0xFF;
                start[8] = start[9] = start[10] = start[11] = 0xFF;
                start[12] = start[13] = start[14] = start[15] = 0xFF;
                start = &buffer[(i * kBlockSize + 1) * kDefaultTexture2DWidth * 4 + j * kBlockSize * 4];
                start[0] = start[1] = start[2] = 0; start[3] = 0xFF;
                start[4] = start[5] = start[6] = 0; start[7] = 0xFF;
                start[8] = start[9] = start[10] = start[11] = 0xFF;
                start[12] = start[13] = start[14] = start[15] = 0xFF;
                start = &buffer[(i * kBlockSize + 2) * kDefaultTexture2DWidth * 4 + j * kBlockSize * 4];
                start[0] = start[1] = start[2] = start[3] = 0xFF;
                start[4] = start[5] = start[6] = start[7] = 0xFF;
                start[8] = start[9] = start[10] = 0; start[11] = 0xFF;
                start[12] = start[13] = start[14] = 0; start[15] = 0xFF;
                start = &buffer[(i * kBlockSize + 3) * kDefaultTexture2DWidth * 4 + j * kBlockSize * 4];
                start[0] = start[1] = start[2] = start[3] = 0xFF;
                start[4] = start[5] = start[6] = start[7] = 0xFF;
                start[8] = start[9] = start[10] = 0; start[11] = 0xFF;
                start[12] = start[13] = start[14] = 0; start[15] = 0xFF;
            }
        auto ret = self->CreateTexture2D(data);
        return ret.ThrowIfError();
    }
}

glm::vec<2, uint32_t> RenderSystem::GetDefaultTexture2DSize() noexcept
{
    return { kDefaultTexture2DWidth, kDefaultTexture2DHeight };
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
    ret.ThrowIfError();

    // 创建默认纹理
    m_pDefaultTexture2D = GenerateDefaultTexture2D(this);

    // 创建清屏工具
    m_pClearHelper = make_shared<Render::detail::ClearHelper>(m_pRenderDevice.get());

#ifdef LSTG_PLATFORM_EMSCRIPTEN
    // 创建 Gamma 校准工具
    if (!IsSRGBFrameBufferAvailable())
    {
        LSTG_LOG_INFO_CAT(RenderSystem, "SRGB framebuffer not available, adding gamma correction post effect");
        m_pGammaCorrectHelper = make_shared<Render::detail::GammaCorrectHelper>(m_pRenderDevice.get());
    }
#endif

    // 创建截图工具
    m_pScreenCaptureHelper = make_shared<Render::detail::ScreenCaptureHelper>(m_pRenderDevice.get());
}

// <editor-fold desc="资源分配">

Result<Render::MeshPtr> RenderSystem::CreateDynamicMesh(const Render::GraphDef::MeshDefinition& def, bool use32BitIndex) noexcept
{
    if (def.GetVertexElements().empty() || def.GetVertexStride() == 0)
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
            vertexBufferDesc.Size = def.GetVertexStride();
            vertexBufferDesc.Usage = Diligent::USAGE_DYNAMIC;
            vertexBufferDesc.CPUAccessFlags = Diligent::CPU_ACCESS_WRITE;
            m_pRenderDevice->GetDevice()->CreateBuffer(vertexBufferDesc, nullptr, &vertexBuffer);
            if (!vertexBuffer)
                return make_error_code(errc::not_enough_memory);
        }

        // 创建 IndexBuffer
        Diligent::RefCntAutoPtr<Diligent::IBuffer> indexBuffer;
        {
            Diligent::BufferDesc indexBufferDesc;
            indexBufferDesc.BindFlags = Diligent::BIND_INDEX_BUFFER;
            indexBufferDesc.Size = use32BitIndex ? sizeof(uint32_t) : sizeof(uint16_t);
            indexBufferDesc.Usage = Diligent::USAGE_DYNAMIC;
            indexBufferDesc.CPUAccessFlags = Diligent::CPU_ACCESS_WRITE;
            m_pRenderDevice->GetDevice()->CreateBuffer(indexBufferDesc, nullptr, &indexBuffer);
            if (!indexBuffer)
                return make_error_code(errc::not_enough_memory);
        }

        // 创建 Mesh 对象
        return make_shared<Render::Mesh>(*m_pRenderDevice, sharedDef, vertexBuffer, indexBuffer, use32BitIndex,
            Render::Mesh::Usage::Dynamic);
    }
    catch (...)  // bad_alloc
    {
        return make_error_code(errc::not_enough_memory);
    }
}

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
        return make_shared<Render::Material>(*GetRenderDevice(), effect, m_pDefaultTexture2D);
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

Result<Render::TexturePtr> RenderSystem::CreateTexture2D(const Render::Texture2DData& data) noexcept
{
    Diligent::TextureDesc desc = data.m_pImpl->m_stDesc;
    desc.BindFlags = Diligent::BIND_SHADER_RESOURCE;
    desc.Usage = Diligent::USAGE_IMMUTABLE;
    desc.MipLevels = data.m_pImpl->m_stSubResources.size();
    assert(desc.MipLevels != 0);
    Diligent::TextureData texData;
    texData.pContext = m_pRenderDevice->GetImmediateContext();
    texData.NumSubresources = data.m_pImpl->m_stSubResources.size();
    texData.pSubResources = data.m_pImpl->m_stSubResources.data();
    Diligent::RefCntAutoPtr<Diligent::ITexture> texture;
    m_pRenderDevice->GetDevice()->CreateTexture(desc, &texData, &texture);
    if (!texture)
        return make_error_code(errc::not_enough_memory);
    return make_shared<Render::Texture>(*m_pRenderDevice, texture);
}

Result<Render::TexturePtr> RenderSystem::CreateDynamicTexture2D(uint32_t width, uint32_t height, Render::Texture2DFormats format) noexcept
{
    try
    {
        auto stride = Render::detail::AlignedScanLineSize(width * Render::detail::GetPixelComponentSize(format));

        // 用一个空纹理初始化
        vector<uint8_t> emptyTexture;
        emptyTexture.resize(stride * height);

        Diligent::TextureDesc desc;
        desc.Type = Diligent::RESOURCE_DIM_TEX_2D;
        desc.Width = width;
        desc.Height = height;
        desc.MipLevels = 1;
        desc.BindFlags = Diligent::BIND_SHADER_RESOURCE;
        desc.Usage = Diligent::USAGE_DEFAULT;
        desc.Format = Render::detail::ToDiligent(format);

        Diligent::TextureSubResData subResData;
        subResData.pData = emptyTexture.data();
        subResData.Stride = stride;

        Diligent::TextureData texData;
        texData.pContext = m_pRenderDevice->GetImmediateContext();
        texData.NumSubresources = 1;
        texData.pSubResources = &subResData;

        Diligent::RefCntAutoPtr<Diligent::ITexture> texture;
        m_pRenderDevice->GetDevice()->CreateTexture(desc, &texData, &texture);
        if (!texture)
            return make_error_code(errc::not_enough_memory);
        return make_shared<Render::Texture>(*m_pRenderDevice, texture);
    }
    catch (...)
    {
        return make_error_code(errc::not_enough_memory);
    }
}

Result<Render::TexturePtr> RenderSystem::CreateRenderTarget(uint32_t width, uint32_t height) noexcept
{
    Diligent::TextureDesc desc;
    desc.Type = Diligent::RESOURCE_DIM_TEX_2D;
    desc.Width = width;
    desc.Height = height;
    desc.MipLevels = 1;
    desc.BindFlags = Diligent::BIND_SHADER_RESOURCE | Diligent::BIND_RENDER_TARGET;
    desc.Usage = Diligent::USAGE_DEFAULT;
    desc.Format = m_pRenderDevice->GetSwapChain()->GetDesc().ColorBufferFormat;  // 需要和 SwapChain 一致
    desc.ClearValue.Format = desc.Format;
    desc.ClearValue.Color[0] = 0.f;
    desc.ClearValue.Color[1] = 0.f;
    desc.ClearValue.Color[2] = 0.f;
    desc.ClearValue.Color[3] = 0.f;

    Diligent::RefCntAutoPtr<Diligent::ITexture> texture;
    m_pRenderDevice->GetDevice()->CreateTexture(desc, nullptr, &texture);
    if (!texture)
        return make_error_code(errc::not_enough_memory);
    return make_shared<Render::Texture>(*m_pRenderDevice, texture);
}

Result<Render::TexturePtr> RenderSystem::CreateDepthStencil(uint32_t width, uint32_t height) noexcept
{
    Diligent::TextureDesc desc;
    desc.Type = Diligent::RESOURCE_DIM_TEX_2D;
    desc.Width = width;
    desc.Height = height;
    desc.MipLevels = 1;
    desc.BindFlags = Diligent::BIND_SHADER_RESOURCE | Diligent::BIND_DEPTH_STENCIL;
    desc.Usage = Diligent::USAGE_DEFAULT;
    desc.Format = m_pRenderDevice->GetSwapChain()->GetDesc().DepthBufferFormat;
    desc.ClearValue.Format = desc.Format;
    desc.ClearValue.DepthStencil.Depth = 1;
    desc.ClearValue.DepthStencil.Stencil = 0;

    Diligent::RefCntAutoPtr<Diligent::ITexture> texture;
    m_pRenderDevice->GetDevice()->CreateTexture(desc, nullptr, &texture);
    if (!texture)
        return make_error_code(errc::not_enough_memory);
    return make_shared<Render::Texture>(*m_pRenderDevice, texture);
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
        return make_shared<Render::Mesh>(*m_pRenderDevice, sharedDef, vertexBuffer, indexBuffer, use32BitIndex,
            Render::Mesh::Usage::Static);
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

#ifdef LSTG_PLATFORM_EMSCRIPTEN
    auto* renderTargetView = m_pGammaCorrectHelper ? m_pGammaCorrectHelper->GetRenderTargetView() : swapChain->GetCurrentBackBufferRTV();
#else
    auto* renderTargetView = swapChain->GetCurrentBackBufferRTV();
#endif
    auto* depthStencilView = swapChain->GetDepthBufferDSV();

    // 在帧开始切换 RT 到 SwapChain 上的 Buffer
    {
        context->SetRenderTargets(1, &renderTargetView, depthStencilView, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
        m_stCurrentOutputViews = {};
    }

    // 切换 VP 到全屏
    auto sz = GetCurrentOutputViewSize();
    {
        Diligent::Viewport vp;
        vp.MinDepth = 0.0f;
        vp.MaxDepth = 1.0f;
        vp.Width = static_cast<float>(std::get<0>(sz));
        vp.Height = static_cast<float>(std::get<1>(sz));
        vp.TopLeftX = 0.f;
        vp.TopLeftY = 0.f;
        context->SetViewports(1, &vp, 0, 0);
        m_stCurrentViewport = { 0.f, 0.f, vp.Width, vp.Height };  // 这里必须强制写出大小
    }

    // 提交裁剪状态
    {
        Diligent::Rect scissor {
            0,
            0,
            static_cast<int32_t>(std::get<0>(sz)),
            static_cast<int32_t>(std::get<1>(sz)),
        };
        context->SetScissorRects(1, &scissor, std::get<0>(sz), std::get<1>(sz));
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
    auto context = m_pRenderDevice->GetImmediateContext();

#ifdef LSTG_PLATFORM_EMSCRIPTEN
    // 切换到默认 RT，完成 Gamma 校准步骤
    if (m_pGammaCorrectHelper)
    {
        auto swapChain = m_pRenderDevice->GetSwapChain();
        auto* renderTargetView = swapChain->GetCurrentBackBufferRTV();
        auto* depthStencilView = swapChain->GetDepthBufferDSV();

        context->SetRenderTargets(1, &renderTargetView, depthStencilView, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
        m_pGammaCorrectHelper->DrawFrameBuffer();
    }
#endif

    // 此时需要撇去 RT，进行后续的截屏动作
    {
        context->SetRenderTargets(0, nullptr, nullptr, Diligent::RESOURCE_STATE_TRANSITION_MODE_NONE);
        m_stCurrentOutputViews = {};
    }

    // 执行截屏任务
    m_pScreenCaptureHelper->ProcessCaptureTasks();

    // 执行 Present
    m_pRenderDevice->Present();
}

Result<void> RenderSystem::CaptureScreen(std::function<void(Result<const Render::Texture2DData*>)> callback, bool clearAlpha) noexcept
{
    return m_pScreenCaptureHelper->AddCaptureTask(std::move(callback), clearAlpha);
}

void RenderSystem::SetCamera(Render::CameraPtr camera) noexcept
{
    if (camera == m_pCurrentCamera)
        return;
    if (camera)
        camera->m_bStateDirty = true;
    m_pCurrentCamera = std::move(camera);
}

void RenderSystem::SetMaterial(Render::MaterialPtr material) noexcept
{
    if (material == m_pCurrentMaterial)
        return;
    m_pCurrentMaterial = std::move(material);
    m_pCurrentPassGroup = nullptr;
}

std::string_view RenderSystem::GetRenderTag(std::string_view key) const noexcept
{
    auto it = m_stEffectRenderTag.find(key);
    if (it == m_stEffectRenderTag.end())
        return {};
    return it->second;
}

void RenderSystem::SetRenderTag(std::string_view key, std::string_view value)
{
    auto it = m_stEffectRenderTag.find(key);
    if (it == m_stEffectRenderTag.end())
    {
        if (!value.empty())
        {
            m_stEffectRenderTag.emplace(key, string{value});
            m_pCurrentPassGroup = nullptr;
        }
    }
    else
    {
        if (value.empty())
        {
            m_stEffectRenderTag.erase(it);
            m_pCurrentPassGroup = nullptr;
        }
        else if (value != it->second)
        {
            it->second = string{value};
            m_pCurrentPassGroup = nullptr;
        }
    }
}

void RenderSystem::SetEffectGroupSelectCallback(EffectGroupSelectCallback selector) noexcept
{
    m_stCurrentEffectGroupSelector = std::move(selector);
    m_pCurrentPassGroup = nullptr;
}

Result<void> RenderSystem::Clear(std::optional<Render::ColorRGBA32> clearColor, std::optional<float> clearZDepth,
    std::optional<float> clearStencil) noexcept
{
    if (!m_pCurrentCamera)
        return make_error_code(errc::invalid_argument);

    // 先提交 Camera
    auto ret = CommitCamera();
    if (!ret)
    {
        LSTG_LOG_ERROR_CAT(RenderSystem, "Commit camera state fail: {}", ret.GetError());
        return ret.GetError();
    }

    auto context = m_pRenderDevice->GetImmediateContext();
    auto swapChain = m_pRenderDevice->GetSwapChain();

    Diligent::ITextureView* renderTargetView = nullptr;
    Diligent::ITextureView* depthStencilView = nullptr;
    if (m_stCurrentOutputViews.ColorView)
    {
        renderTargetView = m_stCurrentOutputViews.ColorView->m_pNativeHandler->GetDefaultView(Diligent::TEXTURE_VIEW_RENDER_TARGET);
        assert(renderTargetView);
    }
    else
    {
#ifdef LSTG_PLATFORM_EMSCRIPTEN
        renderTargetView = m_pGammaCorrectHelper ? m_pGammaCorrectHelper->GetRenderTargetView() : swapChain->GetCurrentBackBufferRTV();
#else
        renderTargetView = swapChain->GetCurrentBackBufferRTV();
#endif
    }
    if (m_stCurrentOutputViews.DepthStencilView)
    {
        depthStencilView = m_stCurrentOutputViews.DepthStencilView->m_pNativeHandler->GetDefaultView(Diligent::TEXTURE_VIEW_DEPTH_STENCIL);
        assert(depthStencilView);
    }
    else
    {
        depthStencilView = swapChain->GetDepthBufferDSV();
    }

    // 全屏清可以直接清 RT
    auto sz = GetCurrentOutputViewSize();
    if (m_stCurrentViewport.Left == 0 && static_cast<float>(std::get<0>(sz)) == m_stCurrentViewport.Width &&
        m_stCurrentViewport.Top == 0 && static_cast<float>(std::get<1>(sz)) == m_stCurrentViewport.Height)
    {
        if (clearColor)
        {
            const float color[4] = { clearColor->r() / 255.f, clearColor->g() / 255.f, clearColor->b() / 255.f, clearColor->a() / 255.f };
            context->ClearRenderTarget(renderTargetView, color, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
        }
        if (clearZDepth || clearStencil)
        {
            auto flag = static_cast<Diligent::CLEAR_DEPTH_STENCIL_FLAGS>((clearZDepth ? Diligent::CLEAR_DEPTH_FLAG : 0) |
                                                                         (clearStencil ? Diligent::CLEAR_STENCIL_FLAG : 0));
            auto depth = clearZDepth ? *clearZDepth : 1.0f;
            auto stencil = clearStencil ? *clearStencil : 0;
            context->ClearDepthStencil(depthStencilView, flag, depth, stencil, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
        }
    }
    else
    {
        assert(!clearStencil);  // not implemented yet

        // 需要调用清屏工具
        if (clearColor && clearZDepth)
            m_pClearHelper->ClearDepthColor(*clearColor, *clearZDepth);
        else if (clearColor)
            m_pClearHelper->ClearColor(*clearColor);
        else if (clearZDepth)
            m_pClearHelper->ClearDepth(*clearZDepth);
    }

    return {};
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
        context->SetVertexBuffers(0, 1, vertexBuffers, vertexOffsets, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION,
            Diligent::SET_VERTEX_BUFFERS_FLAG_RESET);
        context->SetIndexBuffer(mesh->m_pIndexBuffer, indexOffset * (mesh->Is32BitsIndex() ? 4 : 2),
            Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
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
        auto texture = m_stCurrentOutputViews.ColorView->m_pNativeHandler;
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
    if (!m_stCurrentEffectGroupSelector)
        return groups[0].get();

    // 通知选择器进行选择
    auto group = m_stCurrentEffectGroupSelector(def.get(), m_stEffectRenderTag);

    // 没有选出来，则 fallback 到第一个结果
    if (!group)
        return groups[0].get();
    return group;
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
        Diligent::ITextureView* renderTargetView = nullptr;
        Diligent::ITextureView* depthStencilView = nullptr;
        if (outputViews.ColorView)
        {
            renderTargetView = outputViews.ColorView->m_pNativeHandler->GetDefaultView(Diligent::TEXTURE_VIEW_RENDER_TARGET);
            assert(renderTargetView);
        }
        else
        {
#ifdef LSTG_PLATFORM_EMSCRIPTEN
            renderTargetView = m_pGammaCorrectHelper ? m_pGammaCorrectHelper->GetRenderTargetView() : swapChain->GetCurrentBackBufferRTV();
#else
            renderTargetView = swapChain->GetCurrentBackBufferRTV();
#endif
        }
        if (outputViews.DepthStencilView)
        {
            depthStencilView = outputViews.DepthStencilView->m_pNativeHandler->GetDefaultView(Diligent::TEXTURE_VIEW_DEPTH_STENCIL);
            assert(depthStencilView);
        }
        else
        {
            depthStencilView = swapChain->GetDepthBufferDSV();
        }

        // 切换 RT
        context->SetRenderTargets(1, &renderTargetView, depthStencilView, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
        m_stCurrentOutputViews = outputViews;

        // 同时调整 Viewport
        forceUpdateViewport = true;
    }

    // 提交 Viewport
    auto sz = GetCurrentOutputViewSize();
    if (!(m_stCurrentViewport == viewport) || forceUpdateViewport)
    {
        if (!viewport.IsAutoViewport())
        {
            Diligent::Viewport vp;
            vp.MinDepth = 0.0f;
            vp.MaxDepth = 1.0f;
            vp.Width = ::floor(viewport.Width);  // 需要整数，glViewportIndexedf 可能不支持
            vp.Height = ::floor(viewport.Height);
            vp.TopLeftX = ::floor(viewport.Left);
            vp.TopLeftY = ::floor(viewport.Top);
            m_pRenderDevice->GetImmediateContext()->SetViewports(1, &vp, 0, 0);
            m_stCurrentViewport = viewport;
            viewportChanged = true;
        }
        else
        {
            Render::Camera::Viewport targetViewport = {0.f, 0.f, static_cast<float>(std::get<0>(sz)), static_cast<float>(std::get<1>(sz))};
            if (!(targetViewport == m_stCurrentViewport))
            {
                Diligent::Viewport vp;
                vp.MinDepth = 0.0f;
                vp.MaxDepth = 1.0f;
                vp.Width = ::floor(targetViewport.Width);  // 需要整数，glViewportIndexedf 可能不支持
                vp.Height = ::floor(targetViewport.Height);
                vp.TopLeftX = ::floor(targetViewport.Left);
                vp.TopLeftY = ::floor(targetViewport.Top);
                m_pRenderDevice->GetImmediateContext()->SetViewports(1, &vp, 0, 0);
                m_stCurrentViewport = viewport;
                viewportChanged = true;
            }
        }
    }

    // 提交裁剪状态
    {
        Diligent::Rect scissor {
            0,
            0,
            static_cast<int32_t>(std::get<0>(sz)),
            static_cast<int32_t>(std::get<1>(sz)),
        };
        context->SetScissorRects(1, &scissor, std::get<0>(sz), std::get<1>(sz));
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
    return m_pCurrentMaterial->Commit();
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
        m_stCurrentOutputViews.ColorView->m_pNativeHandler->GetDesc().Format);
    key.DepthBufferFormat = (m_stCurrentOutputViews.DepthStencilView == nullptr ? swapChain->GetDesc().DepthBufferFormat :
        m_stCurrentOutputViews.DepthStencilView->m_pNativeHandler->GetDesc().Format);
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
#ifdef LSTG_PLATFORM_EMSCRIPTEN
                m_pGammaCorrectHelper->ResizeFrameBuffer();
#endif
            }
        }
    }
}
