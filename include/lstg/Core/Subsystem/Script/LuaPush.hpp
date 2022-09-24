/**
 * @file
 * @date 2022/3/2
 * @author 9chu
 * 此文件为 LuaSTGPlus 项目的一部分，版权与许可声明详见 COPYRIGHT.txt。
 */
#pragma once
#include <memory>
#include <variant>
#include <optional>
#include "LuaStack.hpp"

namespace lstg::Subsystem::Script
{
    /**
     * 向栈内推入一个 Nil 值
     * @param stack Lua 栈
     * @return 元素个数
     *
     * [-0, +1]
     */
    inline int LuaPush(LuaStack& stack, std::nullptr_t) noexcept
    {
        lua_pushnil(stack);
        return 1;
    }

    /**
     * 向栈内推入一个布尔值
     * @param stack Lua 栈
     * @param v 值
     * @return 元素个数
     *
     * [-0, +1]
     */
    inline int LuaPush(LuaStack& stack, bool v) noexcept
    {
        lua_pushboolean(stack, v);
        return 1;
    }

#define LSTG_DEF_LUA_VM_PUSH_INT(TYPE) \
    inline int LuaPush(LuaStack& stack, TYPE v) noexcept        \
    {                                                           \
        lua_pushinteger(stack, static_cast<lua_Integer>(v));    \
        return 1;                                               \
    }

    /**
     * 向栈上推入一个整数
     * @param stack Lua 栈
     * @param v 值
     * @return 元素个数
     *
     * [-0, +1]
     */
    LSTG_DEF_LUA_VM_PUSH_INT(char)
    LSTG_DEF_LUA_VM_PUSH_INT(wchar_t)
    LSTG_DEF_LUA_VM_PUSH_INT(char16_t)
    LSTG_DEF_LUA_VM_PUSH_INT(char32_t)
    LSTG_DEF_LUA_VM_PUSH_INT(uint8_t)
    LSTG_DEF_LUA_VM_PUSH_INT(int16_t)
    LSTG_DEF_LUA_VM_PUSH_INT(uint16_t)
    LSTG_DEF_LUA_VM_PUSH_INT(int32_t)
    LSTG_DEF_LUA_VM_PUSH_INT(uint32_t)
    LSTG_DEF_LUA_VM_PUSH_INT(int64_t)
    LSTG_DEF_LUA_VM_PUSH_INT(uint64_t)
#undef LSTG_DEF_LUA_VM_PUSH_INT

    /**
     * 向栈上推入一个浮点数
     * @param stack Lua 栈
     * @param v 值
     * @return 元素个数
     *
     * [-0, +1]
     */
    inline int LuaPush(LuaStack& stack, float v) noexcept
    {
        lua_pushnumber(stack, v);
        return 1;
    }

    /**
     * 向栈上推入一个双精度浮点数
     * @param stack Lua 栈
     * @param v 值
     * @return 元素个数
     *
     * [-0, +1]
     */
    inline int LuaPush(LuaStack& stack, double v) noexcept
    {
        lua_pushnumber(stack, v);
        return 1;
    }

    /**
     * 向栈上推入一个 C 函数
     * @param stack Lua 栈
     * @param v 值
     * @return 元素个数
     *
     * [-0, +1]
     */
    inline int LuaPush(LuaStack& stack, lua_CFunction v) noexcept
    {
        if (v == nullptr)
            lua_pushnil(stack);
        else
            lua_pushcfunction(stack, v);
        return 1;
    }

    /**
     * 向栈上推入一个 C 函数闭包
     * @tparam TArgs 闭包元素类型
     * @param stack Lua 栈
     * @param v 值
     * @param args 闭包元素
     * @return 元素个数
     *
     * [-0, +1]
     */
    template <typename... TArgs>
    inline int LuaPush(LuaStack& stack, lua_CFunction v, TArgs&&... args)
    {
        if (v == nullptr)
        {
            lua_pushnil(stack);
            return 1;
        }

        auto cnt = stack.PushValues(std::forward<TArgs>(args)...);
        lua_pushcclosure(stack, v, cnt);
        return 1;
    }

