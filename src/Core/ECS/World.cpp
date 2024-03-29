/**
 * @file
 * @author 9chu
 * @date 2022/7/23
 * 此文件为 LuaSTGPlus 项目的一部分，版权与许可声明详见 COPYRIGHT.txt。
 */
#include <lstg/Core/ECS/World.hpp>

#include <limits>

using namespace std;
using namespace lstg;
using namespace lstg::ECS;

Archetype& World::GetArchetype(ArchetypeId id) noexcept
{
    assert(id < m_stArchetypes.size());
    return m_stArchetypes[id];
}

size_t World::GetAllocatedMemorySize() const noexcept
{
    size_t ret = 0;
    for (const auto& archetype : m_stArchetypes)
        ret += archetype.GetAllocatedMemorySize();
    return ret;
}

size_t World::GetUsedMemorySize() const noexcept
{
    size_t ret = 0;
    for (const auto& archetype : m_stArchetypes)
        ret += archetype.GetUsedMemorySize();
    return ret;
}

size_t World::GetUsedEntityCount() const noexcept
{
    size_t ret = 0;
    for (const auto& archetype : m_stArchetypes)
        ret += archetype.GetUsedEntityCount();
    return ret;
}

size_t World::GetFreeEntityCount() const noexcept
{
    size_t ret = 0;
    for (const auto& archetype : m_stArchetypes)
        ret += archetype.GetFreeEntityCount();
    return ret;
}

Result<Entity> World::CreateEntity(Span<const ComponentDescriptor*> desc) noexcept
{
    assert(!desc.IsEmpty());
    auto archetypeId = GetOrRegisterArchetype(desc);
    if (!archetypeId)
        return archetypeId.GetError();
    auto& archetype = GetArchetype(*archetypeId);
    auto archetypeEntityId = archetype.Alloc();
    if (!archetypeEntityId)
        return archetypeEntityId.GetError();
    auto state = archetype.GetEntityState(*archetypeEntityId);
    assert(state.Used);
    auto id = CompositeEntityId(state.Seq, *archetypeId, *archetypeEntityId);
    return Entity(this, id);
}

Result<ArchetypeId> World::GetOrRegisterArchetype(Span<const ComponentDescriptor*> desc) noexcept
{
    auto typeId = GetArchetypeTypeId(desc);
    auto it = m_stArchetypeTypes.find(typeId);
    if (it != m_stArchetypeTypes.end())
        return it->second;
    assert(m_stArchetypes.size() < std::numeric_limits<uint16_t>::max());
    try
    {
        auto archetypeId = static_cast<ArchetypeId>(m_stArchetypes.size());
        m_stArchetypes.emplace_back(archetypeId, desc);
        m_stArchetypeTypes.emplace(typeId, archetypeId);
        return archetypeId;
    }
    catch (...)  // bad_alloc
    {
        return make_error_code(errc::not_enough_memory);
    }
}
