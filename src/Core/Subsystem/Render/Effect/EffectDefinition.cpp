/**
 * @file
 * @date 2022/3/14
 * @author 9chu
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#include <lstg/Core/Subsystem/Render/Effect/EffectDefinition.hpp>

using namespace std;
using namespace lstg;
using namespace lstg::Subsystem::Render::Effect;

bool EffectDefinition::operator==(const EffectDefinition& rhs) const noexcept
{
    return m_stPassGroups == rhs.m_stPassGroups;
}

void EffectDefinition::AddGroup(ImmutableEffectPassGroupDefinitionPtr group)
{
    assert(group);
    assert(!ContainsGroup(group->GetName()));
    m_stPassGroups.emplace_back(std::move(group));
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

size_t EffectDefinition::GetHashCode() const noexcept
{
    auto ret = std::hash<string_view>{}("Effect");
    for (const auto& e : m_stPassGroups)
        ret ^= e->GetHashCode();
    return ret;
}
