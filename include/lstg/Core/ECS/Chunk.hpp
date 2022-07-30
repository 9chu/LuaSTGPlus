/**
 * @file
 * @author 9chu
 * @date 2022/7/17
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#pragma once
#include <cassert>
#include <cstdlib>
#include <vector>
#include "../Result.hpp"
#include "ComponentDescriptor.hpp"

namespace lstg::ECS
{
    /**
     * 块
     * 块用来存储 Component[]。
     */
    class Chunk
    {
    public:
        Chunk(const ComponentDescriptor& desc);
        Chunk(const Chunk&) = delete;
        Chunk(Chunk&& rhs) noexcept;
        ~Chunk();

        Chunk& operator=(const Chunk& rhs) = delete;
        Chunk& operator=(Chunk&& rhs) noexcept;

    public:
        /**
         * 获取可以存储组件的上限
         */
        [[nodiscard]] size_t GetCapacity() const noexcept { return m_uComponentCapacity; }

        /**
         * 获取内存大小
         */
        [[nodiscard]] size_t GetMemorySize() const noexcept { return m_uMemorySize; }

        /**
         * 扩展空间
         */
        Result<void> Expand() noexcept;

        /**
         * 重置指定索引的数据
         * @param index 索引
         */
        void ResetComponent(ArchetypeEntityId index) noexcept;

        /**
         * 获取 Component
         * @tparam T Component 类型
         * @param index 索引
         * @return Component 对象
         */
        template <typename T>
        T& GetComponent(ArchetypeEntityId index) noexcept
        {
            assert(GetComponentId(static_cast<T*>(nullptr)) == m_pDescriptor->Id);
            return *static_cast<T*>(GetComponentRaw(index));
        }

        template <typename T>
        const T& GetComponent(ArchetypeEntityId index) const noexcept
        {
            return const_cast<Chunk*>(this)->GetComponent<T>(index);
        }

        /**
         * 获取 Component 原始内存
         * @param index 索引
         * @return Component 原始内存
         */
        void* GetComponentRaw(ArchetypeEntityId index) noexcept
        {
            auto p = m_pMemory + index * m_pDescriptor->SizeOfComponent;
            assert(p < m_pMemory + m_uMemorySize);
            return static_cast<void*>(p);
        }

        const void* GetComponentRaw(ArchetypeEntityId index) const noexcept
        {
            return const_cast<Chunk*>(this)->GetComponentRaw(index);
        }

    private:
        void FreeMemory() noexcept;

    private:
        const ComponentDescriptor* m_pDescriptor = nullptr;
        size_t m_uComponentCapacity = 0;
        size_t m_uMemorySize = 0;
        uint8_t* m_pMemory = nullptr;
    };
}
