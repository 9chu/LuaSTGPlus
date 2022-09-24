/**
 * @file
 * @date 2022/4/5
 * @author 9chu
 * 此文件为 LuaSTGPlus 项目的一部分，版权与许可声明详见 COPYRIGHT.txt。
 */
#pragma once
#include <set>
#include <memory>
#include <optional>
#include <lstg/Core/Subsystem/Script/LuaState.hpp>
#include <lstg/Core/Subsystem/Script/AutoBridgeHint.hpp>

namespace lstg::Subsystem::Render
{
    class EffectFactory;
}

namespace lstg::Subsystem::Render::detail::LuaEffectBuilder
{
    /**
     * Script 对象包装类
     */
    template <typename T>
    class ScriptObjectWrapper
    {
    public:
        ScriptObjectWrapper(std::shared_ptr<const T> def)
            : m_pDefinition(std::move(def))
        {}

    public:
        [[nodiscard]] const auto& Get() const noexcept { return m_pDefinition; }

    private:
        std::shared_ptr<const T> m_pDefinition;
    };

    /**
     * 全局状态
     */
    struct BuilderGlobalState
    {
        EffectFactory* Factory = nullptr;
        std::set<std::string, std::less<>> SymbolCache;

        /**
         * 从 Lua 栈获取
         * @param st 栈
         */
        [[nodiscard]] static BuilderGlobalState* FromLuaStack(Script::LuaStack& st)
        {
            lua_getfield(st, LUA_REGISTRYINDEX, "_state");
            assert(lua_type(st, -1) == LUA_TLIGHTUSERDATA);
            auto state = static_cast<BuilderGlobalState*>(lua_touserdata(st, -1));
            assert(state);
            lua_pop(st, 1);
            return state;
        }

        /**
         * 注册到 Lua
         */
        void RegisterToLua(Script::LuaStack& st)
        {
            lua_pushlightuserdata(st, this);
            lua_setfield(st, LUA_REGISTRYINDEX, "_state");
        }

        [[nodiscard]] bool ContainsSymbol(std::string_view name) const noexcept
        {
            return SymbolCache.find(name) != SymbolCache.end();
        }
    };
}

#define LSTG_SCRIPT_RETURN_SELF \
    return Script::LuaStack::AbsIndex {1u}
