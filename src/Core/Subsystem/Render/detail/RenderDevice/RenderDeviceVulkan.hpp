/**
 * @file
 * @date 2022/3/9
 * @author 9chu
 * 此文件为 LuaSTGPlus 项目的一部分，版权与许可声明详见 COPYRIGHT.txt。
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
        ~RenderDeviceVulkan();

    private:
#ifdef LSTG_PLATFORM_MACOS
        // Diligent 使用 Metal 模拟 Vulkan，原生 Metal 支持需要商用许可
        std::unique_ptr<OSX::MetalView> m_stView;
#endif
#ifdef LSTG_PLATFORM_LINUX
#ifdef LSTG_X11_ENABLE
        void* m_pXCBConnection = nullptr;
#endif
#endif
    };
}
