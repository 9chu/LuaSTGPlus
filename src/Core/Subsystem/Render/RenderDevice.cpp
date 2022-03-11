/**
 * @file
 * @date 2022/3/9
 * @author 9chu
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#include <lstg/Core/Subsystem/Render/RenderDevice.hpp>

#include <cassert>
#include <RenderDevice.h>
#include <DeviceContext.h>
#include <SwapChain.h>

using namespace std;
using namespace lstg;
using namespace lstg::Subsystem::Render;

RenderDevice::~RenderDevice() noexcept
{
    if (m_pSwapChain)
        m_pSwapChain->Release();
    if (m_pRenderContext)
        m_pRenderContext->Release();
    if (m_pRenderDevice)
        m_pRenderDevice->Release();
}

Diligent::IRenderDevice* RenderDevice::GetDevice() const noexcept
{
    return m_pRenderDevice;
}

Diligent::IDeviceContext* RenderDevice::GetImmediateContext() const noexcept
{
    return m_pRenderContext;
}

Diligent::ISwapChain* RenderDevice::GetSwapChain() const noexcept
{
    return m_pSwapChain;
}

bool RenderDevice::IsVerticalSyncEnabled() const noexcept
{
    return m_bVerticalSync;
}

void RenderDevice::SetVerticalSyncEnabled(bool enable) noexcept
{
    m_bVerticalSync = enable;
}

uint32_t RenderDevice::GetRenderOutputWidth() const noexcept
{
    assert(m_pSwapChain);
    return m_pSwapChain->GetDesc().Width;
}

uint32_t RenderDevice::GetRenderOutputHeight() const noexcept
{
    assert(m_pSwapChain);
    return m_pSwapChain->GetDesc().Height;
}

void RenderDevice::BeginRender() noexcept
{
    auto* renderTargetView = m_pSwapChain->GetCurrentBackBufferRTV();
    auto* depthView = m_pSwapChain->GetDepthBufferDSV();

    // 设置 RT
    m_pRenderContext->SetRenderTargets(1, &renderTargetView, depthView, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

    // 清空 RT
    static const float kClearColor[4] = {};
    m_pRenderContext->ClearRenderTarget(renderTargetView, kClearColor, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
    m_pRenderContext->ClearDepthStencil(depthView, Diligent::CLEAR_DEPTH_FLAG, 1.0f, 0, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
}

void RenderDevice::EndRenderAndPresent() noexcept
{
    auto* renderTargetView = m_pSwapChain->GetCurrentBackBufferRTV();
    auto* depthView = m_pSwapChain->GetDepthBufferDSV();

    // 再次设置 RT，防止渲染过程中的变动
    m_pRenderContext->SetRenderTargets(1, &renderTargetView, depthView, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

    Present();
}

void RenderDevice::Present() noexcept
{
    // 执行 Present
    m_pSwapChain->Present(m_bVerticalSync ? 1 : 0);
}
