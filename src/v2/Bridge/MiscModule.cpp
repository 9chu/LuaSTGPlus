/**
 * @file
 * @author 9chu
 * @date 2022/4/17
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#include <lstg/v2/Bridge/MiscModule.hpp>

using namespace std;
using namespace lstg;
using namespace lstg::v2::Bridge;

void MiscModule::CaptureSnapshot(const char* path)
{
    // TODO
//    LAPP.SnapShot(luaL_checkstring(L, 1));
}

bool MiscModule::Execute(const char* path, std::optional<std::string_view> arguments, std::optional<const char*> directory,
    std::optional<bool> wait /* =true */)
{
    // TODO
//    struct Detail_
//    {
//        LNOINLINE static bool Execute(const char* path, const char* args, const char* directory, bool bWait, bool bShow)LNOEXCEPT
//        {
//            wstring tPath, tArgs, tDirectory;
//
//            try
//            {
//                tPath = fcyStringHelper::MultiByteToWideChar(path, CP_UTF8);
//                tArgs = fcyStringHelper::MultiByteToWideChar(args, CP_UTF8);
//                if (directory)
//                    tDirectory = fcyStringHelper::MultiByteToWideChar(directory, CP_UTF8);
//
//                SHELLEXECUTEINFO tShellExecuteInfo;
//                memset(&tShellExecuteInfo, 0, sizeof(SHELLEXECUTEINFO));
//
//                tShellExecuteInfo.cbSize = sizeof(SHELLEXECUTEINFO);
//                tShellExecuteInfo.fMask = bWait ? SEE_MASK_NOCLOSEPROCESS : 0;
//                tShellExecuteInfo.lpVerb = L"open";
//                tShellExecuteInfo.lpFile = tPath.c_str();
//                tShellExecuteInfo.lpParameters = tArgs.c_str();
//                tShellExecuteInfo.lpDirectory = directory ? tDirectory.c_str() : nullptr;
//                tShellExecuteInfo.nShow = bShow ? SW_SHOWDEFAULT : SW_HIDE;
//
//                if (FALSE == ShellExecuteEx(&tShellExecuteInfo))
//                    return false;
//
//                if (bWait)
//                {
//                    WaitForSingleObject(tShellExecuteInfo.hProcess, INFINITE);
//                    CloseHandle(tShellExecuteInfo.hProcess);
//                }
//                return true;
//            }
//            catch (const std::bad_alloc&)
//            {
//                return false;
//            }
//        }
//    };
//
//    const char* path = luaL_checkstring(L, 1);
//    const char* args = luaL_optstring(L, 2, "");
//    const char* directory = luaL_optstring(L, 3, NULL);
//    bool bWait = true;
//    bool bShow = true;
//    if (lua_gettop(L) >= 4)
//        bWait = lua_toboolean(L, 4) == 0 ? false : true;
//    if (lua_gettop(L) >= 5)
//        bShow = lua_toboolean(L, 5) == 0 ? false : true;
//
//    lua_pushboolean(L, Detail_::Execute(path, args, directory, bWait, bShow));
//    return 1;
    return false;
}

LSTGRandomizer MiscModule::NewRandomizer()
{
    LSTGRandomizer ret {};
    ret.SetSeed(chrono::duration_cast<chrono::milliseconds>(chrono::system_clock::now().time_since_epoch()).count());
    return ret;
}

LSTGColor MiscModule::NewColor(Subsystem::Script::LuaStack& stack)
{
    LSTGColor ret {};
    if (stack.GetTop() == 1)
    {
        auto argb = luaL_checkinteger(stack, 1);
        ret.r((argb & 0x00FF0000) >> 16);
        ret.g((argb & 0x0000FF00) >> 8);
        ret.b(argb & 0x000000FF);
        ret.a((argb & 0xFF000000) >> 24);
        return ret;
    }

    auto a = luaL_checkinteger(stack, 1);
    auto r = luaL_checkinteger(stack, 2);
    auto g = luaL_checkinteger(stack, 3);
    auto b = luaL_checkinteger(stack, 4);
    ret.r(std::clamp<int>(r, 0, 255));
    ret.g(std::clamp<int>(g, 0, 255));
    ret.b(std::clamp<int>(b, 0, 255));
    ret.a(std::clamp<int>(a, 0, 255));
    return ret;
}

LSTGBentLaserData MiscModule::NewBentLaserData()
{
    // TODO
    return LSTGBentLaserData {};
}
