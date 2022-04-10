/**
 * @file
 * @date 2022/3/8
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
#include "OSX/GLView.hpp"
#include "Emscripten/GLView.hpp"

namespace lstg::Subsystem::Render::detail::RenderDevice
{
    /**
     * OpenGL 渲染设备
     */
    class RenderDeviceGL :
        public Render::RenderDevice
    {
    public:
        RenderDeviceGL(WindowSystem* window);

    protected:  // RenderDevice
        bool IsVerticalSyncEnabled() const noexcept override;
        void SetVerticalSyncEnabled(bool enable) noexcept override;
        void Present() noexcept override;

    private:
#ifdef LSTG_PLATFORM_MACOS
        std::unique_ptr<OSX::GLView> m_stView;
#endif
#ifdef LSTG_PLATFORM_EMSCRIPTEN
        std::unique_ptr<Emscripten::GLView> m_stView;
#endif
    };
}
