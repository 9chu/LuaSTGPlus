/**
 * @file
 * @date 2022/3/15
 * @author 9chu
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#include "TextureVariableBuilder.hpp"

#include <lstg/Core/Subsystem/Render/EffectFactory.hpp>
#include "BuilderModule.hpp"

using namespace std;
using namespace lstg;
using namespace lstg::Subsystem::Render::Effect::detail::LuaBridge;

using AbsIndex = Subsystem::Script::LuaStack::AbsIndex;

#define RETURN_SELF \
    return Script::LuaStack::AbsIndex {1u}

TextureVariableBuilder::TextureVariableBuilder(ShaderTextureDefinition::TextureTypes type, const char* name)
    : m_stDefinition(type, name)
{}

AbsIndex TextureVariableBuilder::SetMinFilter(FilterTypes t)
{
    m_stDefinition.GetSamplerDesc().MinFilter = t;
    RETURN_SELF;
}

AbsIndex TextureVariableBuilder::SetMagFilter(FilterTypes t)
{
    m_stDefinition.GetSamplerDesc().MagFilter = t;
    RETURN_SELF;
}

AbsIndex TextureVariableBuilder::SetMipFilter(FilterTypes t)
{
    m_stDefinition.GetSamplerDesc().MipFilter = t;
    RETURN_SELF;
}

AbsIndex TextureVariableBuilder::SetAddressU(TextureAddressModes u)
{
    m_stDefinition.GetSamplerDesc().AddressU = u;
    RETURN_SELF;
}

AbsIndex TextureVariableBuilder::SetAddressV(TextureAddressModes v)
{
    m_stDefinition.GetSamplerDesc().AddressV = v;
    RETURN_SELF;
}

AbsIndex TextureVariableBuilder::SetAddressW(TextureAddressModes w)
{
    m_stDefinition.GetSamplerDesc().AddressW = w;
    RETURN_SELF;
}

AbsIndex TextureVariableBuilder::SetMaxAnisotropy(uint32_t v)
{
    m_stDefinition.GetSamplerDesc().MaxAnisotropy = v;
    RETURN_SELF;
}

AbsIndex TextureVariableBuilder::SetBorderColor(float r, float g, float b, float a)
{
    m_stDefinition.GetSamplerDesc().BorderColor[0] = r;
    m_stDefinition.GetSamplerDesc().BorderColor[1] = g;
    m_stDefinition.GetSamplerDesc().BorderColor[2] = b;
    m_stDefinition.GetSamplerDesc().BorderColor[3] = a;
    RETURN_SELF;
}

TextureVariableWrapper TextureVariableBuilder::Build(Script::LuaStack& stack)
{
    auto state = BuilderModule::GetScriptState(stack);

    // 检查 Symbol 定义情况
    if (state->ContainsSymbol(m_stDefinition.GetName()))
        stack.Error("Symbol \"%s\" already defined in script", m_stDefinition.GetName().c_str());
    if (state->ContainsSymbol(m_stDefinition.GetSuggestedSamplerName()))
        stack.Error("Symbol \"%s\" already defined in script", m_stDefinition.GetSuggestedSamplerName().c_str());

    // 定义 Symbol
    state->SymbolCache.emplace(m_stDefinition.GetName());
    state->SymbolCache.emplace(m_stDefinition.GetSuggestedSamplerName());

    return { make_shared<const ShaderTextureDefinition>(m_stDefinition) };
}
