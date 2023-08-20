/**
 * @file
 * @date 2022/3/6
 * @author 9chu
 * 此文件为 LuaSTGPlus 项目的一部分，版权与许可声明详见 COPYRIGHT.txt。
 */
#include <lstg/Core/Subsystem/SubsystemEvent.hpp>

// Event 定义
#include <SDL_events.h>
#include <lstg/Core/Subsystem/Render/RenderEvent.hpp>

using namespace std;
using namespace lstg;
using namespace lstg::Subsystem;

SubsystemEvent::SubsystemEvent(const SubsystemEvent& org)
    : m_stEventStorage(org.m_stEventStorage)
{
    if (m_stEventStorage.has_value())
    {
        std::visit([&](auto value) {
            static_assert(std::is_pointer_v<decltype(value)>);
            auto ptr = std::any_cast<std::remove_pointer_t<decltype(value)>>(&m_stEventStorage);
            assert(ptr);
            m_stEvent = ptr;
        }, org.m_stEvent);
    }
    else
    {
        m_stEvent = org.m_stEvent;
    }
}

SubsystemEvent::SubsystemEvent(SubsystemEvent&& org) noexcept
    : m_stEvent(std::move(org.m_stEvent)), m_stEventStorage(std::move(org.m_stEventStorage))
{
    if (m_stEventStorage.has_value())
    {
        std::visit([&](auto value) {
            static_assert(std::is_pointer_v<decltype(value)>);
            auto ptr = std::any_cast<std::remove_pointer_t<decltype(value)>>(&m_stEventStorage);
            assert(ptr);
            m_stEvent = ptr;
        }, m_stEvent);
    }
}

SubsystemEvent& SubsystemEvent::operator=(const SubsystemEvent& rhs)
{
    if (this == &rhs)
        return *this;

    m_stEventStorage = rhs.m_stEventStorage;

    if (m_stEventStorage.has_value())
    {
        std::visit([&](auto value) {
            static_assert(std::is_pointer_v<decltype(value)>);
            auto ptr = std::any_cast<std::remove_pointer_t<decltype(value)>>(&m_stEventStorage);
            assert(ptr);
            m_stEvent = ptr;
        }, rhs.m_stEvent);
    }
    else
    {
        m_stEvent = rhs.m_stEvent;
    }
    return *this;
}

SubsystemEvent& SubsystemEvent::operator=(SubsystemEvent&& rhs) noexcept
{
    if (this == &rhs)
        return *this;

    m_stEvent = std::move(rhs.m_stEvent);
    m_stEventStorage = std::move(rhs.m_stEventStorage);

    if (m_stEventStorage.has_value())
    {
        std::visit([&](auto value) {
            static_assert(std::is_pointer_v<decltype(value)>);
            auto ptr = std::any_cast<std::remove_pointer_t<decltype(value)>>(&m_stEventStorage);
            assert(ptr);
            m_stEvent = ptr;
        }, m_stEvent);
    }
    return *this;
}

const SubsystemEvent::Event& SubsystemEvent::GetEvent() const noexcept
{
    return m_stEvent;
}

bool SubsystemEvent::IsDefaultPrevented() const noexcept
{
    return m_bIsDefaultPrevented;
}

void SubsystemEvent::PreventDefault() noexcept
{
    m_bIsDefaultPrevented = true;
}

bool SubsystemEvent::IsBubbles() const noexcept
{
    return m_bIsBubbles;
}

void SubsystemEvent::StopPropagation() noexcept
{
    m_bIsBubbles = false;
}
