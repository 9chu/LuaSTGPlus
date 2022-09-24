/**
 * @file
 * @date 2022/8/25
 * @author 9chu
 * 此文件为 LuaSTGPlus 项目的一部分，版权与许可声明详见 COPYRIGHT.txt。
 */
#include <lstg/Core/Subsystem/GameController/Buttons.hpp>

#include <cassert>

using namespace std;
using namespace lstg;
using namespace lstg::Subsystem::GameController;

const char* Subsystem::GameController::ToString(Buttons button) noexcept
{
    switch (button)
    {
        case Buttons::A:
            return "A";
        case Buttons::B:
            return "B";
        case Buttons::X:
            return "X";
        case Buttons::Y:
            return "Y";
        case Buttons::Back:
            return "Back";
        case Buttons::Guide:
            return "Guide";
        case Buttons::Start:
            return "Start";
        case Buttons::LeftStick:
            return "LeftStick";
        case Buttons::RightStick:
            return "RightStick";
        case Buttons::LeftShoulder:
            return "LeftShoulder";
        case Buttons::RightShoulder:
            return "RightShoulder";
        case Buttons::DPadUp:
            return "DPadUp";
        case Buttons::DPadDown:
            return "DPadDown";
        case Buttons::DPadLeft:
            return "DPadLeft";
        case Buttons::DPadRight:
            return "DPadRight";
        case Buttons::Misc1:
            return "Misc1";
        case Buttons::Paddle1:
            return "Paddle1";
        case Buttons::Paddle2:
            return "Paddle2";
        case Buttons::Paddle3:
            return "Paddle3";
        case Buttons::Paddle4:
            return "Paddle4";
        case Buttons::TouchPad:
            return "TouchPad";
        default:
            assert(false);
            return "?";
    }
}

