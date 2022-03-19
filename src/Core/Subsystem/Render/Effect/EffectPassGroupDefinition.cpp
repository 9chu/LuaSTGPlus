/**
 * @file
 * @date 2022/3/14
 * @author 9chu
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#include <lstg/Core/Subsystem/Render/Effect/EffectPassGroupDefinition.hpp>

using namespace std;
using namespace lstg;
using namespace lstg::Subsystem::Render::Effect;

bool EffectPassGroupDefinition::operator==(const EffectPassGroupDefinition& rhs) const noexcept
{
    auto ret = (m_stName == rhs.m_stName &&
        m_stTags == rhs.m_stTags);
    if (!ret)
        return false;

    if (m_stPasses.size() != rhs.m_stPasses.size())
        return false;

    for (size_t i = 0; i < m_stPasses.size(); ++i)
    {
        const auto& l = m_stPasses[i];
        assert(l);

        const auto& r = rhs.m_stPasses[i];
        assert(r);

        if (!l->operator==(*r))
            return false;
    }
    return true;
}

std::string_view EffectPassGroupDefinition::GetTag(std::string_view key) const noexcept
{
    auto it = m_stTags.find(key);
    if (it == m_stTags.end())
        return "";
    return it->second;
}

void EffectPassGroupDefinition::SetTag(std::string_view key, std::string_view value)
{
    m_stTags[string{key}] = value;
}

void EffectPassGroupDefinition::AddPass(ImmutableEffectPassDefinitionPtr pass)
{
    assert(pass);
    assert(!ContainsPass(pass->GetName()));
    m_stPasses.emplace_back(std::move(pass));
}

bool EffectPassGroupDefinition::ContainsPass(std::string_view name) const noexcept
{
    for (const auto& pass : m_stPasses)
    {
        if (pass->GetName() == name)
            return true;
    }
    return false;
}

size_t EffectPassGroupDefinition::GetHashCode() const noexcept
{
    auto ret = std::hash<std::string_view>{}("EffectPassGroup");
    ret ^= std::hash<string>{}(m_stName);
    for (const auto& t : m_stTags)
    {
        ret ^= std::hash<string>{}(t.first);
        ret ^= std::hash<string>{}(t.second);
    }
    for (const auto& p : m_stPasses)
        ret ^= p->GetHashCode();
    return ret;
}
