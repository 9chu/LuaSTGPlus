/**
 * @file
 * @date 2022/2/17
 * @author 9chu
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#pragma once
#include "ISubsystem.hpp"
#include "WindowSystem.hpp"
#include "Render/RenderDevice.hpp"

namespace lstg::Subsystem
{
    /**
     * 渲染子系统
     */
    class RenderSystem :
        public ISubsystem
    {
    public:
        RenderSystem(SubsystemContainer& container);
        RenderSystem(const RenderSystem&) = delete;
        RenderSystem(RenderSystem&&) = delete;
        ~RenderSystem() = default;

    public:
        /**
         * 获取渲染设备
         */
        [[nodiscard]] Render::RenderDevice* GetRenderDevice() const noexcept { return m_pRenderDevice.get(); }

    protected:  // ISubsystem
        void OnEvent(SubsystemEvent& event) noexcept;

    private:
        std::shared_ptr<WindowSystem> m_pWindowSystem;
        Render::RenderDevicePtr m_pRenderDevice;
    };
}
