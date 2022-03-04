/**
 * @file
 * @date 2022/3/2
 * @author 9chu
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#pragma once
#include "LuaStack.hpp"
#include "LuaPush.hpp"

namespace lstg::Subsystem::Script
{
    /**
     * 读取一个 nil
     * @param stack Lua 栈
     * @param idx 被读取值的起始索引
     * @param out 输出结果
     * @return 读取的个数
     */
    inline int LuaRead(LuaStack& stack, int idx, Result<nullptr_t>& out) noexcept
    {
        if (lua_isnil(stack, idx))
            out = nullptr_t {};
        else
            out = make_error_code(static_cast<LuaBadArgumentError>(idx));
        return 1;
    }

    /**
     * 读取一个布尔值
     * @param stack Lua 栈
     * @param idx 被读取值的起始索引
     * @param out 输出结果
     * @return 读取的个数
     */
    inline int LuaRead(LuaStack& stack, int idx, Result<bool>& out) noexcept
    {
        out = static_cast<bool>(lua_toboolean(stack, idx));
        return 1;
    }

#define LSTG_DEF_LUA_VM_READ_INT(TYPE) \
    inline int LuaRead(LuaStack& stack, int idx, Result<TYPE>& out) noexcept \
    {                                                                        \
        if (lua_isnumber(stack, idx))                                        \
            out = static_cast<TYPE>(lua_tonumber(stack, idx));               \
        else                                                                 \
            out = make_error_code(static_cast<LuaBadArgumentError>(idx));    \
        return 1;                                                            \
    }

    LSTG_DEF_LUA_VM_READ_INT(char)
    LSTG_DEF_LUA_VM_READ_INT(wchar_t)
    LSTG_DEF_LUA_VM_READ_INT(char16_t)
    LSTG_DEF_LUA_VM_READ_INT(char32_t)
    LSTG_DEF_LUA_VM_READ_INT(uint8_t)
    LSTG_DEF_LUA_VM_READ_INT(int16_t)
    LSTG_DEF_LUA_VM_READ_INT(uint16_t)
    LSTG_DEF_LUA_VM_READ_INT(int32_t)
    LSTG_DEF_LUA_VM_READ_INT(uint32_t)
    LSTG_DEF_LUA_VM_READ_INT(int64_t)
    LSTG_DEF_LUA_VM_READ_INT(uint64_t)
