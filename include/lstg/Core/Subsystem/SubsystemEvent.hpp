/**
 * @file
 * @date 2022/3/6
 * @author 9chu
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#pragma once
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
        template <typename... TArgs>
        SubsystemEvent(TArgs&&... args)
            : m_stEvent(std::forward<TArgs>(args)...) {}

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
    };
}
