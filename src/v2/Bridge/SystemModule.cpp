/**
 * @file
 * @author 9chu
 * @date 2022/4/20
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#include <lstg/v2/Bridge/SystemModule.hpp>

#include <lstg/Core/Logging.hpp>

using namespace std;
using namespace lstg;
using namespace lstg::v2::Bridge;

LSTG_DEF_LOG_CATEGORY(SystemModule);

void SystemModule::SetWindowed(bool windowed)
{
    // TODO
//    LAPP.SetWindowed(lua_toboolean(L, 1) == 0 ? false : true);
//    return 0;
}

void SystemModule::SetFPS(int32_t fps)
{
    // TODO
//    int v = luaL_checkinteger(L, 1);
//    if (v <= 0)
//        v = 60;
//    LAPP.SetFPS(static_cast<fuInt>(v));
//    return 0;
}

int32_t SystemModule::GetFPS()
{
    // TODO
//    lua_pushnumber(L, LAPP.GetFPS());
//    return 1;
    return 0;
}

void SystemModule::SetVsync(bool vsync)
{
    // TODO
//    LAPP.SetVsync(lua_toboolean(L, 1) == 0 ? false : true);
//    return 0;
}

void SystemModule::SetResolution(int32_t width, int32_t height)
{
    // TODO
//    LAPP.SetResolution(static_cast<fuInt>(::max(luaL_checkinteger(L, 1), 0)),
//        static_cast<fuInt>(::max(luaL_checkinteger(L, 2), 0)));
//    return 0;
}

bool SystemModule::ChangeVideoMode(int32_t width, int32_t height, bool windowed, bool vsync)
{
    // TODO
//    lua_pushboolean(L, LAPP.ChangeVideoMode(
//        luaL_checkinteger(L, 1),
//        luaL_checkinteger(L, 2),
//        lua_toboolean(L, 3) == 0 ? false : true,
//        lua_toboolean(L, 4) == 0 ? false : true
//    ));
//    return 1;
    return false;
}

void SystemModule::SetSplash(bool shown)
{
    // TODO
//    LAPP.SetSplash(lua_toboolean(L, 1) == 0 ? false : true);
//    return 0;
}

void SystemModule::SetTitle(const char* title)
{
    // TODO
//    LAPP.SetTitle(luaL_checkstring(L, 1));
//    return 0;
}

void SystemModule::SystemLog(LuaStack& stack, std::string_view what)
{
    // TODO
//    LINFO("脚本日志：%m", luaL_checkstring(L, 1));
//    return 0;
}

void SystemModule::Print(LuaStack& stack)
{
    // TODO
//    int n = lua_gettop(L);
//    lua_getglobal(L, "tostring"); // ... f
//    lua_pushstring(L, ""); // ... f s
//    for (int i = 1; i <= n; i++)
//    {
//        if (i > 1)
//        {
//            lua_pushstring(L, "\t"); // ... f s s
//            lua_concat(L, 2); // ... f s
//        }
//        lua_pushvalue(L, -2); // ... f s f
//        lua_pushvalue(L, i); // ... f s f arg[i]
//        lua_call(L, 1, 1); // ... f s ret
//        const char* x = luaL_checkstring(L, -1);
//        lua_concat(L, 2); // ... f s
//    }
//    LINFO("脚本日志：%m", luaL_checkstring(L, -1));
//    lua_pop(L, 2);
//    return 0;
}

void SystemModule::LoadPack(const char* path, std::optional<std::string_view> password)
{
    // TODO
//    const char* p = luaL_checkstring(L, 1);
//    const char* pwd = nullptr;
//    if (lua_isstring(L, 2))
//        pwd = luaL_checkstring(L, 2);
//    if (!LRES.LoadPack(p, pwd))
//        return luaL_error(L, "failed to load resource pack '%s'.", p);
//    return 0;
}

void SystemModule::UnloadPack(const char* path)
{
    // TODO
//    const char* p = luaL_checkstring(L, 1);
//    LRES.UnloadPack(p);
//    return 0;
}

void SystemModule::ExtractRes(const char* path, const char* target)
{
    LSTG_LOG_WARN_CAT(SystemModule, "ExtractRes is deprecated and has no effect anymore");
}

void SystemModule::DoFile(const char* path)
{
    // TODO
//    int args = lua_gettop(L);//获取此时栈上的值的数量
//    LAPP.LoadScript(luaL_checkstring(L, 1));
//    return (lua_gettop(L)- args);
}

void SystemModule::ShowSplashWindow(const char* path)
{
    LSTG_LOG_WARN_CAT(SystemModule, "ShowSplashWindow is deprecated and has no effect anymore");
}
