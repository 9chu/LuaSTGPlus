/**
 * @file
 * @date 2022/3/3
 * @author 9chu
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#pragma once
#include "LuaStack.hpp"

namespace lstg::Subsystem::Script
{
    /**
     * Lua 状态封装
     */
    class LuaState :
        public LuaStack
    {
    public:
        LuaState()
            : LuaStack(luaL_newstate())
        {
            if (!m_pState)
                throw std::system_error(make_error_code(std::errc::not_enough_memory));

            // lua5.1 没有 LUA_RIDX_MAINTHREAD，这里放置自己到 [REG]_mainthread 供 LuaReference 使用
            lua_pushthread(m_pState);
            lua_setfield(m_pState, LUA_REGISTRYINDEX, "_mainthread");
        }

        LuaState(const LuaState&) = delete;

        ~LuaState() noexcept
        {
            if (m_pState)
                lua_close(m_pState);
        }

    public:
        /**
         * 加载标准库
         */
        void OpenStandardLibrary()
        {
            luaL_openlibs(m_pState);
        }
    };
}
