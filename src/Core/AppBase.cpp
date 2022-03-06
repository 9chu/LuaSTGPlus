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

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#include <emscripten/html5.h>
#endif

using namespace std;
using namespace lstg;

static std::atomic<AppBase*> kGlobalInstance {};

AppBase& AppBase::GetInstance() noexcept
{
    auto p = kGlobalInstance.load(memory_order_acquire);
    assert(p);
    return *p;
}

AppBase::AppBase()
    : m_stScriptSystem(m_stVirtualFileSystem)
{
    assert(kGlobalInstance.load(memory_order_acquire) == nullptr);
    kGlobalInstance.store(this, memory_order_release);

    m_stFrameTask.SetCallback([this]() noexcept {
        Frame();
    });
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
    m_stMainTaskTimer.Reset();
    m_stMainTaskTimer.Schedule(&m_stFrameTask, start + static_cast<uint64_t>(m_stSleeper.GetFrequency() * m_dFrameInterval));

#ifndef __EMSCRIPTEN__
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

void AppBase::OnNativeEvent(const SDL_Event& event) noexcept
{
    // TODO 冒泡消息传递
    if (event.type == SDL_QUIT)
    {
        m_bShouldStop = true;
    }
}

void AppBase::OnUpdate(double elapsed) noexcept
{
    m_stScriptSystem.Update(elapsed);

    // TODO
}

void AppBase::OnRender(double elapsed) noexcept
{
    // TODO
}

#ifdef __EMSCRIPTEN__
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
    // 更新消息
    SDL_Event event;
    while (::SDL_PollEvent(&event) != 0)
        OnNativeEvent(event);

    // 执行更新逻辑
    Update();

    // 执行渲染逻辑
#ifdef __EMSCRIPTEN__
    m_bRenderEmit = true;
#else
    Render();
#endif

    // 继续执行定时任务
    auto now = Pal::GetCurrentTick();
    m_stMainTaskTimer.Schedule(&m_stFrameTask, now + static_cast<uint64_t>(m_stSleeper.GetFrequency() * m_dFrameInterval));
}

void AppBase::Update() noexcept
{
    // 计算更新时间间隔
    auto now = Pal::GetCurrentTick();
    auto elapsed = static_cast<double>(now - m_ullLastUpdateTick) / m_stSleeper.GetFrequency();
    m_ullLastUpdateTick = now;

    // 更新一帧
    OnUpdate(elapsed);
}

void AppBase::Render() noexcept
{
    // 计算渲染时间间隔
    auto now = Pal::GetCurrentTick();
    auto elapsed = static_cast<double>(now - m_ullLastRenderTick) / m_stSleeper.GetFrequency();
    m_ullLastRenderTick = now;

    // 渲染一帧
    OnRender(elapsed);
}
