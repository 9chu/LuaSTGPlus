/**
 * @file
 * @author 9chu
 * @date 2022/2/16
 * 此文件为 LuaSTGPlus 项目的一部分，版权与许可声明详见 COPYRIGHT.txt。
 */
#include <lstg/Core/AppBase.hpp>

#include <cassert>
#include <atomic>
#include <SDL.h>
#include <lstg/Core/Pal.hpp>
#include <lstg/Core/Logging.hpp>

#ifdef LSTG_PLATFORM_EMSCRIPTEN
#include <emscripten.h>
#include <emscripten/html5.h>
#include "detail/EmFileDownloader.hpp"

/**
 * 这些方法在新版 emscripten 中必须定义 USE_PTHREAD 才能使用
 * 由于 SDL 引用了这些方法，这里写死单线程情况下的实现，以避免找不到符号
 */
extern "C"
{
    int emscripten_sync_run_in_main_runtime_thread_(unsigned int sig, void *func_ptr, ...)
    {
        assert(false);
        return 0;
    }
}
#endif

// 所有子系统
#include <lstg/Core/Subsystem/AssetSystem.hpp>
#include <lstg/Core/Subsystem/DebugGUISystem.hpp>
#include <lstg/Core/Subsystem/EventBusSystem.hpp>
#include <lstg/Core/Subsystem/GameControllerSystem.hpp>
#include <lstg/Core/Subsystem/ProfileSystem.hpp>
#include <lstg/Core/Subsystem/RenderSystem.hpp>
#include <lstg/Core/Subsystem/ScriptSystem.hpp>
#include <lstg/Core/Subsystem/VirtualFileSystem.hpp>
#include <lstg/Core/Subsystem/WindowSystem.hpp>
#include <lstg/Core/Subsystem/AudioSystem.hpp>

using namespace std;
using namespace lstg;

using SubsystemRegisterFlags = Subsystem::SubsystemRegisterFlags;

LSTG_DEF_LOG_CATEGORY(AppBase);

static std::atomic<AppBase*> kGlobalInstance {};
static Text::CmdlineParser kCmdlineParserInstance {};

AppBase& AppBase::GetInstance() noexcept
{
    auto p = kGlobalInstance.load(memory_order_acquire);
    assert(p);
    return *p;
}

void AppBase::ParseCmdline(int argc, const char* argv[])
{
#ifdef LSTG_PARSE_CMDLINE
    // 解析命令行
    kCmdlineParserInstance.Parse(argc, argv);
#else
    assert(argc >= 1);
    kCmdlineParserInstance.Parse(1, argv);  // 只保留应用程序路径
#endif
}

const Text::CmdlineParser& AppBase::GetCmdline() noexcept
{
    return kCmdlineParserInstance;
}

