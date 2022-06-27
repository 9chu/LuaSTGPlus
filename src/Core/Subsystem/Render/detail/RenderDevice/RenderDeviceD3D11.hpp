/**
 * @file
 * @date 2022/6/6
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

namespace lstg::Subsystem::Render::detail::RenderDevice
{
    /**
     * D3D11 渲染设备
     */
    class RenderDeviceD3D11 :
        public Render::RenderDevice
    {
    public:
        RenderDeviceD3D11(WindowSystem* window);
    };
}
