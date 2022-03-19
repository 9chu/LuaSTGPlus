/**
 * @file
 * @date 2022/3/16
 * @author 9chu
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#pragma once
#include <lstg/Core/Subsystem/Script/LuaState.hpp>
#include <lstg/Core/Subsystem/Script/AutoBridgeHint.hpp>
#include <lstg/Core/Subsystem/Render/Effect/EffectDefinition.hpp>
#include "ShaderBuilder.hpp"

namespace lstg::Subsystem::Render::Effect::detail::LuaBridge
{
    /**
     * EffectPass 包装类
     */
    LSTG_CLASS()
    class EffectPassWrapper
    {
    public:
        EffectPassWrapper(ImmutableEffectPassDefinitionPtr def)
            : m_pDefinition(std::move(def))
        {}

    public:
        [[nodiscard]] const auto& Get() const noexcept { return m_pDefinition; }

    private:
        ImmutableEffectPassDefinitionPtr m_pDefinition;
    };

    /**
     * EffectPass 构造器
     */
    LSTG_CLASS()
    class EffectPassBuilder
    {
    public:
        enum class BlendStates
        {
            Enable,
            SourceBlend,
            DestBlend,
            BlendOperation,
            SourceAlphaBlend,
            DestAlphaBlend,
            AlphaBlendOperation,
            WriteMask,
        };

        enum class RasterizerStates
        {
            FillMode,
            CullMode,
        };

        enum class DepthStencilStates
        {
            DepthEnable,
            DepthWriteEnable,
            DepthFunction,
        };

    public:
        EffectPassBuilder(std::string_view name);

    public:
        LSTG_METHOD(blendState)
        Script::LuaStack::AbsIndex SetBlendState(Script::LuaStack& stack, BlendStates state, int value);

        LSTG_METHOD(rasterizerState)
        Script::LuaStack::AbsIndex SetRasterizerState(Script::LuaStack& stack, RasterizerStates state, int value);

        LSTG_METHOD(depthStencilState)
        Script::LuaStack::AbsIndex SetDepthStencilState(Script::LuaStack& stack, DepthStencilStates state, int value);

        LSTG_METHOD(vertexShader)
        Script::LuaStack::AbsIndex SetVertexShader(Script::LuaStack& stack, ShaderWrapper* shader);

        LSTG_METHOD(pixelShader)
        Script::LuaStack::AbsIndex SetPixelShader(Script::LuaStack& stack, ShaderWrapper* shader);

        LSTG_METHOD(build)
        EffectPassWrapper Build(Script::LuaStack& stack);

    private:
        EffectPassDefinition m_stDefinition;
    };
}
