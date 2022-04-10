/**
 * @file
 * @date 2022/3/14
 * @author 9chu
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#include <lstg/Core/Subsystem/Render/GraphDef/EffectDefinition.hpp>

#include <lstg/Core/Subsystem/Render/GraphDef/DefinitionError.hpp>

using namespace std;
using namespace lstg;
using namespace lstg::Subsystem::Render::GraphDef;

bool EffectDefinition::operator==(const EffectDefinition& rhs) const noexcept
{
    if (m_stPassGroups.size() != rhs.m_stPassGroups.size())
        return false;
    for (size_t i = 0; i < m_stPassGroups.size(); ++i)
    {
        const auto& l = m_stPassGroups[i];
        const auto& r = rhs.m_stPassGroups[i];
        if (l.get() == r.get())
            continue;
        if (!l->operator==(*r))
            return false;
    }
    assert(m_stSymbolLookupMap == rhs.m_stSymbolLookupMap);
    return true;
}

Result<void> EffectDefinition::AddGroup(ImmutableEffectPassGroupDefinitionPtr group) noexcept
{
    if (!group)
        return make_error_code(errc::invalid_argument);
    if (ContainsGroup(group->GetName()))
        return make_error_code(DefinitionError::SymbolAlreadyDefined);

    try
    {
        // 加入Group
        m_stPassGroups.emplace_back(std::move(group));

        // 加入符号表
        for (const auto& g : m_stPassGroups)
        {
            for (const auto& p : g->GetPasses())
            {
                const ShaderDefinition* shaders[2] = {
                    p->GetVertexShader().get(), p->GetPixelShader().get()
                };

                // 收集 Uniform 符号
                for (const auto s : shaders)
                {
                    const vector<ImmutableConstantBufferDefinitionPtr>* uniformBuffers[2] = {
                        &s->GetGlobalConstantBuffers(), &s->GetConstantBuffers()
                    };
                    for (const auto ubv : uniformBuffers)
                    {
                        for (const auto& ub : *ubv)
                        {
                            for (const auto& f : ub->GetFields())
                            {
                                if (m_stSymbolLookupMap.find(f.Name) != m_stSymbolLookupMap.end())
                                    continue;

                                SymbolInfo info {};
                                info.Type = (ubv == &s->GetGlobalConstantBuffers()) ?
                                    ShaderDefinition::SymbolTypes::GlobalUniform : ShaderDefinition::SymbolTypes::Uniform;
                                info.AssocInfo = UniformSymbolInfo { ub, &f };
                                m_stSymbolLookupMap.emplace(f.Name, std::move(info));
                            }
                        }
                    }
                }

                // 收集 Texture 符号
                for (const auto s : shaders)
                {
                    for (const auto& t : s->GetTextures())
                    {
                        if (m_stSymbolLookupMap.find(t->GetName()) == m_stSymbolLookupMap.end())
                        {
                            SymbolInfo info {};
                            info.Type = ShaderDefinition::SymbolTypes::Texture;
                            info.AssocInfo = TextureOrSamplerSymbolInfo { t };
                            m_stSymbolLookupMap.emplace(t->GetName(), std::move(info));
                        }

                        if (m_stSymbolLookupMap.find(t->GetSuggestedSamplerName()) == m_stSymbolLookupMap.end())
                        {
                            SymbolInfo info {};
                            info.Type = ShaderDefinition::SymbolTypes::Sampler;
                            info.AssocInfo = TextureOrSamplerSymbolInfo { t };
                            m_stSymbolLookupMap.emplace(t->GetSuggestedSamplerName(), std::move(info));
                        }
                    }
                }
            }
        }
    }
    catch (...)
    {
        return make_error_code(errc::not_enough_memory);
    }
    return {};
}

bool EffectDefinition::ContainsGroup(std::string_view name) const noexcept
{
    for (const auto& g : m_stPassGroups)
    {
        if (g->GetName() == name)
            return true;
    }
    return false;
}

bool EffectDefinition::ContainsSymbol(std::string_view name) const noexcept
{
    return m_stSymbolLookupMap.find(name) != m_stSymbolLookupMap.end();
}

const EffectDefinition::SymbolInfo* EffectDefinition::GetSymbol(std::string_view symbol) const noexcept
{
    auto it = m_stSymbolLookupMap.find(symbol);
    if (it == m_stSymbolLookupMap.end())
        return nullptr;
    return &it->second;
}

size_t EffectDefinition::GetHashCode() const noexcept
{
    auto ret = std::hash<string_view>{}("Effect");
    for (const auto& e : m_stPassGroups)
        ret ^= e->GetHashCode();
    return ret;
}
