/**
 * @file
 * @author 9chu
 * @date 2022/2/17
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#include <lstg/Core/Timer.hpp>

#include <lstg/Core/Pal.hpp>

using namespace std;
using namespace lstg;

// <editor-fold desc="TimerTask">

TimerTask::TimerTask(TimerTask&& org) noexcept
    : IntrusiveHeapNode(std::move(org)), m_bInSchedule(org.m_bInSchedule), m_ullScheduleTime(org.m_ullScheduleTime),
    m_stCallback(std::move(org.m_stCallback))
{
    org.m_bInSchedule = false;
    org.m_ullScheduleTime = 0;
}

// </editor-fold>
// <editor-fold desc="Timer">

bool Timer::TimerTaskComparer::operator()(const TimerTask* a, const TimerTask* b) const noexcept
{
    return a->GetScheduleTime() < b->GetScheduleTime();
}

void Timer::Schedule(TimerTask* task, uint64_t scheduleTime) noexcept
{
    assert(task && !task->IsScheduled());

    if (task && !task->IsScheduled())
    {
        task->m_bInSchedule = true;
        task->m_ullScheduleTime = scheduleTime;
        m_stHeap.Insert(task);
    }
}

void Timer::Unschedule(TimerTask* task) noexcept
{
    // FIXME: 检查是否在当前 Timer 中
    assert(task && task->IsScheduled());

    if (task && task->IsScheduled())
    {
        task->m_bInSchedule = false;
        task->m_ullScheduleTime = 0;
        m_stHeap.Remove(task);
    }
}

uint64_t Timer::Update(uint64_t idleWait) noexcept
{
    auto now = Pal::GetCurrentTick();
    TimerTask* top = m_stHeap.GetTop();
    while (top)
    {
        if (top->GetScheduleTime() <= now)
        {
            top->m_bInSchedule = false;
            top->m_ullScheduleTime = 0;
            m_stHeap.Remove(top);

            const auto& cb = top->GetCallback();
            if (cb)
            {
                cb();

                // 执行后刷新当前时间
                now = Pal::GetCurrentTick();
            }

            auto nextTop = m_stHeap.GetTop();
            if (nextTop == top)  // 防止陷入死循环
            {
                idleWait = 0;
                top = nullptr;
                break;
            }
            top = nextTop;
        }
        else
        {
            break;
        }
    }

    if (top)
    {
        assert(top->GetScheduleTime() > now);
        return top->GetScheduleTime() - now;
    }
    return idleWait;
}

void Timer::Reset() noexcept
{
    TimerTask* top = m_stHeap.GetTop();
    while (top)
    {
        top->m_bInSchedule = false;
        top->m_ullScheduleTime = 0;
        m_stHeap.Remove(top);
        top = m_stHeap.GetTop();
    }
}

// </editor-fold>
