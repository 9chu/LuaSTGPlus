/**
 * @file
 * @author 9chu
 * @date 2022/4/17
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#include <lstg/v2/Bridge/InputModule.hpp>

using namespace std;
using namespace lstg;
using namespace lstg::v2::Bridge;

bool InputModule::GetKeyState(int32_t vkCode)
{
    // TODO
//    lua_pushboolean(L, LAPP.GetKeyState(luaL_checkinteger(L, -1)));
//    return 1;
    return false;
}

int32_t InputModule::GetLastKey()
{
    // TODO
//    lua_pushinteger(L, LAPP.GetLastKey());
//    return 1;
    return 0;
}

const char* InputModule::GetLastChar()
{
    // TODO
//    return LAPP.GetLastChar(L);
    return "";
}

Subsystem::Script::Unpack<double, double> InputModule::GetMousePosition()
{
    // TODO
//    fcyVec2 tPos = LAPP.GetMousePosition();
//    lua_pushnumber(L, tPos.x);
//    lua_pushnumber(L, tPos.y);
//    return 2;
    return {0, 0};
}

bool InputModule::GetMouseState(int32_t button)
{
    // TODO
//    lua_pushboolean(L, LAPP.GetMouseState(luaL_checkinteger(L, 1)));
//    return 1;
    return false;
}
