/**
 * @file
 * @date 2022/3/9
 * @author 9chu
 * 此文件为 LuaSTGPlus 项目的一部分，版权与许可声明详见 COPYRIGHT.txt。
 */
#include "RenderDeviceVulkan.hpp"

#include <vector>
#include <SDL_syswm.h>
#include <SDL_system.h>

#if VULKAN_SUPPORTED == 1
#include <EngineFactoryVk.h>
#endif

#if LSTG_X11_ENABLE
#include <X11/Xlib-xcb.h>
#endif

#include <lstg/Core/AppBase.hpp>
#include <lstg/Core/Logging.hpp>

using namespace std;
using namespace lstg;
using namespace lstg::Subsystem::Render::detail::RenderDevice;
using namespace Diligent;

#if VULKAN_SUPPORTED == 1

LSTG_DEF_LOG_CATEGORY(RenderDeviceVulkan);

RenderDeviceVulkan::RenderDeviceVulkan(WindowSystem* window)
{
    auto windowSize = window->GetRenderSize();

    // 获取 SDL 原生窗口
    assert(window);
    SDL_SysWMinfo systemWindowInfo;
    SDL_VERSION(&systemWindowInfo.version);
    if (SDL_FALSE == ::SDL_GetWindowWMInfo(window->GetNativeHandle(), &systemWindowInfo))
        LSTG_THROW(RenderDeviceInitializeFailedException, "SDL_GetWindowWMInfo fail, SDL_GetError: {}", SDL_GetError());

    // 创建上下文窗口
    NativeWindow nativeWindow;
#if defined(LSTG_PLATFORM_WIN32)
    nativeWindow = Win32NativeWindow {systemWindowInfo.info.win.window};
#elif defined(LSTG_PLATFORM_MACOS)
    // 创建 MetalView
    m_stView = make_unique<OSX::MetalView>(systemWindowInfo.info.cocoa.window, window->GetFeatures() & WindowFeatures::HighDPISupport);
    nativeWindow = MacOSNativeWindow {m_stView->GetLayer()};
#elif defined(LSTG_PLATFORM_LINUX)
#ifdef LSTG_X11_ENABLE
#if !SDL_VIDEO_DRIVER_X11
#error "Unexpected configuration error"
#endif
    m_pXCBConnection = ::XGetXCBConnection(systemWindowInfo.info.x11.display);
    nativeWindow = LinuxNativeWindow { static_cast<Uint32>(systemWindowInfo.info.x11.window), systemWindowInfo.info.x11.display,
        static_cast<::xcb_connection_t*>(m_pXCBConnection) };
#else
    LSTG_THROW(RenderDeviceInitializeFailedException, "Unsupported platform");
#endif
#elif defined(LSTG_PLATFORM_ANDROID)
    nativeWindow = AndroidNativeWindow {systemWindowInfo.info.android.window};
#else
    LSTG_THROW(RenderDeviceInitializeFailedException, "Unsupported platform");
#endif

    // 获取 Factory
    auto* factory = GetEngineFactoryVk();
    assert(factory);

#if defined(LSTG_PLATFORM_ANDROID)
    // Diligent 依赖安卓文件系统初始化用于 ShaderFactory，虽然对我们来说没有作用，但是还是让他初始化吧
    auto assetManager = AppBase::GetInstance().GetAndroidAssetManager();
    if (!assetManager)
        LSTG_LOG_ERROR_CAT(RenderDeviceVulkan, "AssetManager is null, cannot initialize AndroidFileSystem");
    else
        factory->InitAndroidFileSystem(nullptr, nullptr, assetManager);
#endif

    // 创建引擎
    SwapChainDesc swapChainDesc;
    swapChainDesc.Width = std::get<0>(windowSize);
    swapChainDesc.Height = std::get<1>(windowSize);
    swapChainDesc.Usage |= SWAP_CHAIN_USAGE_COPY_SOURCE;  // 截屏需要
#if LSTG_PLATFORM_MACOS
    // We need at least 3 buffers in Metal to avoid massive
    // performance degradation in full screen mode.
    // https://github.com/KhronosGroup/MoltenVK/issues/808
    swapChainDesc.BufferCount = 3;
#endif
    EngineVkCreateInfo engineCreateInfo;
#ifdef LSTG_DEVELOPMENT
    engineCreateInfo.SetValidationLevel(VALIDATION_LEVEL_1);
#endif
    engineCreateInfo.DynamicHeapSize = 16 * 1024 * 1024;  // 16MB 动态内存空间，用于存放 VertexBuffer
    factory->CreateDeviceAndContextsVk(engineCreateInfo, &m_pRenderDevice, &m_pRenderContext);
    if (!m_pRenderDevice)
        LSTG_THROW(RenderDeviceInitializeFailedException, "Unable to initialize Vulkan render device");
    if (!m_pSwapChain)
        factory->CreateSwapChainVk(m_pRenderDevice, m_pRenderContext, swapChainDesc, nativeWindow, &m_pSwapChain);
    if (!m_pSwapChain)
        LSTG_THROW(RenderDeviceInitializeFailedException, "Unable to initialize swap chain");
}

RenderDeviceVulkan::~RenderDeviceVulkan()
{
#ifdef LSTG_PLATFORM_LINUX
#ifdef LSTG_X11_ENABLE
    if (m_pXCBConnection)
        ::xcb_disconnect(static_cast<::xcb_connection_t*>(m_pXCBConnection));
#endif
#endif
}

#endif
