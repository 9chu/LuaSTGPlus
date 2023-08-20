/**
 * @file
 * @date 2022/3/9
 * @author 9chu
 * 此文件为 LuaSTGPlus 项目的一部分，版权与许可声明详见 COPYRIGHT.txt。
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

SurfaceTransform RenderDevice::GetRenderOutputPreTransform() const noexcept
{
    assert(m_pSwapChain);

    switch (m_pSwapChain->GetDesc().PreTransform)
    {
        case Diligent::SURFACE_TRANSFORM_HORIZONTAL_MIRROR_ROTATE_90:  // FIXME: 处理镜像
        case Diligent::SURFACE_TRANSFORM_ROTATE_90:
            return SurfaceTransform::Rotate90;
        case Diligent::SURFACE_TRANSFORM_HORIZONTAL_MIRROR_ROTATE_180:  // FIXME: 处理镜像
        case Diligent::SURFACE_TRANSFORM_ROTATE_180:
            return SurfaceTransform::Rotate180;
        case Diligent::SURFACE_TRANSFORM_HORIZONTAL_MIRROR_ROTATE_270:  // FIXME: 处理镜像
        case Diligent::SURFACE_TRANSFORM_ROTATE_270:
            return SurfaceTransform::Rotate270;
        case Diligent::SURFACE_TRANSFORM_OPTIMAL:
        default:
            return SurfaceTransform::Identity;
    }
}

void RenderDevice::Present() noexcept
{
    // 执行 Present
    m_pSwapChain->Present(m_bVerticalSync ? 1 : 0);
    ++m_uPresentedCount;

#ifdef LSTG_PLATFORM_ANDROID
    if (m_pSwapChain)
    {
        // Diligent 需要在每一帧检查 Surface 大小变化
        // 在 Present 结束后进行动作
        m_pSwapChain->Resize(0, 0, Diligent::SURFACE_TRANSFORM_OPTIMAL);
    }
#endif
}
