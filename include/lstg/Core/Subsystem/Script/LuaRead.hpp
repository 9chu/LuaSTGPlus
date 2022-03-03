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
    inline int LuaRead(LuaStack& stack, int idx, Result<nullptr_t>& out) noexcept
    {
        if (lua_isnil(stack, idx))
            out = nullptr_t {};
        else
            out = make_error_code(static_cast<LuaBadArgumentError>(idx));
        return 1;
    }

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

    inline int LuaRead(LuaStack& stack, int idx, Result<float>& out) noexcept
    {
        if (lua_isnumber(stack, idx))
            out = static_cast<float>(lua_tonumber(stack, idx));
        else
            out = make_error_code(static_cast<LuaBadArgumentError>(idx));
        return 1;
    }

    inline int LuaRead(LuaStack& stack, int idx, Result<double>& out) noexcept
    {
        if (lua_isnumber(stack, idx))
            out = lua_tonumber(stack, idx);
        else
            out = make_error_code(static_cast<LuaBadArgumentError>(idx));
        return 1;
    }

    inline int LuaRead(LuaStack& stack, int idx, Result<lua_CFunction>& out)
    {
        if (lua_iscfunction(stack, idx))
            out = lua_tocfunction(stack, idx);
        else
            out = make_error_code(static_cast<LuaBadArgumentError>(idx));
        return 1;
    }

    inline int LuaRead(LuaStack& stack, int idx, Result<std::string>& out)
    {
        size_t len = 0;
        auto str = lua_tolstring(stack, idx, &len);
        out = std::string(str, len);
        return 1;
    }

    inline int LuaRead(LuaStack& stack, int idx, Result<std::string_view>& out)
    {
        size_t len = 0;
        auto str = lua_tolstring(stack, idx, &len);
        out = std::string_view(str, len);
        return 1;
    }

    inline int LuaRead(LuaStack& stack, int idx, Result<void*>& out)
    {
        if (lua_islightuserdata(stack, idx))
            out = lua_touserdata(stack, idx);
        else
            out = make_error_code(static_cast<LuaBadArgumentError>(idx));
        return 1;
    }

    template <typename T>
    inline int LuaRead(typename std::enable_if<std::is_class_v<T>, LuaStack&>::type stack, int idx, Result<T*> out)
    {
        using UqClass = detail::Unqualified<T>;

        auto ud = lua_touserdata(stack, idx);
        if (ud)
        {
            if (lua_getmetatable(stack, idx))
            {
                lua_getfield(stack, LUA_REGISTRYINDEX, detail::GetUniqueTypeName<UqClass>().Name);
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

    template <typename T>
    inline int LuaRead(typename std::enable_if<std::is_class_v<T>, LuaStack&>::type stack, int idx, Result<T> out)
    {
        using UqClass = detail::Unqualified<T>;

        auto ud = lua_touserdata(stack, idx);
        if (ud)
        {
            if (lua_getmetatable(stack, idx))
            {
                lua_getfield(stack, LUA_REGISTRYINDEX, detail::GetUniqueTypeName<UqClass>().Name);
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

    inline int LuaRead(LuaStack& stack, int idx, LuaStack& out)
    {
        static_cast<void>(idx);
        out = stack;
        return 0;
    }

    template <typename T>
    inline int LuaRead(LuaStack& stack, int idx, T& out)
    {
        Result<T> o;
        auto ret = LuaRead(stack, idx, o);
        if (!o)
            out = T{};
        else
            out = std::move(*o);
        return ret;
    }
}