    /**
     * 向栈上推入一个字符串
     * @param stack Lua 栈
     * @param v 值
     * @return 元素个数
     *
     * [-0, +1]
     */
    inline int LuaPush(LuaStack& stack, const char* v)
    {
        lua_pushstring(stack, v);
        return 1;
    }

    /**
     * 向栈上推入一个字符数组
     * @param stack Lua 栈
     * @param v 值（原封不动推入，即使末尾含 \0）
     * @return 元素个数
     *
     * [-0, +1]
     */
    template <size_t Size>
    inline int LuaPush(LuaStack& stack, char const (&v)[Size])
    {
        lua_pushlstring(stack, v, Size);
        return 1;
    }

    /**
     * 向栈上推入一个字符串
     * @param stack Lua 栈
     * @param v 值
     * @return 元素个数
     *
     * [-0, +1]
     */
    inline int LuaPush(LuaStack& stack, const std::string& v)
    {
        lua_pushlstring(stack, v.data(), v.length());
        return 1;
    }
    inline int LuaPush(LuaStack& stack, std::string&& v)
    {
        lua_pushlstring(stack, v.data(), v.length());
        return 1;
    }

    /**
     * 向栈上推入一个字符串视图
     * @param stack Lua 栈
     * @param v 值
     * @return 元素个数
     *
     * [-0, +1]
     */
    inline int LuaPush(LuaStack& stack, std::string_view v)
    {
        lua_pushlstring(stack, v.data(), v.length());
        return 1;
    }

    /**
     * 向栈中推入一个轻量级指针
     * @param stack Lua 栈
     * @param p 指针
     *
     * [-0, +1]
     */
    inline int LuaPush(LuaStack& stack, void* p) noexcept
    {
        lua_pushlightuserdata(stack, p);
        return 1;
    }

    /**
     * 向栈中推入一个枚举
     * @tparam T 类型
     * @param stack 堆栈
     * @param e 枚举
     * @return 元素个数
     *
     * [-0, +1]
     */
    template <typename T>
    inline int LuaPush(typename std::enable_if<std::is_enum_v<T>, LuaStack&>::type stack, T e) noexcept
    {
        lua_pushinteger(stack, static_cast<lua_Integer>(e));
        return 1;
    }

    /**
     * 复制指定索引处的对象
     * @param stack Lua 栈
     * @param v 绝对索引
     * @return 元素个数
     *
     * [-0, +1]
     */
    inline int LuaPush(LuaStack& stack, LuaStack::AbsIndex v) noexcept
    {
        if (v.Index == 0)
            lua_pushnil(stack);
        else
            lua_pushvalue(stack, v);
        return 1;
    }

    /**
     * 推入多个元素
     * @tparam TArgs 元素类型
     * @param stack Lua 栈
     * @param v 元素包
     * @return 元素个数
     *
     * [-0, +n]
     */
    template <typename... TArgs>
    inline int LuaPush(LuaStack& stack, const Unpack<TArgs...>& v)
    {
        return std::apply([&](auto... args) {
            return stack.PushValues(args...);
        }, *static_cast<const std::tuple<TArgs...>*>(&v));
    }
    template <typename... TArgs>
    inline int LuaPush(LuaStack& stack, Unpack<TArgs...>&& v)
    {
        return std::apply([&](auto... args) {
            return stack.PushValues(args...);
        }, std::move(*static_cast<std::tuple<TArgs...>*>(&v)));
    }

    namespace detail
    {
        template <typename T>
        struct IsSpecialClass : public std::false_type {};

        template <typename T>
        struct IsSpecialClass<std::optional<T>> : public std::true_type {};

        template <typename... TArgs>
        struct IsSpecialClass<std::variant<TArgs...>> : public std::true_type {};

