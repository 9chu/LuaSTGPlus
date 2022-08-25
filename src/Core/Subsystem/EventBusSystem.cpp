/**
 * @file
 * @author 9chu
 * @date 2022/6/7
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#include <lstg/Core/Subsystem/EventBusSystem.hpp>

#include <lstg/Core/Logging.hpp>

using namespace std;
using namespace lstg;
using namespace lstg::Subsystem;

LSTG_DEF_LOG_CATEGORY(EventBusSystem);

EventBusSystem::EventBusSystem(SubsystemContainer& container)
{
}

Result<void> EventBusSystem::EmitEvent(SubsystemEvent event) noexcept
{
    try
    {
        m_stEventQueue.push_back(std::move(event));
        return {};
    }
    catch (...)
    {
        LSTG_LOG_ERROR_CAT(EventBusSystem, "Push event fail, out of memory");
        return make_error_code(std::errc::not_enough_memory);
    }
}

std::optional<SubsystemEvent> EventBusSystem::PollEvent() noexcept
{
    if (m_stEventQueue.empty())
        return {};
    auto front = std::move(m_stEventQueue.front());
    m_stEventQueue.pop_front();
    return front;
}
