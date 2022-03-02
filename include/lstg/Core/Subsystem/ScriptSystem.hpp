/**
 * @file
 * @date 2022/3/2
 * @author 9chu
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#pragma once

#ifdef __EMSCRIPTEN__
// include lua5.1
extern "C" {
#include <lua.h>
};
#else
// include luajit
#include <lua.hpp>
#endif

namespace lstg::Subsystem
{
    /**
     * 脚本子系统
     */
    class ScriptSystem
    {
    public:

    };
}