        template <typename T>
        struct NativeObject
        {
            T* Object = nullptr;
        };

        template <typename T>
        struct NativeObjectStorage :
            public NativeObject<T>
        {
            std::variant<T*, std::shared_ptr<T>, T> Storage;

            NativeObjectStorage(T* p)
                : Storage(p)
            {
                assert(Storage.index() == 0);
                NativeObject<T>::Object = std::get<0>(Storage);
            }

            NativeObjectStorage(std::shared_ptr<T>&& p)
                : Storage(std::move(p))
            {
                assert(Storage.index() == 1);
                NativeObject<T>::Object = std::get<1>(Storage).get();
            }

            template <typename TArgs>
            NativeObjectStorage(std::in_place_t, TArgs&& args)
                : Storage(std::in_place_index<2>, std::forward<TArgs>(args))
            {
                assert(Storage.index() == 2);
                NativeObject<T>::Object = &std::get<2>(Storage);
            }

            NativeObjectStorage(const NativeObjectStorage&) = delete;
            NativeObjectStorage(NativeObjectStorage&&) = delete;

            static int LuaGC(lua_State* L) noexcept  // obj
            {
                auto p = static_cast<NativeObjectStorage<T>*>(luaL_checkudata(L, 1, detail::UniqueTypeName<T>().Name.c_str()));
                if (!p)
                {
                    assert(false);
                    return 0;
                }

                p->~NativeObjectStorage<T>();
                return 0;
            }

            static int LuaIndex(lua_State* L)  // obj, k
            {
                // 获取元表
                lua_settop(L, 2);
                if (!lua_getmetatable(L, 1))  // obj, k, meta[obj]
                    return 0;

                // 尝试直接存取
                assert(lua_gettop(L) == 3);
                lua_pushvalue(L, 2);  // obj, key, meta[obj], key
                lua_rawget(L, -2);  // obj, key, meta[obj], value
                if (!lua_isnil(L, -1))
                    return 1;

                // 尝试访问访问器
                assert(lua_gettop(L) == 4);
                lua_pop(L, 1);  // obj, key, meta[obj]

                lua_pushliteral(L, "__get_");  // obj, key, meta[obj], s
                lua_pushvalue(L, 2);  // obj, key, meta[obj], s, key
                lua_concat(L, 2);  // obj, key, meta[obj], s
                lua_rawget(L, -2);  // obj, key, meta[obj], getter
                if (!lua_isfunction(L, -1))
                    return 0;

                // 执行访问器
                assert(lua_gettop(L) == 4);
                lua_insert(L, 1);  // getter, obj, key, meta[obj]
                lua_pop(L, 1);  // getter, obj, key
                lua_call(L, 2, 1);
                return 1;
            }

            static int LuaNewIndex(lua_State* L)  // obj, k, v
            {
                // 预先准备参数
                lua_settop(L, 3);
                lua_insert(L, 2);  // obj, v, k

                // 获取元表
                assert(lua_gettop(L) == 3);
                if (!lua_getmetatable(L, 1))  // obj, v, k, meta[obj]
                {
                    luaL_error(L, "invalid metatable of object %s", lua_tostring(L, 1));
                    return 0;
                }

                // 准备获取存取器
                assert(lua_gettop(L) == 4);
                lua_pushliteral(L, "__set_");  // obj, v, k, meta[obj], s
                lua_pushvalue(L, 3);  // obj, v, k, meta[obj], s, k
                lua_concat(L, 2);  // obj, v, k, meta[obj], s
                lua_rawget(L, -2);  // obj, v, k, meta[obj], setter
                if (lua_isnil(L, -1))
                {
                    luaL_error(L, "property '%s' cannot be set to object %s", lua_tostring(L, 3), lua_tostring(L, 1));
                    return 0;
                }

                // call setter
                assert(lua_gettop(L) == 5);
                lua_insert(L, 1);  // setter, obj, value, key, meta
                lua_pop(L, 1);
                lua_call(L, 3, 0);
                return 0;
            }
        };

