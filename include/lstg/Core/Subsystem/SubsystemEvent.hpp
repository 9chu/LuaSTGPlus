/**
 * @file
 * @date 2022/3/6
 * @author 9chu
 * 此文件为 LuaSTGPlus 项目的一部分，版权与许可声明详见 COPYRIGHT.txt。
 */
#pragma once
#include <cassert>
#include <any>
#include <variant>

union SDL_Event;

namespace lstg::Subsystem
{
    /**
     * 子系统事件
     */
    class SubsystemEvent
    {
    public:
        using Event = std::variant<const SDL_Event*>;

    public:
        /**
         * 使用事件指针构造轻量事件
         * 此时 SubsystemEvent 不会持有事件对象。
         * @tparam P 事件类型
         * @param eventRef 事件引用
         */
        template <typename P>
        SubsystemEvent(P eventRef, std::enable_if_t<std::is_pointer_v<P>, int> = 0)
            : m_stEvent(eventRef) {}

        /**
         * 使用事件对象构造事件
         * 此时 SubsystemEvent 会持有事件对象的拷贝。
         * @tparam P 事件类型
         * @param event 事件对象
         */
        template <typename P>
        SubsystemEvent(P event, std::enable_if_t<!std::is_pointer_v<P>, void*> = nullptr)
            : m_stEventStorage(std::move(event))
        {
            auto ptr = std::any_cast<P>(&m_stEventStorage);
            assert(ptr);
            m_stEvent = ptr;
        }

        SubsystemEvent(const SubsystemEvent& org);
        SubsystemEvent(SubsystemEvent&& org) noexcept;

        SubsystemEvent& operator=(const SubsystemEvent& rhs);
        SubsystemEvent& operator=(SubsystemEvent&& rhs) noexcept;

    public:
        /**
         * 获取事件
         */
        const Event& GetEvent() const noexcept;

        /**
         * 是否具备默认行为
         */
        bool IsDefaultPrevented() const noexcept;

        /**
         * 阻止默认行为
         */
        void PreventDefault() noexcept;

        /**
         * 是否冒泡
         */
        bool IsBubbles() const noexcept;

        /**
         * 停止冒泡
         */
        void StopPropagation() noexcept;

    private:
        bool m_bIsDefaultPrevented = false;
        bool m_bIsBubbles = true;
        Event m_stEvent;
        std::any m_stEventStorage;
    };
}
