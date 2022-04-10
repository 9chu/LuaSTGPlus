/**
 * @file
 * @date 2022/3/13
 * @author 9chu
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#include <lstg/Core/Subsystem/Render/GraphDef/ShaderDefinition.hpp>

#include <Shader.h>
#include <lstg/Core/Subsystem/Render/GraphDef/DefinitionError.hpp>

using namespace std;
using namespace lstg;
using namespace lstg::Subsystem::Render::GraphDef;

#define SMART_PTR_ELEMENT_EQUAL(X) \
    (X && rhs.X ? *X == *rhs.X : static_cast<bool>(X) == static_cast<bool>(rhs.X))

ShaderDefinition::ShaderDefinition(ShaderTypes type)
    : m_iType(type)
{
}

ShaderDefinition::ShaderDefinition(const ShaderDefinition& rhs)
    : m_iType(rhs.m_iType), m_stName(rhs.m_stName), m_stSource(rhs.m_stSource), m_stEntry(rhs.m_stEntry),
    m_stCBufferReferences(rhs.m_stCBufferReferences), m_stGlobalCBufferReferences(rhs.m_stGlobalCBufferReferences),
    m_stTextureReferences(rhs.m_stTextureReferences), m_pVertexLayout(rhs.m_pVertexLayout), m_stSymbolLookupMap(rhs.m_stSymbolLookupMap),
    m_pCompiledShader(rhs.m_pCompiledShader)
{
    if (m_pCompiledShader)
        m_pCompiledShader->AddRef();
}

ShaderDefinition::ShaderDefinition(ShaderDefinition&& rhs) noexcept
    : m_iType(rhs.m_iType), m_stName(std::move(rhs.m_stName)), m_stSource(std::move(rhs.m_stSource)), m_stEntry(std::move(rhs.m_stEntry)),
    m_stCBufferReferences(std::move(rhs.m_stCBufferReferences)), m_stGlobalCBufferReferences(std::move(rhs.m_stGlobalCBufferReferences)),
    m_stTextureReferences(std::move(rhs.m_stTextureReferences)), m_pVertexLayout(std::move(rhs.m_pVertexLayout)),
    m_stSymbolLookupMap(std::move(rhs.m_stSymbolLookupMap)), m_pCompiledShader(rhs.m_pCompiledShader)
{
    rhs.m_pCompiledShader = nullptr;
}

ShaderDefinition::~ShaderDefinition()
{
    if (m_pCompiledShader)
    {
        m_pCompiledShader->Release();
        m_pCompiledShader = nullptr;
    }
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
        if (ref.get() == rhs.m_stCBufferReferences[i].get())  // 指针快速比较
            continue;
        if (!ref->operator==(*rhs.m_stCBufferReferences[i]))
            return false;
    }

    // Texture References
    for (size_t i = 0; i < m_stTextureReferences.size(); ++i)
    {
        const auto& ref = m_stTextureReferences[i];
        assert(ref);
        if (ref.get() == rhs.m_stTextureReferences[i].get())  // 指针快速比较
            continue;
        if (!ref->operator==(*rhs.m_stTextureReferences[i]))
            return false;
    }

    // Vertex Layout
    if (m_pVertexLayout.get() != rhs.m_pVertexLayout.get())  // 指针快速比较
    {
        if (!SMART_PTR_ELEMENT_EQUAL(m_pVertexLayout))
            return false;
    }

    assert(m_stSymbolLookupMap == rhs.m_stSymbolLookupMap);
    return true;
}

Result<void> ShaderDefinition::AddConstantBuffer(ImmutableConstantBufferDefinitionPtr cb, bool global) noexcept
{
    if (!cb)
        return make_error_code(errc::invalid_argument);

    if (ContainsSymbol(cb->GetName()))
        return make_error_code(DefinitionError::SymbolAlreadyDefined);
    for (const auto& f : cb->GetFields())
    {
        if (ContainsSymbol(f.Name))
            return make_error_code(DefinitionError::SymbolAlreadyDefined);
    }

    try
    {
        // 插入 Reference 表
        if (global)
            m_stGlobalCBufferReferences.emplace_back(cb);
        else
            m_stCBufferReferences.emplace_back(cb);

        // 插入符号表
        m_stSymbolLookupMap.emplace(cb->GetName(), global ? SymbolTypes::GlobalCBuffer : SymbolTypes::CBuffer);
        for (const auto& p : cb->GetFields())
            m_stSymbolLookupMap.emplace(p.Name, global ? SymbolTypes::GlobalUniform : SymbolTypes::Uniform);
    }
    catch (...)  // bad_alloc
    {
        return make_error_code(errc::not_enough_memory);
    }
    return {};
}

Result<void> ShaderDefinition::AddTexture(ImmutableShaderTextureDefinitionPtr texture) noexcept
{
    if (!texture)
        return make_error_code(errc::invalid_argument);

    if (ContainsSymbol(texture->GetName()))
        return make_error_code(DefinitionError::SymbolAlreadyDefined);
    if (ContainsSymbol(texture->GetSuggestedSamplerName()))
        return make_error_code(DefinitionError::SymbolAlreadyDefined);

    try
    {
        // 添加到表
        m_stTextureReferences.emplace_back(texture);

        // 插入符号表
        m_stSymbolLookupMap.emplace(texture->GetName(), SymbolTypes::Texture);
        m_stSymbolLookupMap.emplace(texture->GetSuggestedSamplerName(), SymbolTypes::Sampler);
    }
    catch (...)  // bad_alloc
    {
        return make_error_code(errc::not_enough_memory);
    }
    return {};
}

Result<void> ShaderDefinition::SetVertexLayout(ImmutableShaderVertexLayoutDefinitionPtr layout) noexcept
{
    if (!layout)
        return make_error_code(errc::invalid_argument);
    if (m_iType != ShaderTypes::VertexShader)
        return make_error_code(errc::invalid_argument);
    m_pVertexLayout = std::move(layout);
    return {};
}

bool ShaderDefinition::ContainsSymbol(std::string_view symbol) const noexcept
{
    return m_stSymbolLookupMap.find(symbol) != m_stSymbolLookupMap.end();
}

ShaderDefinition::SymbolTypes ShaderDefinition::GetSymbolType(std::string_view symbol) const noexcept
{
    auto it = m_stSymbolLookupMap.find(symbol);
    if (it == m_stSymbolLookupMap.end())
        return SymbolTypes::None;
    return it->second;
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
