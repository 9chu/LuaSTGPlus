/**
 * @file
 * @date 2022/8/25
 * @author 9chu
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#pragma once

namespace lstg::Subsystem::GameController
{
    /**
     * 控制器按钮
     */
    enum class Buttons
    {
        A = 0,
        B,
        X,
        Y,
        Back,
        Guide,
        Start,
        LeftStick,
        RightStick,
        LeftShoulder,
        RightShoulder,
        DPadUp,
        DPadDown,
        DPadLeft,
        DPadRight,
        Misc1,
        Paddle1,
        Paddle2,
        Paddle3,
        Paddle4,
        TouchPad,
        Count_,
    };

    const char* ToString(Buttons button) noexcept;
}
