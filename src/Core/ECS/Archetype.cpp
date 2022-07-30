/**
 * @file
 * @author 9chu
 * @date 2022/7/19
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#include <lstg/Core/ECS/Archetype.hpp>

#include <optional>

using namespace std;
using namespace lstg;
using namespace lstg::ECS;

Archetype::Archetype(ArchetypeId id, Span<const ComponentDescriptor*> descriptors)
    : m_uId(id), m_uTypeId(ECS::GetArchetypeTypeId(descriptors))
{
    assert(!descriptors.IsEmpty());

    // 初始化 Chunk
    // 由于每个 Chunk 初始化时大小相同，可以容纳的 Component 数量不同，这里需要取最小值作为初始可分配数量
    optional<size_t> entityCount;
    for (auto desc : descriptors)
    {
        Chunk chunk(*desc);
        chunk.Expand().ThrowIfError();  // 分配一块内存
        entityCount = entityCount ? std::min(*entityCount, chunk.GetCapacity()) : chunk.GetCapacity();
        m_stChunks.emplace(desc->Id, std::move(chunk));
    }
    assert(entityCount);

    // 初始化 Entity 列表
    m_stEntities.resize(*entityCount);
    for (size_t i = 0; i < m_stEntities.size(); ++i)
    {
        auto& entity = m_stEntities[i];
        entity.Prev = i == 0 ? kInvalidArchetypeEntityId : i - 1;
        entity.Next = (i + 1 >= m_stEntities.size()) ? kInvalidArchetypeEntityId : i + 1;
        entity.Used = false;
        entity.Seq = 0;
    }
    m_uFirstFreeEntity = m_stEntities.empty() ? kInvalidArchetypeEntityId : 0;
    m_uFreeEntity = m_stEntities.size();
}

Archetype::Archetype(Archetype&& org) noexcept
    : m_uId(org.m_uId), m_uTypeId(org.m_uTypeId), m_stChunks(std::move(org.m_stChunks)), m_stEntities(std::move(org.m_stEntities)),
    m_uFirstUsedEntity(org.m_uFirstUsedEntity), m_uLastUsedEntity(org.m_uLastUsedEntity), m_uFirstFreeEntity(org.m_uFirstFreeEntity),
    m_uUsedEntity(org.m_uUsedEntity), m_uFreeEntity(org.m_uFreeEntity)
{
    org.m_uFirstUsedEntity = kInvalidArchetypeEntityId;
    org.m_uLastUsedEntity = kInvalidArchetypeEntityId;
    org.m_uFirstFreeEntity = kInvalidArchetypeEntityId;
    org.m_uUsedEntity = 0;
    org.m_uFreeEntity = 0;
}

Result<ArchetypeEntityId> Archetype::Alloc() noexcept
{
    if (m_uFirstFreeEntity == kInvalidArchetypeEntityId)
    {
        // 此时需要申请空间
        optional<size_t> entityCount;
        for (auto& pair : m_stChunks)
        {
            if (pair.second.GetCapacity() <= m_stEntities.size())  // 只在短板的 Component 分配内存
            {
                assert(pair.second.GetCapacity() == m_stEntities.size());
                auto ret = pair.second.Expand();
                if (!ret)  // 内存分配失败
                    return ret.GetError();
            }
            entityCount = entityCount ? std::min(*entityCount, pair.second.GetCapacity()) : pair.second.GetCapacity();
        }
        assert(entityCount);
        assert(*entityCount > m_stEntities.size());  // 一定可以分配出内存

        try
        {
            auto oldSize = m_stEntities.size();
            m_stEntities.resize(*entityCount);

            // 初始化新分配的节点
            for (auto i = oldSize; i < *entityCount; ++i)
            {
                // 塞到 FreeList 中
                auto& ent = m_stEntities[i];
                ent.Prev = kInvalidArchetypeEntityId;
                ent.Next = m_uFirstFreeEntity;
                ent.Used = false;
                ent.Seq = 0;
                if (m_uFirstFreeEntity != kInvalidArchetypeEntityId)
                {
                    auto& freeEnt = m_stEntities[m_uFirstFreeEntity];
                    assert(freeEnt.Prev == kInvalidArchetypeEntityId);
                    freeEnt.Prev = i;
                }
                m_uFirstFreeEntity = i;
                ++m_uFreeEntity;
            }
        }
        catch (...)  // bad_alloc
        {
            return make_error_code(errc::not_enough_memory);
        }
    }

    // 从 FreeList 分配
    assert(m_uFirstFreeEntity != kInvalidArchetypeEntityId);
    auto id = m_uFirstFreeEntity;
    auto& ent = m_stEntities[id];
    assert(ent.Prev == kInvalidArchetypeEntityId);
    if (ent.Next != kInvalidArchetypeEntityId)
        m_stEntities[ent.Next].Prev = kInvalidArchetypeEntityId;
    m_uFirstFreeEntity = ent.Next;
    assert(m_uFreeEntity > 0);
    --m_uFreeEntity;

    // 接入使用链表
    if (m_uLastUsedEntity != kInvalidArchetypeEntityId)
    {
        assert(m_uFirstUsedEntity != kInvalidArchetypeEntityId);
        auto& lastUsed = m_stEntities[m_uLastUsedEntity];
        assert(lastUsed.Next == kInvalidArchetypeEntityId);
        lastUsed.Next = id;
        ent.Prev = m_uLastUsedEntity;
        ent.Next = kInvalidArchetypeEntityId;
        m_uLastUsedEntity = id;
    }
    else
    {
        assert(m_uFirstUsedEntity == kInvalidArchetypeEntityId);
        ent.Prev = ent.Next = kInvalidArchetypeEntityId;
        m_uFirstUsedEntity = m_uLastUsedEntity = id;
    }
    ++m_uUsedEntity;

    // 刷新状态
    ent.Used = true;
    ++ent.Seq;
    return id;
}

void Archetype::Free(ArchetypeEntityId id) noexcept
{
    assert(id < m_stEntities.size());

    auto& ent = m_stEntities[id];
    assert(ent.Used);

    // 从使用中链表断开
    if (ent.Prev != kInvalidArchetypeEntityId)
    {
        auto& prevEnt = m_stEntities[ent.Prev];
        prevEnt.Next = ent.Next;
    }
    if (ent.Next != kInvalidArchetypeEntityId)
    {
        auto& nextEnt = m_stEntities[ent.Next];
        nextEnt.Prev = ent.Prev;
    }
    if (m_uFirstUsedEntity == id)
        m_uFirstUsedEntity = ent.Next;
    if (m_uLastUsedEntity == id)
        m_uLastUsedEntity = ent.Prev;
    assert(m_uUsedEntity > 0);
    --m_uUsedEntity;

    // 放入空闲链表
    if (m_uFirstFreeEntity != kInvalidArchetypeEntityId)
    {
        auto& freeEnt = m_stEntities[m_uFirstFreeEntity];
        freeEnt.Prev = id;
    }
    ent.Prev = kInvalidArchetypeEntityId;
    ent.Next = m_uFirstFreeEntity;
    m_uFirstFreeEntity = id;
    ++m_uFreeEntity;

    // 初始化 Components
    // 通过 Reset 方法回收资源
    for (auto& pair : m_stChunks)
        pair.second.ResetComponent(id);

    // 刷新状态
    ent.Used = false;
}

EntityState Archetype::GetEntityState(ArchetypeEntityId id) noexcept
{
    assert(id < m_stEntities.size());
    auto& state = m_stEntities[id];
    return state;
}

ArchetypeEntityId Archetype::NextEntity(ArchetypeEntityId id) noexcept
{
    assert(id < m_stEntities.size());
    auto& state = m_stEntities[id];
    return state.Next;
}

ArchetypeEntityId Archetype::PrevEntity(ArchetypeEntityId id) noexcept
{
    assert(id < m_stEntities.size());
    auto& state = m_stEntities[id];
    return state.Prev;
}