        template <class, class = void>
        struct HasLuaRegister :
            public std::false_type {};

        template <class T>
        struct HasLuaRegister<T, std::void_t<decltype(LuaRegister(std::declval<LuaClassRegister<T>&>()))>> :
            public std::true_type {};

        template <int RegisterMethod, typename T>
        struct NativeObjectRegisterImpl;

        // 普通注册器：只注册 __gc
        template <typename T>
        struct NativeObjectRegisterImpl<0, T>
        {
            static void Register(LuaStack& st)
            {
                // 栈顶一定是 metatable
                assert(lua_istable(st, -1));

                // 注册 GC 方法
                lua_pushliteral(st, "__gc");
                lua_pushcfunction(st, NativeObjectStorage<T>::LuaGC);
                lua_rawset(st, -3);
            }
        };

        // 高级注册器：调用 LuaRegister(LuaClassRegister<T>)
        template <typename T>
        struct NativeObjectRegisterImpl<1, T>
        {
            static void Register(LuaStack& st)
            {
                // 注册 GC 方法
                NativeObjectRegisterImpl<0, T>::Register(st);

                // 调用 LuaRegister 注册方法
                LuaClassRegister<T> reg {st, LuaStack::AbsIndex(lua_gettop(st))};
                LuaRegister(reg);

                // 注册 __index 方法
                lua_pushliteral(st, "__index");
                lua_pushcfunction(st, NativeObjectStorage<T>::LuaIndex);
                lua_rawset(st, -3);

                // 注册 __newindex 方法
                lua_pushliteral(st, "__newindex");
                lua_pushcfunction(st, NativeObjectStorage<T>::LuaNewIndex);
                lua_rawset(st, -3);
            }
        };

        template <typename T>
        struct NativeObjectRegister :
            public NativeObjectRegisterImpl<2 - HasLuaRegister<T>::value, T>  // 如果没有 LuaRegister 还是不让自动注册吧
        {};
    }

    /**
     * 向栈中推入一个对象指针
     * @tparam T 类型
     * @param stack 堆栈
     * @param p 对象指针
     * @return 元素个数
     *
     * [-0, +1]
     */
    template <typename T>
    inline int LuaPush(typename std::enable_if<std::is_class_v<detail::Unqualified<T>>, LuaStack&>::type stack, T* p)
    {
        using TClass = detail::Unqualified<T>;
        using TStorage = detail::NativeObjectStorage<TClass>;

        // 创建对象存储
        auto ud = reinterpret_cast<TStorage*>(lua_newuserdata(stack, sizeof(TStorage)));
        if (!ud)
        {
            lua_pushnil(stack);
            return 1;
        }

        // 构造对象
        new (ud) TStorage((TClass*)p);

        // 注册元表
        if (luaL_newmetatable(stack, detail::GetUniqueTypeName<TClass>().Name.c_str()))
            detail::NativeObjectRegister<TClass>::Register(stack);
        lua_setmetatable(stack, -2);
        return 1;
    }

    /**
     * 向栈中推入一个智能指针
     * @tparam T 类型
     * @param stack 堆栈
     * @param p 对象指针
     * @return 元素个数
     *
     * [-0, +1]
     */
    template <typename T>
    inline int LuaPush(typename std::enable_if<std::is_class_v<detail::Unqualified<T>>, LuaStack&>::type stack, std::shared_ptr<T> p)
    {
        using TClass = detail::Unqualified<T>;
        using TStorage = detail::NativeObjectStorage<TClass>;

        // 创建对象存储
        auto ud = reinterpret_cast<TStorage*>(lua_newuserdata(stack, sizeof(TStorage)));
        if (!ud)
        {
            lua_pushnil(stack);
            return 1;
        }

        // 构造对象
        new (ud) TStorage(std::static_pointer_cast<TClass>(std::move(p)));

        // 注册元表
        if (luaL_newmetatable(stack, detail::GetUniqueTypeName<TClass>().Name.c_str()))
            detail::NativeObjectRegister<TClass>::Register(stack);
        lua_setmetatable(stack, -2);
        return 1;
    }