AppBase::AppBase(int argc, const char* argv[])
{
    assert(kGlobalInstance.load(memory_order_acquire) == nullptr);
    kGlobalInstance.store(this, memory_order_release);

    m_stFrameTask.SetCallback([this]() noexcept {
        Frame();
    });

    // 注册子系统
    LSTG_LOG_TRACE_CAT(AppBase, "Begin to initialize subsystem");
    const auto kSubsystemNoInteractive =
        SubsystemRegisterFlags::NoUpdate | SubsystemRegisterFlags::NoRender | SubsystemRegisterFlags::NoEvent;
    const auto kSubsystemUpdateOnly = SubsystemRegisterFlags::NoRender | SubsystemRegisterFlags::NoEvent;
    const auto kSubsystemEventOnly = SubsystemRegisterFlags::NoRender | SubsystemRegisterFlags::NoUpdate;
    m_stSubsystemContainer.Register<Subsystem::EventBusSystem>("EventBusSystem", 0, kSubsystemNoInteractive);
    m_stSubsystemContainer.Register<Subsystem::WindowSystem>("WindowSystem", 0, kSubsystemNoInteractive);
    m_stSubsystemContainer.Register<Subsystem::VirtualFileSystem>("VirtualFileSystem", 0, kSubsystemNoInteractive);
    m_stSubsystemContainer.Register<Subsystem::ScriptSystem>("ScriptSystem", 0, kSubsystemUpdateOnly);
    m_stSubsystemContainer.Register<Subsystem::RenderSystem>("RenderSystem", 0,
        SubsystemRegisterFlags::NoUpdate | SubsystemRegisterFlags::NoRender);
    m_stSubsystemContainer.Register<Subsystem::DebugGUISystem>("DebugGUISystem", 0);
    m_stSubsystemContainer.Register<Subsystem::ProfileSystem>("ProfileSystem", 0, kSubsystemNoInteractive);
    m_stSubsystemContainer.Register<Subsystem::AssetSystem>("AssetSystem", 0, kSubsystemUpdateOnly);
    m_stSubsystemContainer.Register<Subsystem::GameControllerSystem>("GameControllerSystem", 0, kSubsystemEventOnly);
    m_stSubsystemContainer.Register<Subsystem::AudioSystem>("AudioSystem", 0, kSubsystemUpdateOnly);
    m_stSubsystemContainer.ConstructAll();
    LSTG_LOG_TRACE_CAT(AppBase, "All subsystem initialized");

    // 持有渲染子系统指针
    m_pEventBusSystem = m_stSubsystemContainer.Get<Subsystem::EventBusSystem>();
    m_pRenderSystem = m_stSubsystemContainer.Get<Subsystem::RenderSystem>();
    m_pProfileSystem = m_stSubsystemContainer.Get<Subsystem::ProfileSystem>();

#ifdef LSTG_PLATFORM_EMSCRIPTEN
    m_pFileDownloader = make_shared<detail::EmFileDownloader>();
#endif

    // 允许从命令行设置跳帧
    auto cmdRenderFrameSkip = GetCmdline().GetOption<int>("render-frame-skip", 0);
    if (cmdRenderFrameSkip != 0)
    {
        LSTG_LOG_INFO_CAT(AppBase, "Set render frame skip to {}", cmdRenderFrameSkip);
        m_uRenderFrameSkip = cmdRenderFrameSkip;
    }
}

AppBase::~AppBase()
{
    assert(kGlobalInstance.load(memory_order_acquire) == this);
    kGlobalInstance.store(nullptr, memory_order_release);
}

double AppBase::GetFrameRate() const noexcept
{
    return 1. / m_dFrameInterval;
}

void AppBase::SetFrameRate(double rate) noexcept
{
    m_dFrameInterval = 1. / std::max(1., rate);
}

void AppBase::Run()
{
    m_bShouldStop = false;

#ifdef LSTG_PLATFORM_EMSCRIPTEN
    // 执行预初始化
    PreInit();
#else
    // 调用 OnStartup 执行初始化
    OnStartup();
#endif

    auto start = Pal::GetCurrentTick();
    m_ullLastUpdateTick = start;
    m_ullLastRenderTick = start;
    m_dFrameRateCounterTimer = 0;
    m_uUpdateFramesInSecond = 0;
    m_uRenderFramesInSecond = 0;
    m_uRenderFrameSkipCounter = 0;
    m_stMainTaskTimer.Reset();
    m_stMainTaskTimer.Schedule(&m_stFrameTask, start + static_cast<uint64_t>(m_stSleeper.GetFrequency() * GetBestFrameInterval()));

#ifndef LSTG_PLATFORM_EMSCRIPTEN
    // 执行应用循环
    while (!m_bShouldStop)
    {
        auto timeToSleep = LoopOnce();

        // 睡眠
        m_stSleeper.Sleep(timeToSleep);
    }
#else
    // 初始化逻辑定时器
    m_lTimeoutId = ::emscripten_set_timeout(OnWebLoopOnce, 0., this);
    m_bRenderEmit = false;

    // 初始化渲染循环
    // 当设置 fps = 0 时，使用 requestAnimationFrame 保证渲染不发生撕裂
    // 我们用这个方法来跑渲染循环
    ::emscripten_set_main_loop_arg(OnWebRender, this, 0, true);
#endif
}

