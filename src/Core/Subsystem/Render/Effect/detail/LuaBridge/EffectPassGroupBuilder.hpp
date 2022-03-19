/**
 * @file
 * @date 2022/3/19
 * @author 9chu
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#pragma once
#include <lstg/Core/Subsystem/Script/LuaState.hpp>
#include <lstg/Core/Subsystem/Script/AutoBridgeHint.hpp>
#include <lstg/Core/Subsystem/Render/Effect/EffectPassGroupDefinition.hpp>
#include "EffectPassBuilder.hpp"

namespace lstg::Subsystem::Render::Effect::detail::LuaBridge
{
    /**
     * EffectPassGroup 包装类
     */
    LSTG_CLASS()
    class EffectPassGroupWrapper
    {
    public:
        EffectPassGroupWrapper(ImmutableEffectPassGroupDefinitionPtr def)
            : m_pDefinition(std::move(def))
        {}

    public:
        [[nodiscard]] const auto& Get() const noexcept { return m_pDefinition; }

    private:
        ImmutableEffectPassGroupDefinitionPtr m_pDefinition;
    };

    /**
     * EffectPassGroup 构造器
     */
    LSTG_CLASS()
    class EffectPassGroupBuilder
    {
    public:
        EffectPassGroupBuilder(std::string_view name);

    public:
        LSTG_METHOD(tag)
        Script::LuaStack::AbsIndex SetTag(std::string_view key, std::string_view value);

        LSTG_METHOD(pass)
        Script::LuaStack::AbsIndex AddPass(Script::LuaStack& stack, EffectPassWrapper* wrapper);

        LSTG_METHOD(build)
        EffectPassGroupWrapper Build(Script::LuaStack& stack);

    private:
        EffectPassGroupDefinition m_stDefinition;
    };
}
