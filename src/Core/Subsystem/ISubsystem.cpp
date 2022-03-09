/**
 * @file
 * @date 2022/3/6
 * @author 9chu
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#include <lstg/Core/Subsystem/ISubsystem.hpp>

using namespace std;
using namespace lstg;
using namespace lstg::Subsystem;

void ISubsystem::OnUpdate(double elapsedTime) noexcept
{
    static_cast<void>(elapsedTime);
}

void ISubsystem::OnBeforeRender(double elapsedTime) noexcept
{
    static_cast<void>(elapsedTime);
}

void ISubsystem::OnAfterRender(double elapsedTime) noexcept
{
    static_cast<void>(elapsedTime);
}

void ISubsystem::OnEvent(SubsystemEvent& event) noexcept
{
    static_cast<void>(event);
}
