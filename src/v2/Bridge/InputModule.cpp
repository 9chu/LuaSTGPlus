/**
 * @file
 * @author 9chu
 * @date 2022/4/17
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#include <lstg/v2/Bridge/InputModule.hpp>

#include "detail/Helper.hpp"

using namespace std;
using namespace lstg;
using namespace lstg::v2::Bridge;

bool InputModule::GetKeyState(int32_t vkCode)
{
    auto& app = detail::GetGlobalApp();
    return app.IsKeyDown(vkCode);
}

int32_t InputModule::GetLastKey()
{
    auto& app = detail::GetGlobalApp();
    return app.GetLastInputKeyCode();
}

const char* InputModule::GetLastChar()
{
    auto& app = detail::GetGlobalApp();
    return app.GetLastInputChar().c_str();
}

Subsystem::Script::Unpack<double, double> InputModule::GetMousePosition()
{
    auto& app = detail::GetGlobalApp();
    auto pos = app.GetMousePosition();
    return { pos.x, pos.y };
}

bool InputModule::GetMouseState(int32_t button)
{
    auto& app = detail::GetGlobalApp();
    MouseButtons transButton = MouseButtons::MAX;
    switch (button)
    {
        case 0:
            transButton = MouseButtons::Left;
            break;
        case 1:
            transButton = MouseButtons::Middle;
            break;
        case 2:
            transButton = MouseButtons::Right;
            break;
        default:
            return false;
    }
    return app.IsMouseButtonDown(transButton);
}