    /**
     * 向栈中推入一个值对象
     * @tparam T 类型
     * @param stack 堆栈
     * @param p 对象
     * @return 元素个数
     *
     * [-0, +1]
     */
    template <typename T>
    inline int LuaPush(typename std::enable_if<std::is_class_v<detail::Unqualified<T>> &&
        !detail::IsSpecialClass<detail::Unqualified<T>>::value, LuaStack&>::type stack, T&& p)
    {
        using TClass = detail::Unqualified<T>;
        using TStorage = detail::NativeObjectStorage<TClass>;

        // 创建对象存储
        auto ud = reinterpret_cast<TStorage*>(lua_newuserdata(stack, sizeof(TStorage)));
        if (!ud)
        {
            lua_pushnil(stack);
            return 1;
        }

        // 构造对象
        new (ud) TStorage(std::in_place_t{}, std::forward<T>(p));

        // 注册元表
        if (luaL_newmetatable(stack, detail::GetUniqueTypeName<TClass>().Name.c_str()))
            detail::NativeObjectRegister<TClass>::Register(stack);
        lua_setmetatable(stack, -2);
        return 1;
    }

    namespace detail
    {
        template <typename StackIndices, typename TRet, typename... TArgs>
        struct FunctionCallHelper;

        template <int... Indices, typename TRet, typename... TArgs>
        struct FunctionCallHelper<StackIndexSequence<Indices...>, TRet, TArgs...>
        {
            static_assert(sizeof...(Indices) == sizeof...(TArgs));

            static int Wrapper(lua_State* L)
            {
                auto func = reinterpret_cast<TRet(*)(TArgs...)>(lua_touserdata(L, lua_upvalueindex(1)));
                assert(func);

                LuaStack st(L);
                if constexpr (std::is_void_v<TRet>)
                {
                    try
                    {
                        func(st.ReadValue<TArgs>(Indices)...);
                        return 0;
                    }
                    catch (const std::exception& ex)
                    {
                        st.Error("%s", ex.what());
                    }
                }
                else
                {
                    try
                    {
                        return st.PushValue(func(st.ReadValue<TArgs>(Indices)...));
                    }
                    catch (const std::exception& ex)
                    {
                        st.Error("%s", ex.what());
                    }
                }
            }
        };
    }

    /**
     * 推入一个自由函数
     * @tparam TRet 返回值类型
     * @tparam TArgs 参数类型
     * @param stack Lua 栈
     * @param v 函数指针
     * @return 推入元素个数
     *
     * [-0, +1]
     */
    template <typename TRet, typename... TArgs>
    inline int LuaPush(LuaStack& stack, TRet(*v)(TArgs...))
    {
        using StackIndices = typename detail::MakeStackIndexSequence<1, TArgs...>::Sequence;

        if (!v)
        {
            stack.PushValue(nullptr);
            return 1;
        }

        return LuaPush(stack, detail::FunctionCallHelper<StackIndices, TRet, TArgs...>::Wrapper, reinterpret_cast<void*>(v));
    }

    namespace detail
    {
        template <int Qualifier, typename TClass, typename TRet, typename... TArgs>
        struct MemberFunctionStorage;

        template <typename TClass, typename TRet, typename... TArgs>
        struct MemberFunctionStorage<0, TClass, TRet, TArgs...>
        {
            TRet (TClass::*Pointer)(TArgs...);
        };

        template <typename TClass, typename TRet, typename... TArgs>
        struct MemberFunctionStorage<1, TClass, TRet, TArgs...>
        {
            TRet (TClass::*Pointer)(TArgs...) const;
        };

