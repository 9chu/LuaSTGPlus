/**
* @file
* @author 9chu
* @date 2022/7/17
* 此文件为 LuaSTGPlus 项目的一部分，版权与许可声明详见 COPYRIGHT.txt。
*/
#pragma once
#include <cassert>
#include <cstdint>
#include <utility>
#include "../Span.hpp"
#include "Entity.hpp"

namespace lstg::ECS
{
    namespace detail
    {
        template <typename T>
        struct ComponentHelper
        {
        public:
            static void DefaultConstruct(void* p) noexcept
            {
                new (reinterpret_cast<T*>(p)) T();
            }

            static void DefaultMoveConstructor(void* p, void* org) noexcept
            {
                auto& o = *reinterpret_cast<T*>(org);
                new (reinterpret_cast<T*>(p)) T(std::move(o));
            }

            static void DefaultDestructor(void* p) noexcept
            {
                reinterpret_cast<T*>(p)->~T();
            }

            static void DefaultReset(void* p) noexcept
            {
                reinterpret_cast<T*>(p)->Reset();
            }
        };
    }

    using TypeErasedComponentMethod = void(*)(void*) noexcept;
    using TypeErasedComponentMethod2 = void(*)(void*, void*) noexcept;

    /**
     * 组件描述
     */
    struct ComponentDescriptor
    {
        template <typename T>
        static const ComponentDescriptor& GetDescriptor() noexcept
        {
            static_assert(sizeof(T) % alignof(T) == 0);

            static ComponentDescriptor kDescriptor {
                GetComponentId(static_cast<T*>(nullptr)),
                sizeof(T),
                alignof(T),
                detail::ComponentHelper<T>::DefaultConstruct,
                detail::ComponentHelper<T>::DefaultMoveConstructor,
                detail::ComponentHelper<T>::DefaultDestructor,
                detail::ComponentHelper<T>::DefaultReset,
            };
            return kDescriptor;
        }

        ComponentId Id = 0;
        size_t SizeOfComponent = 0;
        size_t AlignOfComponent = 0;
        TypeErasedComponentMethod Constructor = nullptr;
        TypeErasedComponentMethod2 MoveConstructor = nullptr;
        TypeErasedComponentMethod Destructor = nullptr;
        TypeErasedComponentMethod Reset = nullptr;
    };

    /**
     * 计算 Archetype Type ID
     * @param components Component 列表
     * @return 类型 ID
     */
    inline ArchetypeTypeId GetArchetypeTypeId(Span<const ComponentDescriptor*> components) noexcept
    {
        ArchetypeTypeId ret = 0;
        for (size_t i = 0; i < components.GetSize(); ++i)
        {
            auto desc = components[i];
            assert(desc);
            assert(desc->Id < 64);
            ret |= (1 << desc->Id);
        }
        return ret;
    }

    template <typename... TComponent>
    constexpr inline ArchetypeTypeId GetArchetypeTypeId() noexcept
    {
        return ((1u << ComponentDescriptor::GetDescriptor<TComponent>().Id) | ...);
    }
}
