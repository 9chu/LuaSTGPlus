/**
* @file
* @author 9chu
* @date 2022/7/19
* 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
*/
#pragma once
#include <cassert>
#include <cstdint>
#include <tuple>

namespace lstg::ECS
{
    class World;

    /**
     * 实例 ID
     * 0-15 bit: Seq
     * 16-31 bit: Archetype ID
     * 32-63 bit: Index in archetype
     */
    using EntityId = uint64_t;

    /**
     * 无效 Entity 实例 ID
     */
    static constexpr EntityId kInvalidEntityId = static_cast<EntityId>(-1);

    /**
     * 实例序列号
     */
    using EntitySeq = uint16_t;

    /**
     * Archetype ID
     */
    using ArchetypeId = uint16_t;

    /**
     * Archetype 中实例 ID
     */
    using ArchetypeEntityId = uint32_t;

    /**
     * 无效 Archetype 实例 ID
     */
    static constexpr ArchetypeEntityId kInvalidArchetypeEntityId = static_cast<ArchetypeEntityId>(-1);

    /**
     * Archetype 类型 ID
     * 等价于 Component ID 的 Bitmap
     */
    using ArchetypeTypeId = uint64_t;

    /**
     * Component 类型 ID
     * 0 <= N < 64
     */
    using ComponentId = uint8_t;

    /**
     * 获取实例的序列号
     * @param id 实例 ID
     * @return 序列号
     */
    constexpr inline EntitySeq GetEntitySeq(EntityId id) noexcept
    {
        return static_cast<EntitySeq>(id & 0xFFFFu);
    }

    /**
     * 获取实例的 Archetype ID
     * @param id 实例 ID
     * @return 类型 ID
     */
    constexpr inline ArchetypeId GetEntityArchetypeId(EntityId id) noexcept
    {
        return static_cast<ArchetypeId>((id >> 16u) & 0xFFFFu);
    }

    /**
     * 获取 Archetype 中的实例 ID
     * @param id 实例 ID
     * @return Archetype 实例 ID
     */
    constexpr inline ArchetypeEntityId GetEntityArchetypeEntityId(EntityId id) noexcept
    {
        return static_cast<ArchetypeEntityId>((id >> 32u) & 0xFFFFFFFFu);
    }

    /**
     * 解组合实例 ID
     * @param id 实例 ID
     * @return 解组合结果
     */
    inline std::tuple<EntitySeq, ArchetypeId, ArchetypeEntityId> DecompositeEntityId(EntityId id) noexcept
    {
        return {
            GetEntitySeq(id),
            GetEntityArchetypeId(id),
            GetEntityArchetypeEntityId(id),
        };
    }

    /**
     * 组合实例 ID
     * @param seq 序列号
     * @param typeId Archetype ID
     * @param id 实例在 Archetype 中的 ID
     * @return 实例 ID
     */
    inline EntityId CompositeEntityId(EntitySeq seq, ArchetypeId archetypeId, ArchetypeEntityId entityId) noexcept
    {
        EntityId ret = entityId;
        ret <<= 16u;
        ret |= archetypeId;
        ret <<= 16u;
        ret |= seq;
        assert(GetEntitySeq(ret) == seq);
        assert(GetEntityArchetypeId(ret) == archetypeId);
        assert(GetEntityArchetypeEntityId(ret) == entityId);
        return ret;
    }

    /**
     * Entity 操作封装
     */
    class Entity
    {
    public:
        Entity() noexcept = default;
        Entity(World* world, EntityId id) noexcept;
        Entity(const Entity&) noexcept = default;
        Entity(Entity&&) noexcept = default;

        Entity& operator=(const Entity&) noexcept = default;
        Entity& operator=(Entity&&) noexcept = default;

        operator bool() const noexcept;

    public:
        /**
         * 获取关联的世界
         */
        World* GetWorld() noexcept { return m_pWorld; }
        const World* GetWorld() const noexcept { return m_pWorld; }

        /**
         * 获取 ID
         */
        EntityId GetId() const noexcept { return m_uId; }

        /**
         * 是否存在组件
         * @tparam T 组件类型
         */
        template <typename T>
        bool HasComponent() const noexcept
        {
            return HasComponent(GetComponentId(static_cast<T*>(nullptr)));
        }

        /**
         * 获取组件
         * @tparam T 组件类型
         * @return 组件指针
         */
        template <typename T>
        T& GetComponent() noexcept
        {
            return *static_cast<T*>(GetComponent(GetComponentId(static_cast<T*>(nullptr))));
        }

        template <typename T>
        const T& GetComponent() const noexcept
        {
            return *const_cast<T*>(this)->template GetComponent<T>();
        }

        /**
         * 尝试获取组件
         * @tparam T 组件类型
         * @return 组件指针
         */
        template <typename T>
        T* TryGetComponent() noexcept
        {
            return static_cast<T*>(TryGetComponent(GetComponentId(static_cast<T*>(nullptr))));
        }

        template <typename T>
        const T* TryGetComponent() const noexcept
        {
            return const_cast<T*>(this)->template TryGetComponent<T>();
        }

        /**
         * 销毁
         */
        void Destroy() noexcept;

    private:
        bool HasComponent(ComponentId id) const noexcept;
        void* GetComponent(ComponentId id) noexcept;
        void* TryGetComponent(ComponentId id) noexcept;

    private:
        using ComponentCache = std::pair<ComponentId, void*>;

        World* m_pWorld = nullptr;
        EntityId m_uId = kInvalidEntityId;
        mutable ComponentCache m_stLastComponentCache { 0, nullptr };
    };
}
