/**
 * @file
 * @author 9chu
 * @date 2022/7/22
 * 此文件为 LuaSTGPlus 项目的一部分，版权与许可声明详见 COPYRIGHT.txt。
 */
#include <lstg/Core/ECS/Entity.hpp>

#include <lstg/Core/ECS/World.hpp>

using namespace std;
using namespace lstg;
using namespace lstg::ECS;

Entity::Entity(World* world, EntityId id) noexcept
    : m_pWorld(world), m_uId(id)
{
}

Entity::operator bool() const noexcept
{
    if (!m_pWorld)
        return false;
    if (m_uId == kInvalidEntityId)
        return false;

    auto& archetype = m_pWorld->GetArchetype(GetEntityArchetypeId(m_uId));

    auto archetypeEntityId = GetEntityArchetypeEntityId(m_uId);
    assert(archetypeEntityId != kInvalidArchetypeEntityId);
    auto state = archetype.GetEntityState(archetypeEntityId);

    // 没有在使用
    if (!state.Used)
        return false;

    // 序列号有变化
    auto seq = GetEntitySeq(m_uId);
    if (seq != state.Seq)
        return false;
    return true;
}

void Entity::Destroy() noexcept
{
    if (m_uId == kInvalidEntityId)
        return;
    m_pWorld->GetArchetype(GetEntityArchetypeId(m_uId)).Free(GetEntityArchetypeEntityId(m_uId));
    m_uId = kInvalidEntityId;
}

bool Entity::HasComponent(ComponentId id) const noexcept
{
    auto archetypeId = GetEntityArchetypeId(m_uId);
    auto& archetype = m_pWorld->GetArchetype(archetypeId);
    return (archetype.GetTypeId() & (1u << id)) != 0;
}

void* Entity::GetComponent(ComponentId id) noexcept
{
    assert(m_pWorld && m_uId != kInvalidEntityId);
    auto archetypeId = GetEntityArchetypeId(m_uId);
    auto archetypeEntityId = GetEntityArchetypeEntityId(m_uId);
    auto& archetype = m_pWorld->GetArchetype(archetypeId);
    auto ret = archetype.GetComponent(archetypeEntityId, id);
    return ret;
}

void* Entity::TryGetComponent(ComponentId id) noexcept
{
    if (!m_pWorld || m_uId == kInvalidEntityId)
        return nullptr;

    auto archetypeId = GetEntityArchetypeId(m_uId);
    auto& archetype = m_pWorld->GetArchetype(archetypeId);
    if ((archetype.GetTypeId() & (1u << id)) == 0)
        return nullptr;
    auto archetypeEntityId = GetEntityArchetypeEntityId(m_uId);
    auto ret = archetype.GetComponent(archetypeEntityId, id);
    return ret;
}
