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
    namespace detail
    {
        template <typename T>
        struct IsFunctionOrMemberFunction
        {
            enum {
                value = std::is_function_v<std::remove_pointer_t<std::remove_reference_t<T>>> || std::is_member_function_pointer_v<T>
            };
        };
    }

    /**
     * 类注册器
     * @tparam T 类型
     */
    template <typename T>
    class LuaClassRegister
    {
    public:
        LuaClassRegister(LuaStack& stack, LuaStack::AbsIndex metatable)
            : m_stStack(stack), m_stMetaTable(metatable)
        {
            lua_checkstack(m_stStack, 5);
        }

    public:
        LuaStack& GetStack()
        {
            return m_stStack;
        }
        
        template <typename P>
        LuaClassRegister& Method(typename std::enable_if<detail::IsFunctionOrMemberFunction<P>::value, const char*>::type name, P&& func)
        {
            m_stStack.RawSet(m_stMetaTable, name, std::forward<P>(func));
            return *this;
        }

        template <typename P>
        LuaClassRegister& Getter(const char* name, P(T::*reader)())
        {
            m_stStack.PushValue("__get_");
            m_stStack.PushValue(name);
            m_stStack.Concat(2);
            m_stStack.PushValue(reader);
            m_stStack.RawSet(m_stMetaTable);
            return *this;
        }

        template <typename P>
        LuaClassRegister& Getter(const char* name, P(T::*reader)()const)
        {
            m_stStack.PushValue("__get_");
            m_stStack.PushValue(name);
            m_stStack.Concat(2);
            m_stStack.PushValue(reader);
            m_stStack.RawSet(m_stMetaTable);
            return *this;
        }

        template <typename P>
        LuaClassRegister& Setter(const char* name, void(T::*writer)(P))
        {
            m_stStack.PushValue("__set_");
            m_stStack.PushValue(name);
            m_stStack.Concat(2);
            m_stStack.PushValue(writer);
            m_stStack.RawSet(m_stMetaTable);
            return *this;
        }

        template <typename P>
        LuaClassRegister& GetterSetter(const char* name, P(T::*reader)(), void(T::*writer)(P))
        {
            Getter(name, reader);
            Setter(name, writer);
            return *this;
        }

        template <typename P>
        LuaClassRegister& Getter(const char* name, P(T::*reader)()const, void(T::*writer)(P))
        {
            Getter(name, reader);
            Setter(name, writer);
            return *this;
        }

    private:
        LuaStack& m_stStack;
        LuaStack::AbsIndex m_stMetaTable;
    };
}
