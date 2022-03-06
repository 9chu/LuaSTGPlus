/**
 * @file
 * @date 2022/3/6
 * @author 9chu
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#include <lstg/Core/Subsystem/SubsystemEvent.hpp>

using namespace std;
using namespace lstg;
using namespace lstg::Subsystem;

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
    m_bIsDefaultPrevented = false;
}

bool SubsystemEvent::IsBubbles() const noexcept
{
    return m_bIsBubbles;
}

void SubsystemEvent::StopPropagation() noexcept
{
    m_bIsBubbles = false;
}