        template <typename TClass, typename TRet, typename... TArgs>
        struct MemberFunctionStorage<2, TClass, TRet, TArgs...>
        {
            TRet (TClass::*Pointer)(TArgs...) volatile;
        };

        template <typename TClass, typename TRet, typename... TArgs>
        struct MemberFunctionStorage<3, TClass, TRet, TArgs...>
        {
            TRet (TClass::*Pointer)(TArgs...) const volatile;
        };

        // 用于生成释放方法
        template <int Qualifier, typename TClass, typename TRet, typename... TArgs>
        void LuaRegister(LuaClassRegister<MemberFunctionStorage<Qualifier, TClass, TRet, TArgs...>>&) {}

        template <typename StackIndices, int Qualifier, typename TClass, typename TRet, typename... TArgs>
        struct MemberFunctionCallHelper;

        template <int... Indices, int Qualifier, typename TClass, typename TRet, typename... TArgs>
        struct MemberFunctionCallHelper<StackIndexSequence<Indices...>, Qualifier, TClass, TRet, TArgs...>
        {
            static_assert(sizeof...(Indices) == sizeof...(TArgs));

            using UqClass = detail::Unqualified<TClass>;
            using Storage = MemberFunctionStorage<Qualifier, UqClass, TRet, TArgs...>;

            static int Wrapper(lua_State* L)
            {
                // 获得成员函数指针
                auto storage = static_cast<Storage*>(lua_touserdata(L, lua_upvalueindex(1)));
                assert(storage);

                // 获取对象指针
                auto p = static_cast<detail::NativeObjectStorage<UqClass>*>(
                    luaL_checkudata(L, 1, detail::GetUniqueTypeName<UqClass>().Name.c_str()));
                if (!p)
                {
                    assert(false);
                    return 0;
                }

                LuaStack st(L);
                if constexpr (std::is_void_v<TRet>)
                {
                    try
                    {
                        (((TClass*)(p->Object))->*(storage->Pointer))(st.ReadValue<TArgs>(Indices + 1)...);
                        return 0;
                    }
                    catch (const std::exception& ex)
                    {
                        st.Error("%s", ex.what());
                    }
                }
                else
                {
                    try
                    {
                        return st.PushValue((((TClass*)(p->Object))->*(storage->Pointer))(st.ReadValue<TArgs>(Indices + 1)...));
                    }
                    catch(const std::exception& ex)
                    {
                        st.Error("%s", ex.what());
                    }                    
                }
            }
        };
    }

    /**
     * 推入一个成员函数指针
     * @tparam TClass 类
     * @tparam TRet 返回值
     * @tparam TArgs 参数类型
     * @param stack 堆栈对象
     * @param v 值
     * @return 元素个数
     *
     * [-0, +1]
     */
    template <typename TClass, typename TRet, typename... TArgs>
    inline int LuaPush(LuaStack& stack, TRet(TClass::*v)(TArgs...))
    {
        using StackIndices = typename detail::MakeStackIndexSequence<1, TArgs...>::Sequence;
        using Wrapper = detail::MemberFunctionCallHelper<StackIndices, 0, TClass, TRet, TArgs...>;
        using TStorage = typename Wrapper::Storage;

        if (!v)
        {
            stack.PushValue(nullptr);
            return 1;
        }

        // 申请成员函数存储
        auto ud = reinterpret_cast<TStorage*>(lua_newuserdata(stack, sizeof(TStorage)));
        if (!ud)
        {
            lua_pushnil(stack);
            return 1;
        }

        // 赋值
        ud->Pointer = v;

        // 创建闭包
        lua_pushcclosure(stack, Wrapper::Wrapper, 1);
        return 1;
    }

