/**
 * @file
 * @date 2022/3/6
 * @author 9chu
 * 此文件为 LuaSTGPlus 项目的一部分，版权与许可声明详见 COPYRIGHT.txt。
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
