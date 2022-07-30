/**
* @file
* @author 9chu
* @date 2022/7/22
* 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
*/
#pragma once
#include "Archetype.hpp"

namespace lstg::ECS
{
    /**
     * 世界
     */
    class World
    {
    public:
        /**
         * 获取 Archetype
         * @param id ID
         */
        Archetype& GetArchetype(ArchetypeId id) noexcept;

        const Archetype& GetArchetype(ArchetypeId id) const noexcept
        {
            return const_cast<World*>(this)->GetArchetype(id);
        }

        /**
         * 创建实例
         */
        template <typename... TArgs>
        Result<Entity> CreateEntity() noexcept
        {
            const ComponentDescriptor* descriptors[] = { (&ComponentDescriptor::GetDescriptor<TArgs>())... };
            return CreateEntity(Span<const ComponentDescriptor*>(descriptors, std::extent_v<decltype(descriptors)>));
        }

        /**
         * 访问所有实例
         * @tparam TRet 返回值
         * @tparam TCallback 回调类型
         * @tparam TComponents
         * @param callback
         * @return
         */
        template <typename TComponents, typename TCallback>
        void VisitEntities(TCallback&& callback) noexcept
        {
            VisitEntitiesHelper<TComponents>{}(this, std::forward<TCallback>(callback));
        }

    private:
        Result<Entity> CreateEntity(Span<const ComponentDescriptor*> desc) noexcept;
        Result<ArchetypeId> GetOrRegisterArchetype(Span<const ComponentDescriptor*> desc) noexcept;

        template <typename... TComponents>
        struct ComponentApplyHelper
        {
            template <typename TCallback, std::size_t... Indices>
            void operator()(Entity ent, TCallback& callback, Chunk* (&chunks)[sizeof...(TComponents)], ArchetypeEntityId entId,
                std::index_sequence<Indices...>) noexcept
            {
                callback(ent, chunks[Indices]->template GetComponent<TComponents>(entId)...);
            }
        };

        template <typename TComponents>
        struct VisitEntitiesHelper;

        template <typename... TArgs>
        struct VisitEntitiesHelper<std::tuple<TArgs...>>
        {
            template <typename TCallback>
            void operator()(World* self, TCallback callback) noexcept
            {
                self->template VisitEntities<TCallback, TArgs...>(callback);
            }
        };

        template <typename TCallback, typename... TComponents>
        void VisitEntities(TCallback& callback) noexcept
        {
            auto archetypeTypeId = GetArchetypeTypeId<TComponents...>();
            for (auto& archetype : m_stArchetypes)
            {
                // 过滤 Components
                if ((archetype.GetTypeId() & archetypeTypeId) != archetypeTypeId)
                    continue;

                // 获取 Chunks
                Chunk* chunks[sizeof...(TComponents)] = {
                    &archetype.GetChunk(GetComponentId(static_cast<TComponents*>(nullptr)))...
                };

                // 进行迭代
                auto currentEntityId = archetype.FirstEntity();
                auto expectedCurrentEntitySeq = 0;
                if (currentEntityId != kInvalidArchetypeEntityId)
                {
                    auto currentEntityState = archetype.GetEntityState(currentEntityId);
                    assert(currentEntityState.Used);
                    expectedCurrentEntitySeq = currentEntityState.Seq;
                }

                while (currentEntityId != kInvalidArchetypeEntityId)
                {
                    auto currentEntityState = archetype.GetEntityState(currentEntityId);
                    if (currentEntityState.Seq != expectedCurrentEntitySeq)
                        break;  // 此时迭代的链表出现变动，直接 break

                    // 先获取下一个元素
                    auto nextEntityId = archetype.NextEntity(currentEntityId);
                    if (nextEntityId != kInvalidArchetypeEntityId)
                    {
                        auto nextEntityState = archetype.GetEntityState(nextEntityId);
                        assert(nextEntityState.Used);
                        expectedCurrentEntitySeq = nextEntityState.Seq;
                    }

                    // 调用 Visit
                    Entity ent {this, CompositeEntityId(currentEntityState.Seq, archetype.GetId(), currentEntityId)};
                    ComponentApplyHelper<TComponents...>{}(ent, callback, chunks, currentEntityId,
                        std::make_index_sequence<sizeof...(TComponents)> {});

                    // 访问下一个元素
                    currentEntityId = nextEntityId;
                }
            }
        }

    private:
        std::vector<Archetype> m_stArchetypes;
        std::unordered_map<ArchetypeTypeId, ArchetypeId> m_stArchetypeTypes;  // TypeID -> ID 查找表
    };
}
