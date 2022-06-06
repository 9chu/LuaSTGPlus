/**
 * @file
 * @author 9chu
 * @date 2022/4/20
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#include <lstg/v2/Bridge/SystemModule.hpp>

#include <cassert>
#include <lstg/Core/Logging.hpp>
#include <lstg/Core/Subsystem/ProfileSystem.hpp>
#include <lstg/Core/Subsystem/RenderSystem.hpp>
#include <lstg/Core/Subsystem/ScriptSystem.hpp>
#include <lstg/v2/Bridge/Helper.hpp>

using namespace std;
using namespace lstg;
using namespace lstg::v2::Bridge;

using namespace lstg::Subsystem;

LSTG_DEF_LOG_CATEGORY(SystemModule);

namespace
{
    lstg::detail::LogSourceLocation GetScriptLocation(Script::LuaStack& stack)
    {
        lua_Debug ar;
        ::memset(&ar, 0, sizeof(ar));
        if (1 == ::lua_getstack(stack, 1, &ar))
            ::lua_getinfo(stack, "nSlt", &ar);

        // 解出文件名
        const char* fileName = "?";
        if (ar.source)
        {
            if (ar.source[0] == '@')
                fileName = &ar.source[1];
            else if (ar.source[0] == '=')
                fileName = "<source>";
            else
                fileName = ar.source;
        }

        return { fileName, ar.name ? ar.name : "?", ar.currentline };
    }

    // luaL_tolstring @ https://www.lua.org/source/5.3/lauxlib.c.html
    const char* PrettyFormat(lua_State* L, int idx, size_t* len)
    {
        if (::luaL_callmeta(L, idx, "__tostring"))
        {
            if (!::lua_isstring(L, -1))
                ::luaL_error(L, "'__tostring' must return a string");
        }
        else
        {
            switch (::lua_type(L, idx))
            {
                case LUA_TNUMBER:
                    ::lua_pushfstring(L, "%f", (LUAI_UACNUMBER)::lua_tonumber(L, idx));
                    break;
                case LUA_TSTRING:
                    ::lua_pushvalue(L, idx);
                    break;
                case LUA_TBOOLEAN:
                    ::lua_pushstring(L, (::lua_toboolean(L, idx) ? "true" : "false"));
                    break;
                case LUA_TNIL:
                    lua_pushliteral(L, "nil");
                    break;
                default:
                    {
                        int tt = ::luaL_getmetafield(L, idx, "__name");
                        const char* kind = (tt == LUA_TSTRING) ? lua_tostring(L, -1) : luaL_typename(L, idx);
                        ::lua_pushfstring(L, "<%s: %p>", kind, ::lua_topointer(L, idx));
                        if (tt != LUA_TNIL)
                            ::lua_remove(L, -2);
                    }
                    break;
            }
        }
        return ::lua_tolstring(L, -1, len);
    }
}

void SystemModule::SetWindowed(bool windowed)
{
    GetApp().ToggleFullScreen(!windowed);
}

void SystemModule::SetFPS(int32_t fps)
{
    GetApp().SetFrameRate(fps);
}

int32_t SystemModule::GetFPS()
{
    auto profile = GetApp().GetSubsystem<ProfileSystem>();
    assert(profile);
    return static_cast<int32_t>(::round(profile->GetPerformanceCounter(PerformanceCounterTypes::RealTime, "LogicFps")));
}

void SystemModule::SetVsync(bool vsync)
{
    auto render = GetApp().GetSubsystem<RenderSystem>();
    assert(render);
    render->GetRenderDevice()->SetVerticalSyncEnabled(vsync);
}

void SystemModule::SetResolution(int32_t width, int32_t height)
{
    GetApp().ChangeDesiredResolution(static_cast<uint32_t>(std::max(1, width)), static_cast<uint32_t>(std::max(1, height)));
}

bool SystemModule::ChangeVideoMode(int32_t width, int32_t height, bool windowed, bool vsync)
{
    SetWindowed(windowed);
    SetResolution(width, height);
    SetVsync(vsync);
    return true;
}

void SystemModule::SetSplash(bool shown)
{
    auto window = GetApp().GetSubsystem<WindowSystem>();
    auto ret = window->SetMouseCursorVisible(shown);
    if (!ret)
        LSTG_LOG_ERROR_CAT(SystemModule, "SetMouseCursorVisible fail: {}", ret.GetError());
}

void SystemModule::SetTitle(const char* title)
{
    auto window = GetApp().GetSubsystem<WindowSystem>();
    window->SetTitle(title);
}

void SystemModule::SystemLog(LuaStack& stack, std::string_view what)
{
    auto loc = GetScriptLocation(stack);
    Logging::GetInstance().Log("LUA", LogLevel::Info, lstg::detail::GetLogCurrentTime(), loc, "{}", what);
}

void SystemModule::Print(LuaStack& stack)
{
    auto loc = GetScriptLocation(stack);
    auto n = lua_gettop(stack);
    lua_checkstack(stack, 3);
    lua_pushliteral(stack, "");
    for (auto i = 1; i <= n; ++i)
    {
        if (i > 1)
        {
            lua_pushliteral(stack, "\t");
            lua_concat(stack, 2);
        }
        PrettyFormat(stack, i, nullptr);
        lua_concat(stack, 2);
    }
    Logging::GetInstance().Log("LUA", LogLevel::Info, lstg::detail::GetLogCurrentTime(), loc, "{}", lua_tostring(stack, -1));
    lua_pop(stack, 1);
}

void SystemModule::LoadPack(Script::LuaStack& stack, const char* path, std::optional<std::string_view> password)
{
    auto ec = GetApp().MountAssetPack(path, password);
    if (!ec)
        stack.Error("Load asset pack from \"%s\" fail: %s", path, ec.GetError().message().c_str());
}

void SystemModule::UnloadPack(Script::LuaStack& stack, const char* path)
{
    auto ec = GetApp().UnmountAssetPack(path);
    if (!ec)
        stack.Error("Unload asset pack from \"%s\" fail: %s", path, ec.GetError().message().c_str());
}

void SystemModule::ExtractRes(const char* path, const char* target)
{
    LSTG_LOG_WARN_CAT(SystemModule, "ExtractRes is deprecated and has no effect anymore");
}

void SystemModule::DoFile(LuaStack& stack, const char* path)
{
    auto ec = GetApp().GetSubsystem<ScriptSystem>()->LoadScript(path);
    if (!ec)
        stack.Error("Load script from \"%s\" fail: %s", path, ec.GetError().message().c_str());
}

void SystemModule::ShowSplashWindow(std::optional<std::string_view> path)
{
    LSTG_LOG_WARN_CAT(SystemModule, "ShowSplashWindow is deprecated and has no effect anymore");
}
