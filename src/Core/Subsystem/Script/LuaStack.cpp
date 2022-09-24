/**
 * @file
 * @date 2022/3/6
 * @author 9chu
 * 此文件为 LuaSTGPlus 项目的一部分，版权与许可声明详见 COPYRIGHT.txt。
 */
#include <lstg/Core/Subsystem/Script/LuaStack.hpp>

using namespace std;
using namespace lstg;
using namespace lstg::Subsystem::Script;

int lstg::Subsystem::Script::detail::PCallErrorHandler(lua_State* L)
{
    // see lua.c: traceback
    if (!lua_isstring(L, 1))
        return 1;
    lua_getfield(L, LUA_GLOBALSINDEX, "debug");
    if (!lua_istable(L, -1))
    {
        lua_pop(L, 1);
        return 1;
    }
    lua_getfield(L, -1, "traceback");
    if (!lua_isfunction(L, -1))
    {
        lua_pop(L, 2);
        return 1;
    }
    lua_pushvalue(L, 1);
    lua_pushinteger(L, 2);
    lua_call(L, 2, 1);
    return 1;
}
