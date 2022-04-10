/**
 * @file
 * @date 2022/3/16
 * @author 9chu
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#include "EffectPassBuilder.hpp"

#include <lstg/Core/Subsystem/Render/EffectFactory.hpp>
#include "BuilderModule.hpp"

using namespace std;
using namespace lstg;
using namespace lstg::Subsystem::Render::detail::LuaEffectBuilder;

using AbsIndex = Subsystem::Script::LuaStack::AbsIndex;

EffectPassBuilder::EffectPassBuilder(std::string_view name)
{
    m_stDefinition.SetName(string{name});
}

AbsIndex EffectPassBuilder::SetBlendState(Script::LuaStack& stack, BlendStates state, int value)
{
    switch (state)
    {
        case BlendStates::Enable:
            m_stDefinition.GetBlendState().Enable = (value == 1);
            break;
        case BlendStates::SourceBlend:
            m_stDefinition.GetBlendState().SourceBlend = static_cast<GraphDef::BlendFactors>(value);
            break;
        case BlendStates::DestBlend:
            m_stDefinition.GetBlendState().DestBlend = static_cast<GraphDef::BlendFactors>(value);
            break;
        case BlendStates::BlendOperation:
            m_stDefinition.GetBlendState().BlendOperation = static_cast<GraphDef::BlendOperations>(value);
            break;
        case BlendStates::SourceAlphaBlend:
            m_stDefinition.GetBlendState().SourceAlphaBlend = static_cast<GraphDef::BlendFactors>(value);
            break;
        case BlendStates::DestAlphaBlend:
            m_stDefinition.GetBlendState().DestAlphaBlend = static_cast<GraphDef::BlendFactors>(value);
            break;
        case BlendStates::AlphaBlendOperation:
            m_stDefinition.GetBlendState().AlphaBlendOperation = static_cast<GraphDef::BlendOperations>(value);
            break;
        case BlendStates::WriteMask:
            m_stDefinition.GetBlendState().WriteMask = static_cast<GraphDef::ColorWriteMask>(value);
            break;
        default:
            stack.Error("invalid state value");
            break;
    }
    LSTG_SCRIPT_RETURN_SELF;
}

AbsIndex EffectPassBuilder::SetRasterizerState(Script::LuaStack& stack, RasterizerStates state, int value)
{
    switch (state)
    {
        case RasterizerStates::FillMode:
            m_stDefinition.GetRasterizerState().FillMode = static_cast<GraphDef::FillModes>(value);
            break;
        case RasterizerStates::CullMode:
            m_stDefinition.GetRasterizerState().CullMode = static_cast<GraphDef::CullModes>(value);
            break;
        default:
            stack.Error("invalid state value");
            break;
    }
    LSTG_SCRIPT_RETURN_SELF;
}

AbsIndex EffectPassBuilder::SetDepthStencilState(Script::LuaStack& stack, DepthStencilStates state, int value)
{
    switch (state)
    {
        case DepthStencilStates::DepthEnable:
            m_stDefinition.GetDepthStencilState().DepthEnable = (value == 1);
            break;
        case DepthStencilStates::DepthWriteEnable:
            m_stDefinition.GetDepthStencilState().DepthWriteEnable = (value == 1);
            break;
        case DepthStencilStates::DepthFunction:
            m_stDefinition.GetDepthStencilState().DepthFunction = static_cast<GraphDef::ComparisionFunctions>(value);
            break;
        default:
            stack.Error("invalid state value");
            break;
    }
    LSTG_SCRIPT_RETURN_SELF;
}

AbsIndex EffectPassBuilder::SetVertexShader(Script::LuaStack& stack, ShaderWrapper* shader)
{
    assert(shader);
    if (shader->Get()->GetType() != GraphDef::ShaderDefinition::ShaderTypes::VertexShader)
        stack.Error("vertex shader expected");
    m_stDefinition.SetVertexShader(shader->Get());
    LSTG_SCRIPT_RETURN_SELF;
}

AbsIndex EffectPassBuilder::SetPixelShader(Script::LuaStack& stack, ShaderWrapper* shader)
{
    assert(shader);
    if (shader->Get()->GetType() != GraphDef::ShaderDefinition::ShaderTypes::PixelShader)
        stack.Error("pixel shader expected");
    m_stDefinition.SetPixelShader(shader->Get());
    LSTG_SCRIPT_RETURN_SELF;
}

EffectPassWrapper EffectPassBuilder::Build(Script::LuaStack& stack)
{
    auto state = BuilderGlobalState::FromLuaStack(stack);
    auto ec = state->Factory->CompilePass(m_stDefinition);
    stack.ThrowIfError(ec);
    return std::move(*ec);
}
