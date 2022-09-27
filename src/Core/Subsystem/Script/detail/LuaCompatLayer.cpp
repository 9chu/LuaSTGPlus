/**
 * @file
 * @date 2022/9/27
 * @author 9chu
 * 此文件为 LuaSTGPlus 项目的一部分，版权与许可声明详见 COPYRIGHT.txt。
 */
#include "LuaCompatLayer.hpp"

using namespace std;
using namespace lstg;
using namespace lstg::Subsystem::Script::detail;

/*
 * 需要增加的模块：
 *  - [ ] bit
 *      - [ ] tobit
 *      - [ ] bnot
 *      - [ ] bswap
 *      - [ ] lshift
 *      - [ ] rshift
 *      - [ ] arshift
 *      - [ ] rol
 *      - [ ] ror
 *      - [ ] band
 *      - [ ] bor
 *      - [ ] bxor
 *      - [ ] tohex
 * 需要屏蔽的方法：
 *  - [ ] os.execute
 * 需要魔改的方法：
 *  - [ ] os.remove
 *  - [ ] os.rename
 *  - [ ] os.tmpname
 *  - [ ] require
 *  - [ ] loadfile
 *  - [ ] dofile
 *  - [ ] print
 * 需要魔改的模块：
 *  - [ ] io
 */

void LuaCompatLayer::Register(LuaState& state)
{
    Subsystem::Script::LuaStack::BalanceChecker stackChecker(state);

    // <editor-fold desc="math">

    // 修补高版本没有 math.mod
    lua_getglobal(state, "math");  // t
    assert(!lua_isnil(state, -1));
    lua_getfield(state, -1, "mod");  // t f|n
    if (lua_isnil(state, -1))
    {
        lua_pop(state, 1);  // t
        lua_getfield(state, -1, "fmod");  // t f
        lua_setfield(state, -2, "mod");  // t
        lua_pop(state, 1);
    }
    else
    {
        lua_pop(state, 2);
    }

    // </editor-fold>
}

