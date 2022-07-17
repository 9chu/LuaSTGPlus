/**
 * @file
 * @author 9chu
 * @date 2022/4/17
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#include <lstg/v2/Bridge/MiscModule.hpp>

#include <lstg/Core/Logging.hpp>

using namespace std;
using namespace lstg;
using namespace lstg::v2::Bridge;

LSTG_DEF_LOG_CATEGORY(MiscModule);

void MiscModule::Registry()
{
    LSTG_LOG_WARN_CAT(MiscModule, "Registry is deprecated and has no effect anymore");
}

void MiscModule::CaptureSnapshot(const char* path)
{
    // TODO
//    LAPP.SnapShot(luaL_checkstring(L, 1));
}

bool MiscModule::Execute(const char* path, std::optional<std::string_view> arguments, std::optional<const char*> directory,
    std::optional<bool> wait /* =true */)
{
    // NOTE: 出于跨平台限定的考虑，不再提供 Execute 的支持
    LSTG_LOG_WARN_CAT(MiscModule, "Execute is deprecated and has no effect anymore");
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
