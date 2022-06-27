/**
 * @file
 * @date 2022/6/6
 * @author 9chu
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#include "RenderDeviceD3D11.hpp"

#include <SDL_syswm.h>

#if D3D11_SUPPORTED == 1
#include <EngineFactoryD3D11.h>
#endif

using namespace std;
using namespace lstg;
using namespace lstg::Subsystem::Render::detail::RenderDevice;
using namespace Diligent;

#if D3D11_SUPPORTED == 1

RenderDeviceD3D11::RenderDeviceD3D11(WindowSystem* window)
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
    nativeWindow = Win32NativeWindow {systemWindowInfo.info.win.window};

    // 获取 Factory
    auto* factory = GetEngineFactoryD3D11();
    assert(factory);

    // 创建引擎
    SwapChainDesc swapChainDesc;
    swapChainDesc.Width = std::get<0>(windowSize);
    swapChainDesc.Height = std::get<1>(windowSize);
    EngineD3D11CreateInfo engineCreateInfo;
    engineCreateInfo.GraphicsAPIVersion = {11, 0};
#if !(defined(NDEBUG) || defined(LSTG_SHIPPING))
    engineCreateInfo.SetValidationLevel(VALIDATION_LEVEL_2);
#endif
    factory->CreateDeviceAndContextsD3D11(engineCreateInfo, &m_pRenderDevice, &m_pRenderContext);
    if (!m_pRenderDevice)
        LSTG_THROW(RenderDeviceInitializeFailedException, "Unable to initialize D3D11 render device");
    factory->CreateSwapChainD3D11(m_pRenderDevice, m_pRenderContext, swapChainDesc, FullScreenModeDesc{}, nativeWindow, &m_pSwapChain);
    if (!m_pSwapChain)
        LSTG_THROW(RenderDeviceInitializeFailedException, "Unable to initialize D3D11 swap chain");
}

#endif
