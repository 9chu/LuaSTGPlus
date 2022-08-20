/**
 * @file
 * @date 2022/8/20
 * @author 9chu
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#include "RenderDeviceD3D12.hpp"

#include <SDL_syswm.h>

#if D3D12_SUPPORTED == 1
#include <EngineFactoryD3D12.h>
#endif

using namespace std;
using namespace lstg;
using namespace lstg::Subsystem::Render::detail::RenderDevice;
using namespace Diligent;

#if D3D12_SUPPORTED == 1

RenderDeviceD3D12::RenderDeviceD3D12(WindowSystem* window)
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
    auto* factory = GetEngineFactoryD3D12();
    assert(factory);
    if (!factory->LoadD3D12())
        LSTG_THROW(RenderDeviceInitializeFailedException, "LoadD3D12 fail");

    // 创建引擎
    SwapChainDesc swapChainDesc;
    swapChainDesc.Width = std::get<0>(windowSize);
    swapChainDesc.Height = std::get<1>(windowSize);
    swapChainDesc.Usage |= SWAP_CHAIN_USAGE_COPY_SOURCE;  // 截屏需要
    EngineD3D12CreateInfo engineCreateInfo;
    engineCreateInfo.GraphicsAPIVersion = {11, 0};
    engineCreateInfo.NumDynamicHeapPagesToReserve = 16 * 1024 * 1024 / engineCreateInfo.DynamicHeapPageSize; // 16MB 动态内存空间，用于存放 VertexBuffer
#if !(defined(NDEBUG) || defined(LSTG_SHIPPING))
    engineCreateInfo.SetValidationLevel(VALIDATION_LEVEL_2);
#endif
    factory->CreateDeviceAndContextsD3D12(engineCreateInfo, &m_pRenderDevice, &m_pRenderContext);
    if (!m_pRenderDevice)
        LSTG_THROW(RenderDeviceInitializeFailedException, "Unable to initialize D3D12 render device");
    factory->CreateSwapChainD3D12(m_pRenderDevice, m_pRenderContext, swapChainDesc, FullScreenModeDesc{}, nativeWindow, &m_pSwapChain);
    if (!m_pSwapChain)
        LSTG_THROW(RenderDeviceInitializeFailedException, "Unable to initialize D3D12 swap chain");
}

#endif