    template <typename TClass, typename TRet, typename... TArgs>
    inline int LuaPush(LuaStack& stack, TRet(TClass::*v)(TArgs...)const)
    {
        using StackIndices = typename detail::MakeStackIndexSequence<1, TArgs...>::Sequence;
        using Wrapper = detail::MemberFunctionCallHelper<StackIndices, 1, TClass, TRet, TArgs...>;
        using TStorage = typename Wrapper::Storage;

        if (!v)
        {
            stack.PushValue(nullptr);
            return 1;
        }

        // 申请成员函数存储
        auto ud = reinterpret_cast<TStorage*>(lua_newuserdata(stack, sizeof(TStorage)));
        if (!ud)
        {
            lua_pushnil(stack);
            return 1;
        }

        // 赋值
        ud->Pointer = v;

        // 创建闭包
        lua_pushcclosure(stack, Wrapper::Wrapper, 1);
        return 1;
    }

    template <typename TClass, typename TRet, typename... TArgs>
    inline int LuaPush(LuaStack& stack, TRet(TClass::*v)(TArgs...)volatile)
    {
        using StackIndices = typename detail::MakeStackIndexSequence<2, TArgs...>::Sequence;
        using Wrapper = detail::MemberFunctionCallHelper<StackIndices, 2, TClass, TRet, TArgs...>;
        using TStorage = typename Wrapper::Storage;

        if (!v)
        {
            stack.PushValue(nullptr);
            return 1;
        }

        // 申请成员函数存储
        auto ud = reinterpret_cast<TStorage*>(lua_newuserdata(stack, sizeof(TStorage)));
        if (!ud)
        {
            lua_pushnil(stack);
            return 1;
        }

        // 赋值
        ud->Pointer = v;

        // 创建闭包
        lua_pushcclosure(stack, Wrapper::Wrapper, 1);
        return 1;
    }

    template <typename TClass, typename TRet, typename... TArgs>
    inline int LuaPush(LuaStack& stack, TRet(TClass::*v)(TArgs...)const volatile)
    {
        using StackIndices = typename detail::MakeStackIndexSequence<3, TArgs...>::Sequence;
        using Wrapper = detail::MemberFunctionCallHelper<StackIndices, 3, TClass, TRet, TArgs...>;
        using TStorage = typename Wrapper::Storage;

        if (!v)
        {
            stack.PushValue(nullptr);
            return 1;
        }

        // 申请成员函数存储
        auto ud = reinterpret_cast<TStorage*>(lua_newuserdata(stack, sizeof(TStorage)));
        if (!ud)
        {
            lua_pushnil(stack);
            return 1;
        }

        // 赋值
        ud->Pointer = v;

        // 创建闭包
        lua_pushcclosure(stack, Wrapper::Wrapper, 1);
        return 1;
    }

    /**
     * 推入一个可选值
     * @tparam T 类型
     * @param stack Lua 栈
     * @param v 值
     * @return 推入的元素个数
     */
    template <typename T>
    inline int LuaPush(typename std::enable_if<!std::is_same_v<detail::Unqualified<T>, LuaStack>, LuaStack&>::type stack,
        const std::optional<T>& v)
    {
        if (!v)
        {
            return stack.PushValue(nullptr);
        }
        else
        {
            // 总是保证只推入至多一个元素，否则元素的个数计算会出现问题
            auto cnt = stack.PushValue(*v);
            if (cnt > 1)
                stack.Pop(cnt - 1);
            return 1;
        }
    }

    /**
     * 推入一个变体
     * @tparam TArgs 类型
     * @param stack Lua 栈
     * @param v 值
     * @return 推入的元素个数
     */
    template <typename... TArgs>
    inline int LuaPush(typename std::enable_if<!(std::is_same_v<detail::Unqualified<TArgs>, LuaStack> || ...), LuaStack&>::type stack,
        const std::variant<TArgs...>& v)
    {
        return std::visit([&](auto&& arg) {
            // 总是保证只推入至多一个元素，否则元素的个数计算会出现问题
            auto cnt = stack.PushValue(arg);
            if (cnt > 1)
                stack.Pop(cnt - 1);
            return 1;
        }, v);
    }
}
