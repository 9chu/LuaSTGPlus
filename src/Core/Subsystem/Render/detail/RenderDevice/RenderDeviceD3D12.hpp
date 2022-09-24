/**
 * @file
 * @date 2022/6/6
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

namespace lstg::Subsystem::Render::detail::RenderDevice
{
    /**
     * D3D12 渲染设备
     */
    class RenderDeviceD3D12 :
        public Render::RenderDevice
    {
    public:
        RenderDeviceD3D12(WindowSystem* window);
    };
}
