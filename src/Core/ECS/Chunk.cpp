/**
 * @file
 * @author 9chu
 * @date 2022/7/17
 * 此文件为 LuaSTGPlus 项目的一部分，版权与许可声明详见 COPYRIGHT.txt。
 */
#include <lstg/Core/ECS/Chunk.hpp>

#include <new>

using namespace std;
using namespace lstg;
using namespace lstg::ECS;

static const size_t kChunkMemoryExpandSize = 16 * 1024;  // 16K

namespace
{
    void* AlignedAlloc(size_t size, size_t alignment) noexcept
    {
#ifdef _MSC_VER
        // M$VC 没有 aligned_alloc
        return _aligned_malloc(size, alignment);
#else
        return aligned_alloc(alignment, size);
#endif
    }

    void AlignedFree(void* p) noexcept
    {
#ifdef _MSC_VER
        return _aligned_free(p);
#else
        return free(p);
#endif
    }
}

Chunk::Chunk(const ComponentDescriptor& descriptor)
    : m_pDescriptor(&descriptor)
{
    assert(kChunkMemoryExpandSize >= descriptor.SizeOfComponent);
}

Chunk::Chunk(Chunk&& rhs) noexcept
    : m_pDescriptor(rhs.m_pDescriptor), m_uComponentCapacity(rhs.m_uComponentCapacity), m_uMemorySize(rhs.m_uMemorySize),
    m_pMemory(rhs.m_pMemory)
{
    rhs.m_uComponentCapacity = rhs.m_uMemorySize = 0u;
    rhs.m_pMemory = nullptr;
}

Chunk::~Chunk()
{
    FreeMemory();
}

Chunk& Chunk::operator=(Chunk&& rhs) noexcept
{
    if (&rhs == this)
        return *this;

    FreeMemory();

    m_pDescriptor = rhs.m_pDescriptor;
    m_uComponentCapacity = rhs.m_uComponentCapacity;
    m_uMemorySize = rhs.m_uMemorySize;
    m_pMemory = rhs.m_pMemory;

    rhs.m_uComponentCapacity = rhs.m_uMemorySize = 0;
    rhs.m_pMemory = nullptr;
    return *this;
}

Result<void> Chunk::Expand() noexcept
{
    // 分配内存块，每次增长 kChunkMemoryExpandSize
    auto size = m_uMemorySize + kChunkMemoryExpandSize;
    auto capacity = size / m_pDescriptor->SizeOfComponent;
    auto m = reinterpret_cast<uint8_t*>(AlignedAlloc(size, m_pDescriptor->AlignOfComponent));
    if (!m)
        return make_error_code(errc::not_enough_memory);

    // 进行移动构造和释放
    for (size_t i = 0; i < m_uComponentCapacity; ++i)
    {
        auto dest = m + i * m_pDescriptor->SizeOfComponent;
        auto src = m_pMemory + i * m_pDescriptor->SizeOfComponent;
        m_pDescriptor->MoveConstructor(dest, src);
        m_pDescriptor->Destructor(src);
    }

    // 进行构造
    for (size_t i = m_uComponentCapacity; i < capacity; ++i)
    {
        auto dest = m + i * m_pDescriptor->SizeOfComponent;
        m_pDescriptor->Constructor(dest);
    }

    // 释放原始数据
    if (m_pMemory)
        ::AlignedFree(m_pMemory);

    m_uComponentCapacity = capacity;
    m_uMemorySize = size;
    m_pMemory = m;
    return {};
}

void Chunk::ResetComponent(ArchetypeEntityId index) noexcept
{
    auto p = m_pMemory + index * m_pDescriptor->SizeOfComponent;
    assert(p < m_pMemory + m_uMemorySize);
    m_pDescriptor->Reset(p);
}

void Chunk::FreeMemory() noexcept
{
    if (m_pMemory)
    {
        // 调用析构
        for (size_t i = 0; i < m_uComponentCapacity; ++i)
        {
            auto dest = m_pMemory + i * m_pDescriptor->SizeOfComponent;
            m_pDescriptor->Destructor(dest);
        }

        // 释放内存
        AlignedFree(m_pMemory);

        m_uComponentCapacity = m_uMemorySize = 0;
        m_pMemory = nullptr;
    }
}