void AppBase::Stop() noexcept
{
    m_bShouldStop = true;
}

void AppBase::OnStartup()
{
}

void AppBase::OnEvent(Subsystem::SubsystemEvent& event) noexcept
{
    // 冒泡消息传递
    m_stSubsystemContainer.BubbleEvent(event);

    // 执行默认行为
    if (!event.IsDefaultPrevented())
    {
        const auto& underlay = event.GetEvent();
        if (underlay.index() == 0)
        {
            auto sdlEvent = std::get<0>(underlay);
            assert(sdlEvent);

            if (sdlEvent->type == SDL_QUIT)
            {
                LSTG_LOG_TRACE_CAT(AppBase, "SDL quit event received");
                m_bShouldStop = true;
            }
        }
    }
}

void AppBase::OnUpdate(double elapsed) noexcept
{
    // 覆写该方法实现应用程序行为
}

void AppBase::OnRender(double elapsed) noexcept
{
    // 覆写该方法实现应用程序行为
}

#ifdef LSTG_PLATFORM_EMSCRIPTEN
void AppBase::OnWebLoopOnce(void* userdata) noexcept
{
    auto self = static_cast<AppBase*>(userdata);

    auto timeToSleep = self->LoopOnce();

    // 如果要求退出，则终止循环，否则继续下一次逻辑更新
    if (self->m_bShouldStop)
        ::emscripten_cancel_main_loop();
    else
        self->m_lTimeoutId = ::emscripten_set_timeout(OnWebLoopOnce, timeToSleep * 1000., self);
}

void AppBase::OnWebRender(void* userdata) noexcept
{
    auto self = static_cast<AppBase*>(userdata);

    // 浏览器下 setTimeout 并不很准确
    // 这里主动调用下 LoopOnce
    self->LoopOnce();

    if (self->m_bRenderEmit)
    {
        self->m_bRenderEmit = false;
        self->Render();
    }
}

EM_JS(int, GetPreloadAssetCount_, (), {
    return getPreloadAssetCount();
});

EM_JS(const char*, GetPreloadAssetUrl_, (int i), {
    let s = getPreloadAssetUrl(i);
    if (!s)
        return null;
    let length = lengthBytesUTF8(s) + 1;
    let str = _malloc(length);
    stringToUTF8(s, str, length);
    return str;
});

EM_JS(const char*, GetPreloadAssetSaveAs_, (int i), {
    let s = getPreloadAssetSaveAs(i);
    if (!s)
        return null;
    let length = lengthBytesUTF8(s) + 1;
    let str = _malloc(length);
    stringToUTF8(s, str, length);
    return str;
});

