/**
 * @file
 * @author 9chu
 * @date 2022/2/16
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
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
#endif

// 所有子系统
#include <lstg/Core/Subsystem/WindowSystem.hpp>
#include <lstg/Core/Subsystem/VirtualFileSystem.hpp>
#include <lstg/Core/Subsystem/ScriptSystem.hpp>
#include <lstg/Core/Subsystem/RenderSystem.hpp>
#include <lstg/Core/Subsystem/DebugGUISystem.hpp>
#include <lstg/Core/Subsystem/ProfileSystem.hpp>

using namespace std;
using namespace lstg;

using SubsystemRegisterFlags = Subsystem::SubsystemRegisterFlags;

LSTG_DEF_LOG_CATEGORY(AppBase);

static std::atomic<AppBase*> kGlobalInstance {};

AppBase& AppBase::GetInstance() noexcept
{
    auto p = kGlobalInstance.load(memory_order_acquire);
    assert(p);
    return *p;
}

AppBase::AppBase()
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
    m_stSubsystemContainer.Register<Subsystem::WindowSystem>("WindowSystem", 0, kSubsystemNoInteractive);
    m_stSubsystemContainer.Register<Subsystem::VirtualFileSystem>("VirtualFileSystem", 0, kSubsystemNoInteractive);
    m_stSubsystemContainer.Register<Subsystem::ScriptSystem>("ScriptSystem", 0,
        SubsystemRegisterFlags::NoRender | SubsystemRegisterFlags::NoEvent);
    m_stSubsystemContainer.Register<Subsystem::RenderSystem>("RenderSystem", 0,
        SubsystemRegisterFlags::NoUpdate | SubsystemRegisterFlags::NoRender);
    m_stSubsystemContainer.Register<Subsystem::DebugGUISystem>("DebugGUISystem", 0);
    m_stSubsystemContainer.Register<Subsystem::ProfileSystem>("ProfileSystem", 0, kSubsystemNoInteractive);
    m_stSubsystemContainer.ConstructAll();
    LSTG_LOG_TRACE_CAT(AppBase, "All subsystem initialized");

    // 持有渲染子系统指针
    m_pRenderSystem = m_stSubsystemContainer.Get<Subsystem::RenderSystem>();
    m_pProfileSystem = m_stSubsystemContainer.Get<Subsystem::ProfileSystem>();
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

Result<void> AppBase::Run() noexcept
{
    m_bShouldStop = false;

    auto start = Pal::GetCurrentTick();
    m_ullLastUpdateTick = start;
    m_ullLastRenderTick = start;
    m_dFrameRateCounterTimer = 0;
    m_uUpdateFramesInSecond = 0;
    m_uRenderFramesInSecond = 0;
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
    return {};
#else
    // 初始化逻辑定时器
    m_lTimeoutId = ::emscripten_set_timeout(OnWebLoopOnce, 0., this);
    m_bRenderEmit = false;

    // 初始化渲染循环
    // 当设置 fps = 0 时，使用 requestAnimationFrame 保证渲染不发生撕裂
    // 我们用这个方法来跑渲染循环
    ::emscripten_set_main_loop_arg(OnWebRender, this, 0, false);

    // EMSCRIPTEN 在启用异常后，如果 main_loop 模拟无限循环，会抛出异常，导致 unwind
    // 因此我们不使用这个功能，在这里通过 interrupted 告诉外界
    return make_error_code(errc::interrupted);
#endif
}

void AppBase::Stop() noexcept
{
    m_bShouldStop = true;
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
        LSTG_PER_FRAME_PROFILE("EventDispatchTime");
        SDL_Event event;
        while (::SDL_PollEvent(&event) != 0)
        {
            Subsystem::SubsystemEvent transformed(&event);
            OnEvent(transformed);
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
    LSTG_PER_FRAME_PROFILE("UpdateTime");

    // 计算更新时间间隔
    auto now = Pal::GetCurrentTick();
    auto elapsed = static_cast<double>(now - m_ullLastUpdateTick) / m_stSleeper.GetFrequency();
    m_ullLastUpdateTick = now;

    // 更新一帧
    {
        m_stSubsystemContainer.Update(elapsed);
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
    LSTG_PER_FRAME_PROFILE("RenderTime");

    // 计算渲染时间间隔
    auto now = Pal::GetCurrentTick();
    auto elapsed = static_cast<double>(now - m_ullLastRenderTick) / m_stSleeper.GetFrequency();
    m_ullLastRenderTick = now;

    // 渲染一帧
    {
        m_pRenderSystem->BeginFrame();
        m_stSubsystemContainer.BeforeRender(elapsed);
        OnRender(elapsed);
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
