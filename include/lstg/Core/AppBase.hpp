/**
 * @file
 * @author 9chu
 * @date 2022/2/15
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#pragma once
#include "Timer.hpp"
#include "PreciseSleeper.hpp"
#include "Subsystem/WindowSystem.hpp"

union SDL_Event;

namespace lstg
{
    /**
     * 应用程序框架基类
     */
    class AppBase
    {
    public:
        /**
         * 获取全局唯一实例
         */
        static AppBase& GetInstance() noexcept;

    public:
        AppBase();
        AppBase(const AppBase&) = delete;
        AppBase(AppBase&&)noexcept = delete;
        virtual ~AppBase();

    public:
        // <editor-fold desc="子系统">

        /**
         * 获取窗口子系统
         */
        [[nodiscard]] WindowSystem& GetWindowSystem() noexcept { return m_stWindowSystem; }
        [[nodiscard]] const WindowSystem& GetWindowSystem() const noexcept { return m_stWindowSystem; }

        // </editor-fold>
        // <editor-fold desc="消息循环">

        /**
         * 获取更新帧率
         * @return 返回每秒频率
         */
        [[nodiscard]] double GetFrameRate() const noexcept;

        /**
         * 设置更新帧率
         * @param rate 帧率
         */
        void SetFrameRate(double rate) noexcept;

        /**
         * 启动应用程序循环
         */
        void Run() noexcept;

        /**
         * 通知应用程序终止
         */
        void Stop() noexcept;

        // </editor-fold>

    protected:  // 框架事件
        /**
         * 当收到系统消息时触发
         * @param event 系统消息
         */
        virtual void OnNativeEvent(const SDL_Event& event) noexcept;

        /**
         * 当更新逻辑时触发
         * @param elapsed 距离上一次更新的间隔时间（秒）
         */
        virtual void OnUpdate(double elapsed) noexcept;

        /**
         * 当渲染画面时触发
         * @param elapsed 距离上一次渲染的间隔时间（秒）
         */
        virtual void OnRender(double elapsed) noexcept;

    private:
#ifdef __EMSCRIPTEN__
        static void OnWebLoopOnce(void* userdata) noexcept;
        static void OnWebRender(void* userdata) noexcept;
#endif
        double LoopOnce() noexcept;
        void Frame() noexcept;
        void Update() noexcept;
        void Render() noexcept;

    private:
        // 子系统
        WindowSystem m_stWindowSystem;

        // 帧率控制
        Timer m_stMainTaskTimer;
        PreciseSleeper m_stSleeper;
        double m_dFrameInterval = 1. / 60.;
        TimerTask m_stFrameTask;
#ifdef __EMSCRIPTEN__
        long m_lTimeoutId = 0;  // 主逻辑循环定时器
        bool m_bRenderEmit = false;  // HTML下，渲染不跟随逻辑进行，通过标志位控制
#endif

        // 消息循环
        bool m_bShouldStop = false;
        uint64_t m_ullLastUpdateTick = 0;
        uint64_t m_ullLastRenderTick = 0;
    };
} // namespace lstg
