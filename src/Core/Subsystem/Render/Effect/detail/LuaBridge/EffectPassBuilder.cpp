/**
 * @file
 * @date 2022/3/16
 * @author 9chu
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#include "EffectPassBuilder.hpp"

using namespace std;
using namespace lstg;
using namespace lstg::Subsystem::Render::Effect::detail::LuaBridge;

using AbsIndex = Subsystem::Script::LuaStack::AbsIndex;

#define RETURN_SELF \
    return Script::LuaStack::AbsIndex {1u}

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
            m_stDefinition.GetBlendState().SourceBlend = static_cast<Effect::BlendFactors>(value);
            break;
        case BlendStates::DestBlend:
            m_stDefinition.GetBlendState().DestBlend = static_cast<Effect::BlendFactors>(value);
            break;
        case BlendStates::BlendOperation:
            m_stDefinition.GetBlendState().BlendOperation = static_cast<Effect::BlendOperations>(value);
            break;
        case BlendStates::SourceAlphaBlend:
            m_stDefinition.GetBlendState().SourceAlphaBlend = static_cast<Effect::BlendFactors>(value);
            break;
        case BlendStates::DestAlphaBlend:
            m_stDefinition.GetBlendState().DestAlphaBlend = static_cast<Effect::BlendFactors>(value);
            break;
        case BlendStates::AlphaBlendOperation:
            m_stDefinition.GetBlendState().AlphaBlendOperation = static_cast<Effect::BlendOperations>(value);
            break;
        case BlendStates::WriteMask:
            m_stDefinition.GetBlendState().WriteMask = static_cast<Effect::ColorWriteMask>(value);
            break;
        default:
            stack.Error("Invalid state value");
            break;
    }
    RETURN_SELF;
}

AbsIndex EffectPassBuilder::SetRasterizerState(Script::LuaStack& stack, RasterizerStates state, int value)
{
    switch (state)
    {
        case RasterizerStates::FillMode:
            m_stDefinition.GetRasterizerState().FillMode = static_cast<Effect::FillModes>(value);
            break;
        case RasterizerStates::CullMode:
            m_stDefinition.GetRasterizerState().CullMode = static_cast<Effect::CullModes>(value);
            break;
        default:
            stack.Error("Invalid state value");
            break;
    }
    RETURN_SELF;
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
            m_stDefinition.GetDepthStencilState().DepthFunction = static_cast<Effect::ComparisionFunctions>(value);
            break;
        default:
            stack.Error("Invalid state value");
            break;
    }
    RETURN_SELF;
}

AbsIndex EffectPassBuilder::SetVertexShader(Script::LuaStack& stack, ShaderWrapper* shader)
{
    assert(shader);
    if (shader->Get()->GetType() != ShaderDefinition::ShaderTypes::VertexShader)
        stack.Error("Vertex shader expected");
    m_stDefinition.SetVertexShader(shader->Get());
    RETURN_SELF;
}

AbsIndex EffectPassBuilder::SetPixelShader(Script::LuaStack& stack, ShaderWrapper* shader)
{
    assert(shader);
    if (shader->Get()->GetType() != ShaderDefinition::ShaderTypes::PixelShader)
        stack.Error("Pixel shader expected");
    m_stDefinition.SetPixelShader(shader->Get());
    RETURN_SELF;
}

EffectPassWrapper EffectPassBuilder::Build(Script::LuaStack& stack)
{
    if (!m_stDefinition.GetVertexShader())
        stack.Error("Vertex shader is not defined");
    if (!m_stDefinition.GetPixelShader())
        stack.Error("Pixel shader is not defined");
    return { make_shared<const EffectPassDefinition>(m_stDefinition) };
}
