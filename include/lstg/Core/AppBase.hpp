/**
 * @file
 * @author 9chu
 * @date 2022/2/15
 * 此文件为 LuaSTGPlus 项目的一部分，版权与许可声明详见 COPYRIGHT.txt。
 */
#pragma once
#include "Timer.hpp"
#include "PreciseSleeper.hpp"
#include "Result.hpp"
#include "JniUtils.hpp"
#include "Text/CmdlineParser.hpp"
#include "Subsystem/SubsystemContainer.hpp"

struct AAssetManager;

namespace lstg
{
    namespace detail
    {
        class EmFileDownloader;
    }

    namespace Subsystem
    {
        class EventBusSystem;
        class RenderSystem;
        class ProfileSystem;
    }

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

        /**
         * 解析命令行
         */
        static void ParseCmdline(int argc, const char* argv[]);

        /**
         * 获取命令行参数
         */
        static const Text::CmdlineParser& GetCmdline() noexcept;

    public:
        AppBase(int argc, const char* argv[]);
        AppBase(const AppBase&) = delete;
        AppBase(AppBase&&)noexcept = delete;
        virtual ~AppBase();

    public:
        // <editor-fold desc="子系统">

        template <typename T>
        std::shared_ptr<T> GetSubsystem()
        {
            return m_stSubsystemContainer.Get<T>();
        }

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
        void Run();

        /**
         * 通知应用程序终止
         */
        void Stop() noexcept;

        // </editor-fold>
        // <editor-fold desc="平台相关">

#ifdef LSTG_PLATFORM_ANDROID
        /**
         * 获取 Android AssetManager 对象指针
         */
        [[nodiscard]] AAssetManager* GetAndroidAssetManager() const noexcept { return m_pAndroidAssetManager; }
#endif

        // </editor-fold>

    protected:  // 框架事件
        /**
         * 当调用 Run 时触发
         */
        virtual void OnStartup();

        /**
         * 当收到消息时触发
         * @param event 消息
         */
        virtual void OnEvent(Subsystem::SubsystemEvent& event) noexcept;

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
#ifdef LSTG_PLATFORM_EMSCRIPTEN
        static void OnWebLoopOnce(void* userdata) noexcept;
        static void OnWebRender(void* userdata) noexcept;
        void PreInit();
#endif
        double LoopOnce() noexcept;
        void Frame() noexcept;
        void Update() noexcept;
        void Render() noexcept;
        double GetBestFrameInterval() noexcept;

    private:
        // 子系统
        Subsystem::SubsystemContainer m_stSubsystemContainer;
        std::shared_ptr<Subsystem::EventBusSystem> m_pEventBusSystem;
        std::shared_ptr<Subsystem::RenderSystem> m_pRenderSystem;
        std::shared_ptr<Subsystem::ProfileSystem> m_pProfileSystem;

        // 帧率控制
        Timer m_stMainTaskTimer;
        PreciseSleeper m_stSleeper;
        double m_dFrameInterval = 1. / 60.;
        uint32_t m_uRenderFrameSkip = 0;
        TimerTask m_stFrameTask;
#ifdef LSTG_PLATFORM_EMSCRIPTEN
        long m_lTimeoutId = 0;  // 主逻辑循环定时器
        bool m_bRenderEmit = false;  // HTML下，渲染不跟随逻辑进行，通过标志位控制
#endif

        // 消息循环
        bool m_bShouldStop = false;
        uint64_t m_ullLastUpdateTick = 0;
        uint64_t m_ullLastRenderTick = 0;
        double m_dFrameRateCounterTimer = 0;
        unsigned m_uUpdateFramesInSecond = 0;
        unsigned m_uRenderFramesInSecond = 0;
        uint32_t m_uRenderFrameSkipCounter = 0;

#ifdef LSTG_PLATFORM_EMSCRIPTEN
        // Emscripten 环境下，我们需要在启动程序前完成资源包下载
        // 因此在 AppBase 下控制相关流程
        enum {
            STATE_NOT_READY,
            STATE_PRE_INIT,
            STATE_INITED,
            STATE_ERROR,
        } m_iAppState = STATE_NOT_READY;
        std::shared_ptr<detail::EmFileDownloader> m_pFileDownloader;
#endif
#ifdef LSTG_PLATFORM_ANDROID
        JniUtils::JObjectReference m_pAndroidAssetManagerReference;
        AAssetManager* m_pAndroidAssetManager = nullptr;
#endif
    };
} // namespace lstg
