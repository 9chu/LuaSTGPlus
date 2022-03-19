/**
 * @file
 * @date 2022/3/17
 * @author 9chu
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#pragma once
#include <lstg/Core/Subsystem/Script/LuaState.hpp>
#include <lstg/Core/Subsystem/Script/AutoBridgeHint.hpp>
#include <lstg/Core/Subsystem/Render/Effect/EffectDefinition.hpp>
#include "EffectPassGroupBuilder.hpp"

namespace lstg::Subsystem::Render::Effect::detail::LuaBridge
{
    /**
     * Effect 包装类
     */
    LSTG_CLASS()
    class EffectWrapper
    {
    public:
        EffectWrapper(ImmutableEffectDefinitionPtr def)
            : m_pDefinition(std::move(def))
        {}

    public:
        [[nodiscard]] const auto& Get() const noexcept { return m_pDefinition; }

    private:
        ImmutableEffectDefinitionPtr m_pDefinition;
    };

    /**
     * Effect 构造器
     */
    LSTG_CLASS()
    class EffectBuilder
    {
    public:
        EffectBuilder() = default;

    public:
        LSTG_METHOD(passGroup)
        Script::LuaStack::AbsIndex AddPassGroup(Script::LuaStack& stack, EffectPassGroupWrapper* group);

        LSTG_METHOD(build)
        EffectWrapper Build(Script::LuaStack& stack);

    private:
        EffectDefinition m_stDefinition;
    };
}
