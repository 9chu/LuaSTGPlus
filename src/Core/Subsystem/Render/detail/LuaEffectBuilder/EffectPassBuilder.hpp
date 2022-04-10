/**
 * @file
 * @date 2022/3/16
 * @author 9chu
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#pragma once
#include "Helper.hpp"
#include <lstg/Core/Subsystem/Render/GraphDef/EffectPassDefinition.hpp>
#include "ShaderBuilder.hpp"

namespace lstg::Subsystem::Render::detail::LuaEffectBuilder
{
    /**
     * EffectPass 包装类
     */
    LSTG_CLASS()
    using EffectPassWrapper = ScriptObjectWrapper<GraphDef::EffectPassDefinition>;

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
        GraphDef::EffectPassDefinition m_stDefinition;
    };
}
