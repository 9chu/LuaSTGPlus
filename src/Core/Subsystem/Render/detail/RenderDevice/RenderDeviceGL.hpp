/**
 * @file
 * @date 2022/3/8
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
#include "OSX/GLView.hpp"
#include "Emscripten/GLView.hpp"
#include "Linux/GLContext.hpp"

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
#ifdef LSTG_PLATFORM_LINUX
        std::unique_ptr<Linux::GLContext> m_stContext;
#endif
    };
}
