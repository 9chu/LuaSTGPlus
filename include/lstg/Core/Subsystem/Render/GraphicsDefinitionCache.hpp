/**
 * @file
 * @date 2022/3/20
 * @author 9chu
 * 此文件为 LuaSTGPlus 项目的一部分，版权与许可声明详见 COPYRIGHT.txt。
 */
#pragma once
#include <cassert>
#include <memory>
#include <unordered_set>

namespace lstg::Subsystem::Render
{
    /**
     * 图形数据定义缓存
     * 将定义进行缓存，使得可以直接通过指针比较对象是否相同。
     */
    template <typename T>
    class GraphicsDefinitionCache
    {
    public:
        using ImmutablePtr = std::shared_ptr<const T>;

    public:
        template <typename P>
        ImmutablePtr CreateDefinition(P&& arg)
        {
            // FIXME: C++17 尚不支持对 unordered_set 的 Heterogeneous lookup （P0919R3 P1690R1）
            // FIXME: 因此此处必须先创建对象进行比较，升级到 C++20 可以减少这里无用的内存分配
            auto ptr = std::make_shared<const T>(std::forward<P>(arg));
            auto it = m_stCache.find(ptr);
            if (it == m_stCache.end())
            {
                m_stCache.emplace(ptr);
                return ptr;
            }

            // 如果已经定义过，则使用已经定义的对象
            assert((*it)->GetHashCode() == ptr->GetHashCode());
            assert(*(*it) == *ptr);
            return *it;
        }

    private:
        struct GetHashCodeHasher
        {
            size_t operator()(const ImmutablePtr& ptr) const noexcept
            {
                return ptr->GetHashCode();
            }

            size_t operator()(const T& e) const noexcept
            {
                return e.GetHashCode();
            }
        };

        struct SmartPtrElementEqualTo
        {
            bool operator()(const ImmutablePtr& lhs, const ImmutablePtr& rhs) const noexcept
            {
                return lhs && rhs ? *lhs == *rhs : static_cast<bool>(lhs) == static_cast<bool>(rhs);
            }

            bool operator()(const ImmutablePtr& lhs, const T& e) const noexcept
            {
                return lhs && *lhs == e;
            }
        };

        std::unordered_set<ImmutablePtr, GetHashCodeHasher, SmartPtrElementEqualTo> m_stCache;
    };
}
