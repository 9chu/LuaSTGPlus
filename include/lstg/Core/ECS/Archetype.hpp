/**
 * @file
 * @author 9chu
 * @date 2022/7/17
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#pragma once
#include <unordered_map>
#include "Chunk.hpp"
#include "Entity.hpp"
#include "../Span.hpp"

namespace lstg::ECS
{
    struct EntityState
    {
        EntitySeq Seq = 0u;
        bool Used = false;
    };

    /**
     * Archetype
     * 用于存储具备相同 Component 的 Entity
     */
    class Archetype
    {
    public:
        Archetype(ArchetypeId id, Span<const ComponentDescriptor*> descriptors);
        Archetype(const Archetype&) = delete;
        Archetype(Archetype&& org) noexcept;

        Archetype& operator=(const Archetype&) = delete;
        Archetype& operator=(Archetype&&) = delete;

    public:
        /**
         * 获取 ID
         */
        [[nodiscard]] ArchetypeId GetId() const noexcept { return m_uId; }

        /**
         * 获取类型 ID
         */
        [[nodiscard]] ArchetypeTypeId GetTypeId() const noexcept { return m_uTypeId; }

        /**
         * 获取 Entity 的容量
         */
        [[nodiscard]] size_t GetEntityCapacity() const noexcept { return m_stEntities.size(); }

        /**
         * 获取 Entity 的数量
         */
        [[nodiscard]] size_t GetUsedEntityCount() const noexcept { return m_uUsedEntity; }

        /**
         * 获取空闲 Entity 的数量
         */
        [[nodiscard]] size_t GetFreeEntityCount() const noexcept { return m_uFreeEntity; }

        /**
         * 分配一个 Entity
         */
        Result<ArchetypeEntityId> Alloc() noexcept;

        /**
         * 释放一个 Entity
         * @param id ID
         */
        void Free(ArchetypeEntityId id) noexcept;

        /**
         * 获取实例状态
         * @param id ID
         */
        EntityState GetEntityState(ArchetypeEntityId id) noexcept;

        /**
         * 获取下一个元素
         * @param id ID
         */
        ArchetypeEntityId NextEntity(ArchetypeEntityId id) noexcept;

        /**
         * 获取上一个元素
         * @param id ID
         */
        ArchetypeEntityId PrevEntity(ArchetypeEntityId id) noexcept;

        /**
         * 获取首个元素
         */
        ArchetypeEntityId FirstEntity() noexcept { return m_uFirstUsedEntity; }

        /**
         * 获取 Component
         * @tparam T 类型
         * @param id ID
         * @return Component
         */
        template <typename T>
        T& GetComponent(ArchetypeEntityId id) noexcept
        {
            auto component = GetComponent(id, GetComponentId(static_cast<T*>(nullptr)));
            return *static_cast<T*>(component);
        }

        template <typename T>
        const T& GetComponent(ArchetypeEntityId id) const noexcept
        {
            return const_cast<Archetype*>(this)->GetComponent<T>(id);
        }

        /**
         * 获取 Component
         * @param id 实例 ID
         * @param componentId 组件 ID
         * @return 组件内存地址
         */
        void* GetComponent(ArchetypeEntityId id, ComponentId componentId) noexcept
        {
            assert(id < m_stEntities.size());
            assert(m_stEntities[id].Used);

            auto& chunk = GetChunk(componentId);
            return chunk.GetComponentRaw(id);
        }

        const void* GetComponent(ArchetypeEntityId id, ComponentId componentId) const noexcept
        {
            return const_cast<Archetype*>(this)->GetComponent(id, componentId);
        }

        /**
         * 获取 Chunk
         * @param componentId 组件ID
         */
        Chunk& GetChunk(ComponentId componentId) noexcept
        {
            assert((m_uTypeId & (1 << componentId)));
            auto it = m_stChunks.find(componentId);
            assert(it != m_stChunks.end());
            return it->second;
        }

        const Chunk& GetChunk(ComponentId componentId) const noexcept
        {
            return const_cast<Archetype*>(this)->GetChunk(componentId);
        }

    private:
        struct EntityInfo : public EntityState
        {
            ArchetypeEntityId Prev = kInvalidArchetypeEntityId;
            ArchetypeEntityId Next = kInvalidArchetypeEntityId;
        };

        ArchetypeId m_uId = 0u;
        ArchetypeTypeId m_uTypeId = 0u;
        std::unordered_map<ComponentId, Chunk> m_stChunks;  // 存储 Component[]
        std::vector<EntityInfo> m_stEntities;  // 存储所有 Entity
        ArchetypeEntityId m_uFirstUsedEntity = kInvalidArchetypeEntityId;  // 首个使用中的 Entity
        ArchetypeEntityId m_uLastUsedEntity = kInvalidArchetypeEntityId;  // 最后一个使用中的 Entity
        ArchetypeEntityId m_uFirstFreeEntity = kInvalidArchetypeEntityId;  // 首个空闲的 Entity
        size_t m_uUsedEntity = 0u;
        size_t m_uFreeEntity = 0u;
    };
}
