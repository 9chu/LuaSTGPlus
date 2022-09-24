/**
 * @file
 * @author 9chu
 * @date 2022/2/17
 * 此文件为 LuaSTGPlus 项目的一部分，版权与许可声明详见 COPYRIGHT.txt。
 */
#pragma once
#include <cstdint>
#include <functional>
#include "IntrusiveHeap.hpp"

namespace lstg
{
    /**
     * 定时器任务
     */
    class TimerTask :
        public IntrusiveHeapNode
    {
        friend class Timer;

    public:
        using Callback = std::function<void()>;

    public:
        TimerTask() = default;
        TimerTask(const TimerTask&) = delete;
        TimerTask(TimerTask&& org) noexcept;

        TimerTask& operator=(const TimerTask& rhs) = delete;
        TimerTask& operator=(TimerTask&& rhs) noexcept = delete;

    public:
        /**
         * 是否已经在调度
         */
        bool IsScheduled() const noexcept { return m_bInSchedule; }

        /**
         * 获取唤醒时间
         */
        uint64_t GetScheduleTime() const noexcept { return m_ullScheduleTime; }

        /**
         * 获取回调方法
         */
        const Callback& GetCallback() const noexcept { return m_stCallback; }

        /**
         * 设置回调方法
         * @param cb 回调（需要满足noexcept）
         */
        void SetCallback(Callback cb) noexcept { m_stCallback = std::move(cb); }

    private:
        bool m_bInSchedule = false;
        uint64_t m_ullScheduleTime = 0;
        Callback m_stCallback;
    };

    /**
     * 定时器
     */
    class Timer
    {
    public:
        /**
         * 设定定时器任务
         * @note 注意定时器不会维护任务的内存。
         * @param task 任务
         * @param scheduleTime 下次执行时间
         */
        void Schedule(TimerTask* task, uint64_t scheduleTime) noexcept;

        /**
         * 取消定时任务
         * @note 注意定时器不会维护任务的内存。
         * @param task 任务
         */
        void Unschedule(TimerTask* task) noexcept;

        /**
         * 更新状态并执行任务
         * @param idleWait 默认空闲等待时间
         * @return 距离下次调度的时间差
         */
        uint64_t Update(uint64_t idleWait) noexcept;

        /**
         * 清理所有任务
         */
        void Reset() noexcept;

    private:
        struct TimerTaskComparer
        {
            bool operator()(const TimerTask* a, const TimerTask* b) const noexcept;
        };

        IntrusiveHeap<TimerTask, TimerTaskComparer> m_stHeap;
    };
}