#undef LSTG_DEF_LUA_VM_READ_INT

    /**
     * 读取一个浮点数
     * @param stack Lua 栈
     * @param idx 被读取值的起始索引
     * @param out 输出结果
     * @return 读取的个数
     */
    inline int LuaRead(LuaStack& stack, int idx, Result<float>& out) noexcept
    {
        if (lua_isnumber(stack, idx))
            out = static_cast<float>(lua_tonumber(stack, idx));
        else
            out = make_error_code(static_cast<LuaBadArgumentError>(idx));
        return 1;
    }

    /**
     * 读取一个双精度浮点数
     * @param stack Lua 栈
     * @param idx 被读取值的起始索引
     * @param out 输出结果
     * @return 读取的个数
     */
    inline int LuaRead(LuaStack& stack, int idx, Result<double>& out) noexcept
    {
        if (lua_isnumber(stack, idx))
            out = lua_tonumber(stack, idx);
        else
            out = make_error_code(static_cast<LuaBadArgumentError>(idx));
        return 1;
    }

    /**
     * 读取一个 C 函数指针
     * @param stack Lua 栈
     * @param idx 被读取值的起始索引
     * @param out 输出结果
     * @return 读取的个数
     */
    inline int LuaRead(LuaStack& stack, int idx, Result<lua_CFunction>& out)
    {
        if (lua_iscfunction(stack, idx))
            out = lua_tocfunction(stack, idx);
        else
            out = make_error_code(static_cast<LuaBadArgumentError>(idx));
        return 1;
    }

    /**
     * 读取一个字符串
     * @param stack Lua 栈
     * @param idx 被读取值的起始索引
     * @param out 输出结果
     * @return 读取的个数
     */
    inline int LuaRead(LuaStack& stack, int idx, Result<std::string>& out)
    {
        size_t len = 0;
        auto str = lua_tolstring(stack, idx, &len);
        out = std::string(str, len);
        return 1;
    }

    /**
     * 读取一个字符串
     * @param stack Lua 栈
     * @param idx 被读取值的起始索引
     * @param out 输出结果
     * @return 读取的个数
     */
    inline int LuaRead(LuaStack& stack, int idx, Result<std::string_view>& out)
    {
        size_t len = 0;
        auto str = lua_tolstring(stack, idx, &len);
        out = std::string_view(str, len);
        return 1;
    }

    /**
     * 读取一个指针
     * @param stack Lua 栈
     * @param idx 被读取值的起始索引
     * @param out 输出结果
     * @return 读取的个数
     */
    inline int LuaRead(LuaStack& stack, int idx, Result<void*>& out)
    {
        if (lua_islightuserdata(stack, idx))
            out = lua_touserdata(stack, idx);
        else
            out = make_error_code(static_cast<LuaBadArgumentError>(idx));
        return 1;
    }

    /**
     * 读取一个类指针
     * @param stack Lua 栈
     * @param idx 被读取值的起始索引
     * @param out 输出结果
     * @return 读取的个数
     */
    template <typename T>
    inline int LuaRead(typename std::enable_if<std::is_class_v<T>, LuaStack&>::type stack, int idx, Result<T*>& out)
    {
        using UqClass = detail::Unqualified<T>;

        auto ud = lua_touserdata(stack, idx);
        if (ud)
        {
            if (lua_getmetatable(stack, idx))
            {
                lua_getfield(stack, LUA_REGISTRYINDEX, detail::GetUniqueTypeName<UqClass>().Name.c_str());
                auto eq = lua_rawequal(stack, -1, -2);
                lua_pop(stack, 2);
                if (eq)
                {
                    auto storage = static_cast<detail::NativeObjectStorage<UqClass>*>(ud);
                    out = storage->Object;
                    return 1;
                }
            }
        }
        out = make_error_code(static_cast<LuaBadArgumentError>(idx));
        return 1;
    }

    /**
     * 读取一个类
     * @param stack Lua 栈
     * @param idx 被读取值的起始索引
     * @param out 输出结果
     * @return 读取的个数
     */
    template <typename T>
    inline int LuaRead(typename std::enable_if<std::is_class_v<T>, LuaStack&>::type stack, int idx, Result<T>& out)
    {
        using UqClass = detail::Unqualified<T>;

        auto ud = lua_touserdata(stack, idx);
        if (ud)
        {
            if (lua_getmetatable(stack, idx))
            {
                lua_getfield(stack, LUA_REGISTRYINDEX, detail::GetUniqueTypeName<UqClass>().Name.c_str());
                auto eq = lua_rawequal(stack, -1, -2);
                lua_pop(stack, 2);
                if (eq)
                {
                    auto storage = static_cast<detail::NativeObjectStorage<UqClass>*>(ud);
                    out = *(storage->Object);
                    return 1;
                }
            }
        }
        out = make_error_code(static_cast<LuaBadArgumentError>(idx));
        return 1;
    }

    /**
     * 获取当前 Lua 栈
     * @param stack Lua 栈
     * @param idx 被读取值的起始索引
     * @param out 输出结果
     * @return 读取的个数
     */
    inline int LuaRead(LuaStack& stack, int idx, LuaStack& out)
    {
        static_cast<void>(idx);
        out = stack;
        return 0;
    }

    /**
     * 读取值
     * 如果读取失败，则赋值为默认值。
     * @param stack Lua 栈
     * @param idx 被读取值的起始索引
     * @param out 输出结果
     * @return 读取的个数
     */
    template <typename T>
    inline int LuaRead(typename std::enable_if<!lstg::detail::IsResult<T>::value, LuaStack&>::type stack, int idx, T& out)
    {
        Result<T> o { make_error_code(std::errc::invalid_argument) };
        auto ret = LuaRead(stack, idx, o);
        if (!o)
            out = T{};
        else
            out = std::move(*o);
        return ret;
    }

    /**
     * 读取一个可选值
     * @tparam T 值类型
     * @param stack Lua 栈
     * @param idx 被读取值的起始索引
     * @param out 输出结果
     * @return 读取的个数
     */
    template <typename T>
    inline int LuaRead(typename std::enable_if<!std::is_same_v<LuaStack, detail::Unqualified<T>> &&
        !detail::IsUnpack<detail::Unqualified<T>>::value, LuaStack&>::type stack, int idx, std::optional<T>& out)
    {
        if (lua_isnil(stack, idx))
        {
            out.reset();
        }
        else
        {
            auto cnt = stack.ReadValue(idx, *out);
            static_cast<void>(cnt);
            assert(cnt == 1);
        }
        return 1;
    }

    namespace detail
    {
        template <size_t Index, typename... TArgs>
        inline bool ReadVariant(LuaStack& stack, int idx, std::variant<TArgs...>& out)
        {
            if constexpr (Index > 0)
            {
                if (ReadVariant<Index - 1, TArgs...>(stack, idx, out))
                    return true;
            }

            Result<detail::Unqualified<decltype(std::get<Index>(out))>> t { make_error_code(std::errc::invalid_argument) };
            auto cnt = stack.ReadValue(idx, t);
            static_cast<void>(cnt);
            assert(cnt == 1);

            if (!t)
                return false;

            out.template emplace<Index>(std::move(*t));
            return true;
       }
    }

    /**
     * 读取一个可选值
     * @tparam TArgs 值类型
     * @param stack Lua 栈
     * @param idx 被读取值的起始索引
     * @param out 输出结果
     * @return 读取的个数
     */
    template <typename... TArgs>
    inline int LuaRead(typename std::enable_if<!(
            (std::is_same_v<LuaStack, detail::Unqualified<TArgs>> || ...) ||  // 排除 LuaStack 和 Unpack，这会导致读取的值不为定值
            (detail::IsUnpack<detail::Unqualified<TArgs>>::value || ...)
        ), LuaStack&>::type stack, int idx, std::variant<TArgs...>& out)
    {
        detail::ReadVariant<sizeof...(TArgs) - 1, TArgs...>(stack, idx, out);
        return 1;
    }
}
