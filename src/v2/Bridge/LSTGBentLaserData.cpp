/**
 * @file
 * @author 9chu
 * @date 2022/4/17
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#include <lstg/v2/Bridge/LSTGBentLaserData.hpp>

using namespace std;
using namespace lstg;
using namespace lstg::v2::Bridge;

LSTGBentLaserData::~LSTGBentLaserData()
{
    Release();
}

void LSTGBentLaserData::Update(Subsystem::Script::LuaStack& stack, Subsystem::Script::LuaStack::AbsIndex baseObject, uint32_t length,
    uint32_t width)
{
    // TODO
//    Wrapper* p = static_cast<Wrapper*>(luaL_checkudata(L, 1, TYPENAME_BENTLASER));
//    if (!p->handle)
//        return luaL_error(L, "lstgBentLaserData was released.");
//    if (!lua_istable(L, 2))
//        return luaL_error(L, "invalid lstg object for 'Update'.");
//    lua_rawgeti(L, 2, 2);  // self t(object) ??? id
//    size_t id = (size_t)luaL_checkinteger(L, -1);
//    lua_pop(L, 1);
//    if (!p->handle->Update(id, luaL_checkinteger(L, 3), (float)luaL_checknumber(L, 4)))
//        return luaL_error(L, "invalid lstg object for 'Update'.");
//    return 0;
}

void LSTGBentLaserData::Release()
{
    // TODO
//    Wrapper* p = static_cast<Wrapper*>(luaL_checkudata(L, 1, TYPENAME_BENTLASER));
//    if (p->handle)
//    {
//        GameObjectBentLaser::FreeInstance(p->handle);
//        p->handle = nullptr;
//    }
//    return 0;
}

void LSTGBentLaserData::Render(const char* texture, const char* blend, LSTGColor* color, double texLeft, double texTop, double texWidth,
    double texHeight, std::optional<double> scale /* =1 */) const
{
    // TODO
//    Wrapper* p = static_cast<Wrapper*>(luaL_checkudata(L, 1, TYPENAME_BENTLASER));
//    if (!p->handle)
//        return luaL_error(L, "lstgBentLaserData was released.");
//    if (!p->handle->Render(
//        luaL_checkstring(L, 2),
//        TranslateBlendMode(L, 3),
//        *static_cast<fcyColor*>(luaL_checkudata(L, 4, TYPENAME_COLOR)),
//        (float)luaL_checknumber(L, 5),
//        (float)luaL_checknumber(L, 6),
//        (float)luaL_checknumber(L, 7),
//        (float)luaL_checknumber(L, 8),
//        (float)luaL_optnumber(L, 9, 1.) * LRES.GetGlobalImageScaleFactor()
//    ))
//    {
//        return luaL_error(L, "can't render object with texture '%s'.", luaL_checkstring(L, 2));
//    }
//    return 0;
}

bool LSTGBentLaserData::CollisionCheck(double x, double y, std::optional<double> rot /* =0 */, std::optional<double> a /* =0 */,
    std::optional<double> b /* =0 */, std::optional<bool> rect /* =false */) const
{
    // TODO
//    Wrapper* p = static_cast<Wrapper*>(luaL_checkudata(L, 1, TYPENAME_BENTLASER));
//    if (!p->handle)
//        return luaL_error(L, "lstgBentLaserData was released.");
//    bool r = p->handle->CollisionCheck(
//        (float)luaL_checknumber(L, 2),
//        (float)luaL_checknumber(L, 3),
//        (float)luaL_optnumber(L, 4, 0),
//        (float)luaL_optnumber(L, 5, 0),
//        (float)luaL_optnumber(L, 6, 0),
//        lua_toboolean(L, 7) == 0 ? false : true
//    );
//    lua_pushboolean(L, r);
//    return 1;
    return false;
}

bool LSTGBentLaserData::BoundCheck() const
{
    // TODO
//    Wrapper* p = static_cast<Wrapper*>(luaL_checkudata(L, 1, TYPENAME_BENTLASER));
//    if (!p->handle)
//        return luaL_error(L, "lstgBentLaserData was released.");
//    bool r = p->handle->BoundCheck();
//    lua_pushboolean(L, r);
//    return 1;
    return false;
}

std::string LSTGBentLaserData::ToString() const
{
    return "lstgBentLaserData";
}
