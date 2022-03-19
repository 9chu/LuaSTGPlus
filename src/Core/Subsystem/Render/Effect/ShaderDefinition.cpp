/**
 * @file
 * @date 2022/3/13
 * @author 9chu
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#include <lstg/Core/Subsystem/Render/Effect/ShaderDefinition.hpp>

using namespace std;
using namespace lstg;
using namespace lstg::Subsystem::Render::Effect;

#define SMART_PTR_ELEMENT_EQUAL(X) \
    (X && rhs.X ? *X == *rhs.X : static_cast<bool>(X) == static_cast<bool>(rhs.X))

ShaderDefinition::ShaderDefinition(ShaderTypes type)
    : m_iType(type)
{
}

bool ShaderDefinition::operator==(const ShaderDefinition& rhs) const noexcept
{
    // Basic Attributes
    auto ret = (m_iType == rhs.m_iType && m_stName == rhs.m_stName && m_stEntry == rhs.m_stEntry);
    if (!ret)
        return false;
    if (m_stCBufferReferences.size() != rhs.m_stCBufferReferences.size() ||
        m_stTextureReferences.size() != rhs.m_stTextureReferences.size())
    {
        return false;
    }

    // CBuffer References
    for (size_t i = 0; i < m_stCBufferReferences.size(); ++i)
    {
        const auto& ref = m_stCBufferReferences[i];
        assert(ref);
        if (!ref->operator==(*rhs.m_stCBufferReferences[i]))
            return false;
    }

    // Texture References
    for (size_t i = 0; i < m_stTextureReferences.size(); ++i)
    {
        const auto& ref = m_stTextureReferences[i];
        assert(ref);
        if (!ref->operator==(*rhs.m_stTextureReferences[i]))
            return false;
    }

    // Vertex Layout
    if (!SMART_PTR_ELEMENT_EQUAL(m_pVertexLayout))
        return false;

    return true;
}

void ShaderDefinition::AddConstantBufferReference(ImmutableConstantBufferDefinitionPtr cb)
{
    assert(cb);
    assert(!ContainsSymbol(cb->GetName()));
    assert(!std::any_of(cb->GetFields().begin(), cb->GetFields().end(), [this](const auto& f) { return ContainsSymbol(f.Name); }));

    // 添加到表
    m_stCBufferReferences.emplace_back(std::move(cb));
}

void ShaderDefinition::AddTextureReference(ImmutableShaderTextureDefinitionPtr texture)
{
    assert(texture);
    assert(!ContainsSymbol(texture->GetName()));
    assert(!ContainsSymbol(texture->GetSuggestedSamplerName()));

    // 添加到表
    m_stTextureReferences.emplace_back(std::move(texture));
}

void ShaderDefinition::SetVertexLayout(ImmutableShaderVertexLayoutDefinitionPtr layout) noexcept
{
    assert(m_iType == ShaderTypes::VertexShader);
    m_pVertexLayout = std::move(layout);
}

bool ShaderDefinition::ContainsSymbol(std::string_view symbol) const noexcept
{
    // 检查 CBuffer
    for (const auto& cb : m_stCBufferReferences)
    {
        if (cb->GetName() == symbol)
            return true;
        for (const auto& f : cb->GetFields())
        {
            if (f.Name == symbol)
                return true;
        }
    }

    // 检查 Texture Variable
    for (const auto& tex : m_stTextureReferences)
    {
        if (tex->GetName() == symbol)
            return true;
        if (tex->GetSuggestedSamplerName() == symbol)
            return true;
    }

    return false;
}

size_t ShaderDefinition::GetHashCode() const noexcept
{
    auto ret = std::hash<std::string_view>{}("Shader") ^ std::hash<ShaderTypes>{}(m_iType);
    ret ^= std::hash<std::string>{}(m_stName);
    ret ^= std::hash<std::string>{}(m_stSource);
    ret ^= std::hash<std::string>{}(m_stEntry);
    for (const auto& ref : m_stCBufferReferences)
    {
        assert(ref);
        ret ^= ref->GetHashCode();
    }
    for (const auto& ref : m_stTextureReferences)
    {
        assert(ref);
        ret ^= ref->GetHashCode();
    }
    if (m_pVertexLayout)
        ret ^= m_pVertexLayout->GetHashCode();
    return ret;
}
