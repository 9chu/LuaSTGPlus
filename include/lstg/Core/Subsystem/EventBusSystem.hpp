/**
 * @file
 * @author 9chu
 * @date 2022/6/7
 * 此文件为 LuaSTGPlus 项目的一部分，版权与许可声明详见 COPYRIGHT.txt。
 */
#pragma once
#include <deque>
#include <optional>
#include "ISubsystem.hpp"
#include "../Result.hpp"

namespace lstg::Subsystem
{
    /**
     * 事件总线
     */
    class EventBusSystem :
        public ISubsystem
    {
    public:
        EventBusSystem(SubsystemContainer& container);
        EventBusSystem(const EventBusSystem&) = delete;
        EventBusSystem(EventBusSystem&&)noexcept = delete;

    public:
        /**
         * 触发事件
         * @param event 事件
         */
        Result<void> EmitEvent(SubsystemEvent event) noexcept;

        /**
         * 读取事件
         * @return 事件
         */
        std::optional<SubsystemEvent> PollEvent() noexcept;

    private:
        std::deque<SubsystemEvent> m_stEventQueue;
    };

    using EventBusSystemPtr = std::shared_ptr<EventBusSystem>;
}