void AppBase::PreInit()
{
    struct JsStringDeleter
    {
        void operator()(const char* p) noexcept
        {
            ::free(const_cast<char*>(p));
        }
    };

    if (m_iAppState != STATE_NOT_READY)
        return;
    m_iAppState = STATE_PRE_INIT;

    // 获取需要预载的资源列表
    auto preloadAssets = GetPreloadAssetCount_();
    if (preloadAssets > 0)
    {
        // 显示进度窗口
        auto progressWindow = GetSubsystem<Subsystem::DebugGUISystem>()->GetProgressWindow();
        progressWindow->Show();

        for (auto i = 0; i < preloadAssets; ++i)
        {
            shared_ptr<const char> url {GetPreloadAssetUrl_(i), JsStringDeleter{}};
            shared_ptr<const char> saveAs {GetPreloadAssetSaveAs_(i), JsStringDeleter{}};

            // 发起任务
            m_pFileDownloader->AddTask(url.get(), saveAs.get(), [this, url, saveAs, progressWindow](Result<void> ret) noexcept {
                // 检查是否有错误
                if (!ret)
                {
                    LSTG_LOG_ERROR_CAT(AppBase, "Download asset from url '{}' fail: {}", url.get(), ret.GetError());
                    m_pFileDownloader->CancelPendingTasks();  // 取消所有在途任务
                    m_iAppState = STATE_ERROR;
                    try
                    {
                        progressWindow->SetPercent(0);
                        progressWindow->SetHintText(fmt::format("Fail to download {}: {}", saveAs.get(), ret.GetError()));
                    }
                    catch (...)
                    {
                    }
                    return;
                }

                // 检查是否所有下载任务完成
                if (m_pFileDownloader->GetTaskCount() == 0)
                {
                    LSTG_LOG_INFO_CAT(AppBase, "All preload asset downloaded");

                    optional<string> errorString;
                    try
                    {
                        progressWindow->SetPercent(1);
                        progressWindow->SetHintText("Initializing game app");

                        OnStartup();
                    }
                    catch (const std::exception& ex)
                    {
                        try
                        {
                            errorString.emplace(ex.what());
                        }
                        catch (...)
                        {
                            errorString.emplace(string{});
                        }
                    }
                    catch (...)
                    {
                        try
                        {
                            errorString.emplace("Out of memory");
                        }
                        catch (...)
                        {
                            errorString.emplace(string{});
                        }
                    }

                    if (!errorString)
                    {
                        m_iAppState = STATE_INITED;
                        progressWindow->Hide();
                    }
                    else
                    {
                        m_iAppState = STATE_ERROR;
                        try
                        {
                            progressWindow->SetPercent(0);
                            progressWindow->SetHintText(fmt::format("Fail to init game: {}", *errorString));
                        }
                        catch (...)
                        {
                        }
                    }
                }
            }, [progressWindow](std::string_view url, std::string_view saveAs, float percent) noexcept {
                try
                {
                    progressWindow->SetPercent(percent);
                    progressWindow->SetHintText(fmt::format("Downloading {}", saveAs));
                }
                catch (...)
                {
                }
            });
        }
    }
    else
    {
        m_iAppState = STATE_INITED;
    }
}
#endif

double AppBase::LoopOnce() noexcept
{
    // 更新定时器
    auto suggestedSleepTime = m_stMainTaskTimer.Update(std::numeric_limits<uint64_t>::max());
    assert(suggestedSleepTime != std::numeric_limits<uint64_t>::max());

    // 计算Sleep时间
    return static_cast<double>(suggestedSleepTime) / m_stSleeper.GetFrequency();
}

void AppBase::Frame() noexcept
{
    m_pProfileSystem->NewFrame();
    auto now = Pal::GetCurrentTick();

    // 更新消息
    {
#ifdef LSTG_DEVELOPMENT
        LSTG_PER_FRAME_PROFILE(EventDispatchTime);
#endif

        // SDL 消息
        SDL_Event event;
        while (::SDL_PollEvent(&event) != 0)
        {
            Subsystem::SubsystemEvent transformed(&event);

#ifdef LSTG_PLATFORM_EMSCRIPTEN
            if (m_iAppState == STATE_INITED)
#endif
            {
                OnEvent(transformed);
            }
#ifdef LSTG_PLATFORM_EMSCRIPTEN
            else
            {
                AppBase::OnEvent(transformed);
            }
#endif
        }

        // 总线消息
        while (true)
        {
            auto subsystemEvent = m_pEventBusSystem->PollEvent();
            if (!subsystemEvent)
                break;

#ifdef LSTG_PLATFORM_EMSCRIPTEN
            if (m_iAppState == STATE_INITED)
#endif
            {
                OnEvent(*subsystemEvent);
            }
#ifdef LSTG_PLATFORM_EMSCRIPTEN
            else
            {
                AppBase::OnEvent(*subsystemEvent);
            }
#endif
        }
    }

    // 执行更新逻辑
    Update();

    // 执行渲染逻辑
#ifdef LSTG_PLATFORM_EMSCRIPTEN
    m_bRenderEmit = true;
#else
    Render();
#endif

    // 继续执行定时任务
    m_stMainTaskTimer.Schedule(&m_stFrameTask, now + static_cast<uint64_t>(m_stSleeper.GetFrequency() * GetBestFrameInterval()));
}

