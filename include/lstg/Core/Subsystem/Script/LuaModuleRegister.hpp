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
            lua_checkstack(m_stStack, 4);

            // 获取 package.loaded
            lua_getglobal(m_stStack, "package");  // ?
            if (lua_istable(m_stStack, -1))  // t(package)
            {
                lua_getfield(m_stStack, -1, "loaded");  // t(package) ?
                if (lua_istable(m_stStack, -1))  // t(package) t(loaded)
                {
                    // 检查是否已经存在
                    lua_getfield(m_stStack, -1, name);  // t(package) t(loaded) ?
                    if (!lua_istable(m_stStack, -1))  // t(package) t(loaded) t(name)
                    {
                        lua_pop(m_stStack, 1);  // t(package) t(loaded)
                        lua_newtable(m_stStack);  // t(package) t(loaded) t
                        lua_pushvalue(m_stStack, -1);  // t(package) t(loaded) t t
                        lua_setfield(m_stStack, -3, name);  // t(package) t(loaded) t
                    }
                    lua_remove(m_stStack, -3);  // t(loaded) t
                    lua_remove(m_stStack, -2);  // t
                    m_stIndexOfModuleTable = static_cast<unsigned>(lua_gettop(m_stStack));
                    assert(m_stIndexOfModuleTable != 0);
                }
                else
                {
                    lua_pop(m_stStack, 2);
                }
            }
            else
            {
                lua_pop(m_stStack, 1);
            }

            if (m_stIndexOfModuleTable == 0)
            {
                assert(false);
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
