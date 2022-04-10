/**
 * @file
 * @date 2022/3/15
 * @author 9chu
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#include "TextureVariableBuilder.hpp"

using namespace std;
using namespace lstg;
using namespace lstg::Subsystem::Render::detail::LuaEffectBuilder;

using AbsIndex = Subsystem::Script::LuaStack::AbsIndex;

TextureVariableBuilder::TextureVariableBuilder(GraphDef::ShaderTextureDefinition::TextureTypes type, const char* name)
    : m_stDefinition(type, name)
{}

AbsIndex TextureVariableBuilder::SetMinFilter(GraphDef::FilterTypes t)
{
    m_stDefinition.GetSamplerDesc().MinFilter = t;
    LSTG_SCRIPT_RETURN_SELF;
}

AbsIndex TextureVariableBuilder::SetMagFilter(GraphDef::FilterTypes t)
{
    m_stDefinition.GetSamplerDesc().MagFilter = t;
    LSTG_SCRIPT_RETURN_SELF;
}

AbsIndex TextureVariableBuilder::SetMipFilter(GraphDef::FilterTypes t)
{
    m_stDefinition.GetSamplerDesc().MipFilter = t;
    LSTG_SCRIPT_RETURN_SELF;
}

AbsIndex TextureVariableBuilder::SetAddressU(GraphDef::TextureAddressModes u)
{
    m_stDefinition.GetSamplerDesc().AddressU = u;
    LSTG_SCRIPT_RETURN_SELF;
}

AbsIndex TextureVariableBuilder::SetAddressV(GraphDef::TextureAddressModes v)
{
    m_stDefinition.GetSamplerDesc().AddressV = v;
    LSTG_SCRIPT_RETURN_SELF;
}

AbsIndex TextureVariableBuilder::SetAddressW(GraphDef::TextureAddressModes w)
{
    m_stDefinition.GetSamplerDesc().AddressW = w;
    LSTG_SCRIPT_RETURN_SELF;
}

AbsIndex TextureVariableBuilder::SetMaxAnisotropy(uint32_t v)
{
    m_stDefinition.GetSamplerDesc().MaxAnisotropy = v;
    LSTG_SCRIPT_RETURN_SELF;
}

AbsIndex TextureVariableBuilder::SetBorderColor(float r, float g, float b, float a)
{
    m_stDefinition.GetSamplerDesc().BorderColor[0] = r;
    m_stDefinition.GetSamplerDesc().BorderColor[1] = g;
    m_stDefinition.GetSamplerDesc().BorderColor[2] = b;
    m_stDefinition.GetSamplerDesc().BorderColor[3] = a;
    LSTG_SCRIPT_RETURN_SELF;
}

TextureVariableWrapper TextureVariableBuilder::Build(Script::LuaStack& stack)
{
    auto state = BuilderGlobalState::FromLuaStack(stack);

    // 检查 Symbol 定义情况
    if (state->ContainsSymbol(m_stDefinition.GetName()))
        stack.Error("Symbol \"%s\" already defined in script", m_stDefinition.GetName().c_str());
    if (state->ContainsSymbol(m_stDefinition.GetSuggestedSamplerName()))
        stack.Error("Symbol \"%s\" already defined in script", m_stDefinition.GetSuggestedSamplerName().c_str());

    // 定义 Symbol
    state->SymbolCache.emplace(m_stDefinition.GetName());
    state->SymbolCache.emplace(m_stDefinition.GetSuggestedSamplerName());

    return { make_shared<const GraphDef::ShaderTextureDefinition>(m_stDefinition) };
}
