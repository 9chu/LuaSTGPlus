/**
 * @file
 * @date 2022/3/3
 * @author 9chu
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#pragma once
#include "LuaRead.hpp"

namespace lstg::Subsystem::Script
{
    /**
     * Lua 模块注册器
     */
    class LuaModuleRegister
    {
    public:
        LuaModuleRegister(LuaStack& stack, const char* name, bool exposeToGlobalTable = false)
            : m_stStack(stack), m_stIndexOfModuleTable(0)
        {
            // 获取 package.loaded
            lua_getfield(m_stStack, LUA_ENVIRONINDEX, "_LOADED");  // t
            if (lua_istable(m_stStack, -1))
            {
                // 检查是否已经存在
                lua_getfield(m_stStack, -1, name);  // t t
                if (!lua_istable(m_stStack, -1))
                {
                    lua_pop(m_stStack, 1);  // t
                    lua_newtable(m_stStack);  // t t
                    lua_pushvalue(m_stStack, -1);  // t t t
                    lua_setfield(m_stStack, -3, name);  // t t
                }
                lua_remove(m_stStack, -2);  // t
                m_stIndexOfModuleTable = static_cast<unsigned>(lua_gettop(m_stStack));
            }
            else
            {
                assert(false);
                lua_pop(m_stStack, 1);
                lua_newtable(m_stStack);
                m_stIndexOfModuleTable = static_cast<unsigned>(lua_gettop(m_stStack));
            }

            if (exposeToGlobalTable)
            {
                lua_pushvalue(m_stStack, m_stIndexOfModuleTable);
                lua_setglobal(m_stStack, name);
            }
        }

        ~LuaModuleRegister()
        {
            lua_remove(m_stStack, m_stIndexOfModuleTable);
        }

    public:
        template <typename P>
        LuaModuleRegister& Put(const char* key, P&& value)
        {
            if (m_stIndexOfModuleTable == 0)
            {
                assert(false);
                return *this;
            }
            m_stStack.RawSet(m_stIndexOfModuleTable, key, std::forward<P>(value));
            return *this;
        }

    private:
        LuaStack& m_stStack;
        LuaStack::AbsIndex m_stIndexOfModuleTable;
    };
}
