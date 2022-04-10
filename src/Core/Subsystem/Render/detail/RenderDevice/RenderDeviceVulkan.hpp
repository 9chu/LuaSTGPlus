/**
 * @file
 * @date 2022/3/9
 * @author 9chu
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#pragma once
#include <RenderDevice.h>
#include <DeviceContext.h>
#include <SwapChain.h>
#include <RefCntAutoPtr.hpp>
#include <lstg/Core/Subsystem/WindowSystem.hpp>
#include <lstg/Core/Subsystem/Render/RenderDevice.hpp>
#include "OSX/MetalView.hpp"

namespace lstg::Subsystem::Render::detail::RenderDevice
{
    /**
     * Vulkan 渲染设备
     */
    class RenderDeviceVulkan :
        public Render::RenderDevice
    {
    public:
        RenderDeviceVulkan(WindowSystem* window);

    private:
#ifdef LSTG_PLATFORM_MACOS
        // Diligent 使用 Metal 模拟 Vulkan，原生 Metal 支持需要商用许可
        std::unique_ptr<OSX::MetalView> m_stView;
#endif
    };
}