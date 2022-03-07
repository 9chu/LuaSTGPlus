/**
 * @file
 * @date 2022/2/17
 * @author 9chu
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#pragma once
#include "ISubsystem.hpp"
#include "WindowSystem.hpp"

namespace lstg::Subsystem
{
    LSTG_DEFINE_EXCEPTION(RendererInitializeFailedException);

    namespace detail
    {
        class BgfxCallback;
    }

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
        ~RenderSystem() override;

    public:
        /**
         * 获取渲染分辨率
         */
        std::tuple<int, int> GetBackBufferResolution() const noexcept { return m_stBackBufferResolution; }

    private:
        std::shared_ptr<WindowSystem> m_pWindowSystem;
        std::shared_ptr<detail::BgfxCallback> m_pBgfxCallback;
#ifdef __ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__
        void* m_pMetalView = nullptr;
        void* m_pMetalViewLayer = nullptr;
#endif

        std::tuple<int, int> m_stBackBufferResolution;
    };
}
