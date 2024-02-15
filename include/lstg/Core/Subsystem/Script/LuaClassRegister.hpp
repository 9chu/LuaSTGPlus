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
    private:
        template <typename P>
        struct PropertyAccessor
        {
            using UqClass = detail::Unqualified<T>;

            static int GetterWrapper(lua_State* L)
            {
                // 获得成员偏移量
                assert(!lua_isnoneornil(L, lua_upvalueindex(1)));
                auto memberOffset = static_cast<ptrdiff_t>(lua_tointeger(L, lua_upvalueindex(1)));

                // 获取对象指针
                auto p = static_cast<detail::NativeObjectStorage<UqClass>*>(
                    luaL_checkudata(L, 1, detail::GetUniqueTypeName<UqClass>().Name.c_str()));
                if (!p)
                {
                    assert(false);
                    return 0;
                }

                // 访问对象
                auto address = reinterpret_cast<const uint8_t*>(static_cast<void*>(p->Object)) + memberOffset;
                auto member = reinterpret_cast<const P*>(address);

                // 设置到栈上
                LuaStack st(L);
                return st.PushValue(*member);
            }

            static int SetterWrapper(lua_State* L)
            {
                // 获得成员偏移量
                assert(!lua_isnoneornil(L, lua_upvalueindex(1)));
                auto memberOffset = static_cast<ptrdiff_t>(lua_tointeger(L, lua_upvalueindex(1)));

                // 获取对象指针
                auto p = static_cast<detail::NativeObjectStorage<UqClass>*>(
                    luaL_checkudata(L, 1, detail::GetUniqueTypeName<UqClass>().Name.c_str()));
                if (!p)
                {
                    assert(false);
                    return 0;
                }

                // 访问对象
                auto address = reinterpret_cast<uint8_t*>(static_cast<void*>(p->Object)) + memberOffset;
                auto member = reinterpret_cast<P*>(address);

                // 获取值
                LuaStack st(L);

                P val;
                st.ReadValue(2, val);

                // 设置到对象
                *member = std::move(val);
                return 0;
            }
        };

    public:
        LuaClassRegister(LuaStack& stack, LuaStack::AbsIndex metatable)
            : m_stStack(stack), m_stMetaTable(metatable)
        {
            lua_checkstack(m_stStack, 5);
        }

    public:
        /**
         * 获取上下文栈
         */
        LuaStack& GetStack()
        {
            return m_stStack;
        }

        /**
         * 注册方法
         * 注册后的方法通过 foo:bar(...) 进行调用。
         * @tparam P 成员函数类型
         * @param name 函数名
         * @param func 函数指针
         * @return 注册器自身
         */
        template <typename P>
        LuaClassRegister& Method(typename std::enable_if<detail::IsFunctionOrMemberFunction<P>::value, const char*>::type name, P&& func)
        {
            m_stStack.RawSet(m_stMetaTable, name, std::forward<P>(func));
            return *this;
        }

        /**
         * 注册取属性访问器
         * 注册后通过 foo.bar 进行访问。
         * @tparam P 属性类型
         * @param name 属性名
         * @param reader 读访问器
         * @return 注册器自身
         */
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

        /**
         * 注册写属性访问器
         * 注册后通过 foo.bar 进行访问。
         * @tparam P 属性类型
         * @param name 属性名
         * @param writer 写访问器
         * @return 注册器自身
         */
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

        /**
         * 注册读写属性访问器
         * 注册后通过 foo.bar 进行访问。
         * @tparam P 属性类型
         * @param name 属性名
         * @param reader 读访问器
         * @param writer 写访问器
         * @return 注册器自身
         */
        template <typename P>
        LuaClassRegister& GetterSetter(const char* name, P(T::*reader)(), void(T::*writer)(P))
        {
            Getter(name, reader);
            Setter(name, writer);
            return *this;
        }

        template <typename P>
        LuaClassRegister& GetterSetter(const char* name, P(T::*reader)()const, void(T::*writer)(P))
        {
            Getter(name, reader);
            Setter(name, writer);
            return *this;
        }

        /**
         * 直接注册属性
         * 注册后通过 foo.bar 进行访问。
         * @tparam P 属性类型
         * @param name 属性名
         * @param member 属性成员指针
         * @param readOnly 是否只读
         * @return 注册器自身
         */
        template <typename P>
        LuaClassRegister& Property(const char* name, P T::*member, bool readOnly = false)
        {
            auto offset = reinterpret_cast<const uint8_t*>(&(static_cast<T*>(nullptr)->*member)) - reinterpret_cast<const uint8_t*>(0);

            // Bind getter
            m_stStack.PushValue("__get_");
            m_stStack.PushValue(name);
            m_stStack.Concat(2);
            lua_pushinteger(m_stStack, offset);
            lua_pushcclosure(m_stStack, PropertyAccessor<P>::GetterWrapper, 1);
            m_stStack.RawSet(m_stMetaTable);

            // Bind setter
            if (!readOnly)
            {
                m_stStack.PushValue("__set_");
                m_stStack.PushValue(name);
                m_stStack.Concat(2);
                lua_pushinteger(m_stStack, offset);
                lua_pushcclosure(m_stStack, PropertyAccessor<P>::SetterWrapper, 1);
                m_stStack.RawSet(m_stMetaTable);
            }
            return *this;
        }

    private:
        LuaStack& m_stStack;
        LuaStack::AbsIndex m_stMetaTable;
    };
}
