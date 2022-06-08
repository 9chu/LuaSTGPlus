/**
 * @file
 * @author 9chu
 * @date 2022/6/7
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
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