void AppBase::Update() noexcept
{
#ifdef LSTG_DEVELOPMENT
    LSTG_PER_FRAME_PROFILE(UpdateTime);
#endif

    // 计算更新时间间隔
    auto now = Pal::GetCurrentTick();
    auto elapsed = static_cast<double>(now - m_ullLastUpdateTick) / m_stSleeper.GetFrequency();
    m_ullLastUpdateTick = now;

    // 更新一帧
    m_stSubsystemContainer.Update(elapsed);
#ifdef LSTG_PLATFORM_EMSCRIPTEN
    m_pFileDownloader->Update();
    if (m_iAppState == STATE_INITED)
#endif
    {
        OnUpdate(elapsed);
    }
    ++m_uUpdateFramesInSecond;

    // 更新计时器
    m_dFrameRateCounterTimer += elapsed;
    if (m_dFrameRateCounterTimer >= 1.0)
    {
        auto logicRate = m_uUpdateFramesInSecond / m_dFrameRateCounterTimer;
        auto renderRate = m_uRenderFramesInSecond / m_dFrameRateCounterTimer;
        m_pProfileSystem->SetPerformanceCounter(Subsystem::PerformanceCounterTypes::RealTime, "LogicFps", logicRate);
        m_pProfileSystem->SetPerformanceCounter(Subsystem::PerformanceCounterTypes::RealTime, "RenderFps", renderRate);
        m_dFrameRateCounterTimer = 0;
        m_uUpdateFramesInSecond = 0;
        m_uRenderFramesInSecond = 0;
    }
}

void AppBase::Render() noexcept
{
#ifdef LSTG_DEVELOPMENT
    LSTG_PER_FRAME_PROFILE(RenderTime);
#endif

    // 计算跳帧
    if (m_uRenderFrameSkip != 0)
    {
        if (m_uRenderFrameSkipCounter >= m_uRenderFrameSkip)
        {
            m_uRenderFrameSkipCounter = 0;
        }
        else
        {
            ++m_uRenderFrameSkipCounter;
            return;
        }
    }

    // 计算渲染时间间隔
    auto now = Pal::GetCurrentTick();
    auto elapsed = static_cast<double>(now - m_ullLastRenderTick) / m_stSleeper.GetFrequency();
    m_ullLastRenderTick = now;

    // 渲染一帧
    {
        m_pRenderSystem->BeginFrame();
        m_stSubsystemContainer.BeforeRender(elapsed);
#ifdef LSTG_PLATFORM_EMSCRIPTEN
        if (m_iAppState == STATE_INITED)
#endif
        {
            OnRender(elapsed);
        }
        m_stSubsystemContainer.AfterRender(elapsed);
        m_pRenderSystem->EndFrame();
    }
    ++m_uRenderFramesInSecond;
}

double AppBase::GetBestFrameInterval() noexcept
{
    // FIXME: 我们假设开启垂直同步时的刷新率是 60 hz
    if (m_pRenderSystem->GetRenderDevice()->IsVerticalSyncEnabled())
    {
        // 如果帧率限制和垂直同步刷新率近似，会导致锁帧出现问题，我们多给一帧冗余
        if (::fabs(m_dFrameInterval - 1 / 60.) <= std::numeric_limits<double>::epsilon())
            return 1 / 60.5;
    }

    // 其他情况不管
    return m_dFrameInterval;
}
