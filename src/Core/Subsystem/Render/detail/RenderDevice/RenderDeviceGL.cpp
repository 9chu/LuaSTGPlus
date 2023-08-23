/**
 * @file
 * @date 2022/3/8
 * @author 9chu
 * 此文件为 LuaSTGPlus 项目的一部分，版权与许可声明详见 COPYRIGHT.txt。
 */
#include "RenderDeviceGL.hpp"

#include <SDL_syswm.h>
#include <SDL_system.h>

#if GL_SUPPORTED == 1 || GLES_SUPPORTED == 1
#include <EngineFactoryOpenGL.h>
#endif

#ifdef Status
#undef Status
#endif

#include <lstg/Core/AppBase.hpp>
#include <lstg/Core/Logging.hpp>

using namespace std;
using namespace lstg;
using namespace lstg::Subsystem::Render::detail::RenderDevice;
using namespace Diligent;

#if GL_SUPPORTED == 1 || GLES_SUPPORTED == 1

LSTG_DEF_LOG_CATEGORY(RenderDeviceGL);

RenderDeviceGL::RenderDeviceGL(WindowSystem* window)
{
    auto windowSize = window->GetRenderSize();

#ifndef LSTG_PLATFORM_EMSCRIPTEN
    // 获取 SDL 原生窗口
    assert(window);
    SDL_SysWMinfo systemWindowInfo;
    SDL_VERSION(&systemWindowInfo.version);
    if (SDL_FALSE == ::SDL_GetWindowWMInfo(window->GetNativeHandle(), &systemWindowInfo))
        LSTG_THROW(RenderDeviceInitializeFailedException, "SDL_GetWindowWMInfo fail, SDL_GetError: {}", SDL_GetError());
#endif

    // 创建上下文窗口
    NativeWindow nativeWindow;
#if defined(LSTG_PLATFORM_WIN32)
    nativeWindow = Win32NativeWindow {systemWindowInfo.info.win.window};
#elif defined(LSTG_PLATFORM_MACOS)
    // 创建 GLView
    m_stView = make_unique<OSX::GLView>(systemWindowInfo.info.cocoa.window, window->GetFeatures() & WindowFeatures::HighDPISupport);
    nativeWindow = MacOSNativeWindow {m_stView->GetView()};
#elif defined(LSTG_PLATFORM_EMSCRIPTEN)
    m_stView = make_unique<Emscripten::GLView>("#canvas");  // 固定为 #canvas
    nativeWindow = EmscriptenNativeWindow(m_stView->GetView());
#elif defined(LSTG_PLATFORM_LINUX)
#ifdef LSTG_X11_ENABLE
#if !SDL_VIDEO_DRIVER_X11
#error "Unexpected configuration error"
#endif
    // Create OpenGL Context
    m_stContext = make_unique<Linux::GLContext>(systemWindowInfo.info.x11.display, systemWindowInfo.info.x11.window);
    nativeWindow = LinuxNativeWindow { static_cast<Uint32>(systemWindowInfo.info.x11.window), systemWindowInfo.info.x11.display, nullptr };
#else
    LSTG_THROW(RenderDeviceInitializeFailedException, "Unsupported platform");
#endif
#elif defined(LSTG_PLATFORM_ANDROID)
    nativeWindow = AndroidNativeWindow {systemWindowInfo.info.android.window};
#else
    LSTG_THROW(RenderDeviceInitializeFailedException, "Unsupported platform");
#endif

    // 获取 Factory
    auto* factory = GetEngineFactoryOpenGL();
    assert(factory);

#if defined(LSTG_PLATFORM_ANDROID)
    // Diligent 依赖安卓文件系统初始化用于 ShaderFactory，虽然对我们来说没有作用，但是还是让他初始化吧
    auto assetManager = AppBase::GetInstance().GetAndroidAssetManager();
    if (!assetManager)
        LSTG_LOG_ERROR_CAT(RenderDeviceGL, "AssetManager is null, cannot initialize AndroidFileSystem");
    else
        factory->InitAndroidFileSystem(nullptr, nullptr, assetManager);
#endif

    // 创建引擎
    SwapChainDesc swapChainDesc;
    swapChainDesc.Width = std::get<0>(windowSize);
    swapChainDesc.Height = std::get<1>(windowSize);
    swapChainDesc.Usage |= SWAP_CHAIN_USAGE_COPY_SOURCE;  // 截屏需要
    EngineGLCreateInfo engineCreateInfo;
    engineCreateInfo.Window = nativeWindow;
    factory->CreateDeviceAndSwapChainGL(engineCreateInfo, &m_pRenderDevice, &m_pRenderContext, swapChainDesc, &m_pSwapChain);
    if (!m_pRenderDevice)
        LSTG_THROW(RenderDeviceInitializeFailedException, "Unable to initialize OpenGL render device");

#if defined(LSTG_PLATFORM_MACOS)
    // MacOS 需要手工 Resize 一下
    m_pSwapChain->Resize(swapChainDesc.Width, swapChainDesc.Height);
#endif
}

bool RenderDeviceGL::IsVerticalSyncEnabled() const noexcept
{
#if defined(LSTG_PLATFORM_EMSCRIPTEN)
    return true;
#else
    return RenderDevice::IsVerticalSyncEnabled();
#endif
}

void RenderDeviceGL::SetVerticalSyncEnabled(bool enable) noexcept
{
#if !defined(LSTG_PLATFORM_EMSCRIPTEN)
    RenderDevice::SetVerticalSyncEnabled(enable);
#endif
}

void RenderDeviceGL::Present() noexcept
{
#if defined(LSTG_PLATFORM_MACOS)
    m_stView->Present();
#elif defined(LSTG_PLATFORM_EMSCRIPTEN)
    m_stView->Present();
#else
    RenderDevice::Present();
#endif
}

#endif
