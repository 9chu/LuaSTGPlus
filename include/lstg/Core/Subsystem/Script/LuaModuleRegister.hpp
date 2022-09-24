/**
 * @file
 * @date 2022/3/3
 * @author 9chu
 * 此文件为 LuaSTGPlus 项目的一部分，版权与许可声明详见 COPYRIGHT.txt。
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
        class EnumRegister
        {
        public:
            EnumRegister(LuaModuleRegister& parent, LuaStack::AbsIndex enumTable)
                : m_pParent(parent), m_stTable(enumTable)
            {
                assert(m_stTable.Index != 0);
                lua_checkstack(m_pParent.GetStack(), 3);
            }

            ~EnumRegister()
            {
                assert(m_stTable == 0);
            }

        public:
            template <typename P>
            EnumRegister& Put(const char* key, P&& value)
            {
                assert(m_stTable != 0);
                m_pParent.GetStack().RawSet(m_stTable, key, std::forward<P>(value));
                return *this;
            }

            LuaModuleRegister& End()
            {
                assert(m_stTable != 0);
                lua_remove(m_pParent.GetStack(), m_stTable);
                m_stTable = 0;
                return m_pParent;
            }

        private:
            LuaModuleRegister& m_pParent;
            LuaStack::AbsIndex m_stTable;
        };

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
        LuaStack& GetStack()
        {
            return m_stStack;
        }

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

        EnumRegister Enum(const char* name)
        {
            lua_newtable(m_stStack);
            LuaStack::AbsIndex index { static_cast<unsigned>(lua_gettop(m_stStack)) };
            lua_pushvalue(m_stStack, -1);
            lua_setfield(m_stStack, m_stIndexOfModuleTable, name);
            return {*this, index};
        }

    private:
        LuaStack& m_stStack;
        LuaStack::AbsIndex m_stIndexOfModuleTable;
    };
}
