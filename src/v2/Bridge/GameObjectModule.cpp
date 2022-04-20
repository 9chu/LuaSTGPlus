/**
 * @file
 * @author 9chu
 * @date 2022/4/17
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#include <lstg/v2/Bridge/GameObjectModule.hpp>

#include <lstg/Core/Logging.hpp>

using namespace std;
using namespace lstg;
using namespace lstg::v2::Bridge;

LSTG_DEF_LOG_CATEGORY(GameObjectModule);

Subsystem::Script::LuaStack::AbsIndex GameObjectModule::GetObjectTable()
{
    // TODO
//			return LPOOL.GetObjectTable(L);
    return Subsystem::Script::LuaStack::AbsIndex { 0u };
}

int32_t GameObjectModule::GetObjectCount()
{
    // TODO
//    lua_pushinteger(L, (lua_Integer)LPOOL.GetObjectCount());
//    return 1;
    return 0;
}

void GameObjectModule::UpdateObjectList()
{
    LSTG_LOG_WARN_CAT(GameObjectModule, "UpdateObjList is deprecated and has no effect anymore");
}

void GameObjectModule::UpdateObjects()
{
    // TODO
//    LPOOL.CheckIsMainThread(L);
//    LPOOL.DoFrame();
//    return 0;
}

void GameObjectModule::RenderObjects()
{
    // TODO
//    LPOOL.CheckIsMainThread(L);
//    LPOOL.DoRender();
//    return 0;
}

void GameObjectModule::SetBound(double left, double right, double bottom, double top)
{
    // TODO
//    LPOOL.SetBound(
//        luaL_checkinteger(L, 1),
//        luaL_checkinteger(L, 2),
//        luaL_checkinteger(L, 3),
//        luaL_checkinteger(L, 4)
//    );
//    return 0;
}

void GameObjectModule::BoundCheck()
{
    // TODO
//    LPOOL.CheckIsMainThread(L);
//    LPOOL.BoundCheck();
//    return 0;
}

void GameObjectModule::CollisionCheck(int32_t groupIdA, int32_t groupIdB)
{
    // TODO
//    LPOOL.CheckIsMainThread(L);
//    LPOOL.CollisionCheck(luaL_checkinteger(L, 1), luaL_checkinteger(L, 2));
//    return 0;
}

void GameObjectModule::UpdateXY()
{
    // TODO
//    LPOOL.CheckIsMainThread(L);
//    LPOOL.UpdateXY();
//    return 0;
}

void GameObjectModule::AfterFrame()
{
    // TODO
//    LPOOL.CheckIsMainThread(L);
//    LPOOL.AfterFrame();
//    return 0;
}

void GameObjectModule::NewObject(LuaStack& stack, AbsIndex cls)
{
    // TODO
//    return LPOOL.New(L);
}

void GameObjectModule::DelObject(LuaStack& stack, AbsIndex object)
{
    // TODO
//    return LPOOL.Del(L);
}

void GameObjectModule::KillObject(LuaStack& stack, AbsIndex object)
{
    // TODO
//    return LPOOL.Kill(L);
}

void GameObjectModule::IsObjectValid(LuaStack& stack, AbsIndex object)
{
    // TODO
//    return LPOOL.IsValid(L);
}

GameObjectModule::Unpack<double, double> GameObjectModule::GetObjectVelocity(LuaStack& stack, AbsIndex object)
{
    // TODO
//    if (!lua_istable(L, 1))
//        return luaL_error(L, "invalid lstg object for 'GetV'.");
//    double v, a;
//    lua_rawgeti(L, 1, 2);  // t(object) ??? id
//    if (!LPOOL.GetV((size_t)luaL_checkinteger(L, -1), v, a))
//        return luaL_error(L, "invalid lstg object for 'GetV'.");
//    lua_pushnumber(L, v);
//    lua_pushnumber(L, a);
//    return 2;
    return {0, 0};
}

void GameObjectModule::SetObjectVelocity(LuaStack& stack, AbsIndex object, double velocity, double angle, bool track)
{
    // TODO
//    if (!lua_istable(L, 1))
//        return luaL_error(L, "invalid lstg object for 'SetV'.");
//    if (lua_gettop(L) == 3)
//    {
//        lua_rawgeti(L, 1, 2);  // t(object) 'v' 'a' ??? id
//        if (!LPOOL.SetV((size_t)luaL_checkinteger(L, -1), luaL_checknumber(L, 2), luaL_checknumber(L, 3), false))
//            return luaL_error(L, "invalid lstg object for 'SetV'.");
//    }
//    else if (lua_gettop(L) == 4)
//    {
//        lua_rawgeti(L, 1, 2);  // t(object) 'v' 'a' 'rot' ??? id
//        if (!LPOOL.SetV((size_t)luaL_checkinteger(L, -1), luaL_checknumber(L, 2), luaL_checknumber(L, 3), lua_toboolean(L, 4) == 0 ? false : true))
//            return luaL_error(L, "invalid lstg object for 'SetV'.");
//    }
//    else
//        return luaL_error(L, "invalid argument count for 'SetV'.");
//    return 0;
}

void GameObjectModule::SetImageStateByObject(LuaStack& stack, AbsIndex object, const char* blend, int32_t a, int32_t r, int32_t g,
    int32_t b)
{
    // TODO
//    if (!lua_istable(L, 1))
//        return luaL_error(L, "invalid lstg object for 'SetImgState'.");
//    lua_rawgeti(L, 1, 2);  // t(object) ??? id
//    size_t id = (size_t)luaL_checkinteger(L, -1);
//    lua_pop(L, 1);
//
//    BlendMode m = TranslateBlendMode(L, 2);
//    fcyColor c(luaL_checkinteger(L, 3), luaL_checkinteger(L, 4), luaL_checkinteger(L, 5), luaL_checkinteger(L, 6));
//    if (!LPOOL.SetImgState(id, m, c))
//        return luaL_error(L, "invalid lstg object for 'SetImgState'.");
//    return 0;
}

double GameObjectModule::CalculateAngle(LuaStack& stack, std::variant<double, AbsIndex> a, std::variant<double, AbsIndex> b,
    std::optional<double> c, std::optional<double> d)
{
    // TODO
//    if (lua_gettop(L) == 2)
//    {
//        if (!lua_istable(L, 1) || !lua_istable(L, 2))
//            return luaL_error(L, "invalid lstg object for 'Angle'.");
//        lua_rawgeti(L, 1, 2);  // t(object) t(object) ??? id
//        lua_rawgeti(L, 2, 2);  // t(object) t(object) ??? id id
//        double tRet;
//        if (!LPOOL.Angle((size_t)luaL_checkint(L, -2), (size_t)luaL_checkint(L, -1), tRet))
//            return luaL_error(L, "invalid lstg object for 'Angle'.");
//        lua_pushnumber(L, tRet);
//    }
//    else
//    {
//        lua_pushnumber(L,
//            atan2(luaL_checknumber(L, 4) - luaL_checknumber(L, 2), luaL_checknumber(L, 3) - luaL_checknumber(L, 1)) * LRAD2DEGREE
//        );
//    }
//    return 1;
    return 0;
}

double GameObjectModule::CalculateDistance(LuaStack& stack, std::variant<double, AbsIndex> a, std::variant<double, AbsIndex> b,
    std::optional<double> c, std::optional<double> d)
{
    // TODO
//    if (lua_gettop(L) == 2)
//    {
//        if (!lua_istable(L, 1) || !lua_istable(L, 2))
//            return luaL_error(L, "invalid lstg object for 'Dist'.");
//        lua_rawgeti(L, 1, 2);  // t(object) t(object) id
//        lua_rawgeti(L, 2, 2);  // t(object) t(object) id id
//        double tRet;
//        if (!LPOOL.Dist((size_t)luaL_checkint(L, -2), (size_t)luaL_checkint(L, -1), tRet))
//            return luaL_error(L, "invalid lstg object for 'Dist'.");
//        lua_pushnumber(L, tRet);
//    }
//    else
//    {
//        lua_Number dx = luaL_checknumber(L, 3) - luaL_checknumber(L, 1);
//        lua_Number dy = luaL_checknumber(L, 4) - luaL_checknumber(L, 2);
//        lua_pushnumber(L, sqrt(dx*dx + dy*dy));
//    }
//    return 1;
    return 0;
}

bool GameObjectModule::BoxCheck(LuaStack& stack, AbsIndex object, double left, double right, double top, double bottom)
{
    // TODO
//    if (!lua_istable(L, 1))
//        return luaL_error(L, "invalid lstg object for 'BoxCheck'.");
//    lua_rawgeti(L, 1, 2);  // t(object) 'l' 'r' 't' 'b' ??? id
//    bool tRet;
//    if (!LPOOL.BoxCheck(
//        (size_t)luaL_checkinteger(L, -1),
//        luaL_checknumber(L, 2),
//        luaL_checknumber(L, 3),
//        luaL_checknumber(L, 4),
//        luaL_checknumber(L, 5),
//        tRet))
//    {
//        return luaL_error(L, "invalid lstg object for 'BoxCheck'.");
//    }
//    lua_pushboolean(L, tRet);
//    return 1;
    return false;
}

void GameObjectModule::ResetPool()
{
    // TODO
//    LPOOL.ResetPool();
//    return 0;
}

void GameObjectModule::DefaultRenderFunc(LuaStack& stack, AbsIndex object)
{
    // TODO
//    if (!lua_istable(L, 1))
//        return luaL_error(L, "invalid lstg object for 'DefaultRenderFunc'.");
//    lua_rawgeti(L, 1, 2);  // t(object) ??? id
//    if (!LPOOL.DoDefaultRender(luaL_checkinteger(L, -1)))
//        return luaL_error(L, "invalid lstg object for 'DefaultRenderFunc'.");
//    return 0;
}

GameObjectModule::Unpack<int32_t, GameObjectModule::AbsIndex> GameObjectModule::NextObject(int32_t groupId, int32_t id)
{
    // TODO
//    return LPOOL.NextObject(L);
    return {0, AbsIndex {}};
}

GameObjectModule::Unpack<GameObjectModule::AbsIndex, int32_t, int32_t> GameObjectModule::EnumerateObjectList(int32_t groupId)
{
    // TODO
//    int g = luaL_checkinteger(L, 1);  // i(groupId)
//    lua_pushcfunction(L, WrapperImplement::NextObject);
//    lua_pushinteger(L, g);
//    lua_pushinteger(L, LPOOL.FirstObject(g));
//    return 3;
    return {{}, 0, 0};
}

void GameObjectModule::ParticleFire(LuaStack& stack, AbsIndex object)
{
    // TODO
//    return LPOOL.ParticleFire(L);
}

void GameObjectModule::ParticleStop(LuaStack& stack, AbsIndex object)
{
    // TODO
//    return LPOOL.ParticleStop(L);
}

int32_t GameObjectModule::ParticleGetCount(LuaStack& stack, AbsIndex object)
{
    // TODO
//    return LPOOL.ParticleGetn(L);
    return 0;
}

int32_t GameObjectModule::ParticleGetEmission(LuaStack& stack, AbsIndex object)
{
    // TODO
//    return LPOOL.ParticleGetEmission(L);
    return 0;
}

void GameObjectModule::ParticleSetEmission(LuaStack& stack, AbsIndex object, int32_t count)
{
    // TODO
//    return LPOOL.ParticleSetEmission(L);
}
