/**
 * @file
 * @date 2022/3/3
 * @author 9chu
 * 此文件为 LuaSTGPlus 项目的一部分，版权与许可声明详见 COPYRIGHT.txt。
 */
#pragma once
#include "LuaStack.hpp"

namespace lstg::Subsystem::Script
{
    /**
     * Lua 对象引用
     */
    class LuaReference
    {
    public:
        LuaReference() noexcept = default;

        LuaReference(LuaStack stack, int index)
            : m_stMainThread(stack)
        {
            lua_checkstack(m_stMainThread, 1);

            // 获取主线程
            lua_getfield(stack, LUA_REGISTRYINDEX, "_mainthread");
            if (!lua_isthread(stack, -1))
            {
                assert(false);
            }
            else
            {
                m_stMainThread = lua_tothread(stack, -1);
                lua_pop(stack, 1);
            }

            lua_pushvalue(stack, index);
            m_iRef = luaL_ref(stack, LUA_REGISTRYINDEX);
        }

        LuaReference(const LuaReference& rhs)
            : m_stMainThread(rhs.m_stMainThread), m_iRef(rhs.m_iRef)
        {
            if (m_iRef != LUA_NOREF && m_iRef != LUA_REFNIL)
            {
                lua_checkstack(m_stMainThread, 1);

                // 复制引用
                lua_rawgeti(m_stMainThread, LUA_REGISTRYINDEX, m_iRef);
                m_iRef = luaL_ref(m_stMainThread, LUA_REGISTRYINDEX);
            }
        }

        LuaReference(LuaReference&& rhs) noexcept
            : m_stMainThread(rhs.m_stMainThread), m_iRef(rhs.m_iRef)
        {
            rhs.m_iRef = LUA_NOREF;
        }

        ~LuaReference() noexcept
        {
            Reset();
        }

        LuaReference& operator=(const LuaReference& rhs)
        {
            Reset();

            auto ref = rhs.m_iRef;
            if (ref != LUA_NOREF && ref != LUA_REFNIL)
            {
                lua_checkstack(m_stMainThread, 1);

                // 复制引用
                lua_rawgeti(m_stMainThread, LUA_REGISTRYINDEX, ref);
                ref = luaL_ref(m_stMainThread, LUA_REGISTRYINDEX);
            }

            m_stMainThread = rhs.m_stMainThread;
            m_iRef = ref;
            return *this;
        }

        LuaReference& operator=(LuaReference&& rhs)noexcept
        {
            Reset();
            m_stMainThread = rhs.m_stMainThread;
            std::swap(m_iRef, rhs.m_iRef);
            return *this;
        }

        operator bool() const noexcept
        {
            return m_iRef != LUA_NOREF;
        }

    public:
        bool IsEmpty() const noexcept { return m_iRef == LUA_NOREF; }

        bool IsNil() const noexcept { return m_iRef == LUA_REFNIL; }

        bool IsEmptyOrNil() const noexcept { return m_iRef == LUA_NOREF || m_iRef == LUA_REFNIL; }

        void Push(LuaStack& stack) const
        {
            if (IsEmptyOrNil())
                lua_pushnil(stack);
            else
                lua_rawgeti(stack, LUA_REGISTRYINDEX, m_iRef);
        }

        void Reset()
        {
            if (m_iRef != LUA_NOREF && m_iRef != LUA_REFNIL)
                luaL_unref(m_stMainThread, LUA_REGISTRYINDEX, m_iRef);
            m_iRef = LUA_NOREF;
        }

    private:
        LuaStack m_stMainThread;  // 总是主线程
        int m_iRef = LUA_NOREF;
    };

    inline int LuaPush(LuaStack& stack, LuaReference& ref)
    {
        ref.Push(stack);
        return 1;
    }

    inline int LuaPush(LuaStack& stack, const LuaReference& ref)
    {
        ref.Push(stack);
        return 1;
    }

    inline int LuaRead(LuaStack& stack, int idx, LuaReference& out)
    {
        out = LuaReference {stack, idx};
        return 1;
    }
}
