/**
 * @file
 * @author 9chu
 * @date 2022/7/23
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#include <lstg/Core/ECS/World.hpp>

using namespace std;
using namespace lstg;
using namespace lstg::ECS;

Archetype& World::GetArchetype(ArchetypeId id) noexcept
{
    assert(id < m_stArchetypes.size());
    return m_stArchetypes[id];
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
