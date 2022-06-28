/**
 * @file
 * @date 2022/6/26
 * @author 9chu
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#pragma once
#include <list>
#include <unordered_map>

namespace lstg
{
    /**
     * LRU 容器
     * @tparam TKey 键类型
     * @tparam TValue 值类型
     * @tparam Capacity 容量
     */
    template <typename TKey, typename TValue, size_t Capacity>
    class LRUCache
    {
        using NodeContainer = std::list<std::pair<TKey, TValue>>;
        using NodeIterator = typename NodeContainer::iterator;

    public:
        LRUCache() noexcept = default;

        LRUCache(const LRUCache& rhs)
            : m_stNodes(rhs.m_stNodes)
        {
            RebuildCache();
        }

        LRUCache(LRUCache&& rhs) noexcept
            : m_stNodes(std::move(rhs.m_stNodes)), m_stLookupTable(std::move(rhs.m_stLookupTable))
        {
            RefreshCache();
        }

        LRUCache& operator=(const LRUCache& rhs)
        {
            if (this == &rhs)
                return *this;

            m_stLookupTable.clear();
            m_stNodes = rhs.m_stNodes;
            try
            {
                RebuildCache();
            }
            catch (...)
            {
                // 此时状态难以恢复
                m_stNodes.clear();
                m_stLookupTable.clear();
                throw;
            }
            return *this;
        }

        LRUCache& operator=(LRUCache&& rhs) noexcept
        {
            if (this == &rhs)
                return *this;

            m_stNodes = std::move(rhs.m_stNodes);
            m_stLookupTable = std::move(rhs.m_stLookupTable);
            RefreshCache();
            return *this;
        }

        const TValue& operator[](const TKey& key) const noexcept
        {
            auto it = m_stLookupTable.find(key);
            assert(it != m_stLookupTable.end());
            m_stNodes.splice(m_stNodes.begin(), m_stNodes, it->second);
            return it->second->second;
        }

        TValue& operator[](const TKey& key)
        {
            auto it = m_stLookupTable.find(key);

            if (it != m_stLookupTable.end())
                return it->second->second;

            // 创建一个空节点
            m_stNodes.push_front(std::make_pair(key, TValue {}));
            try
            {
                m_stLookupTable.emplace(std::make_pair(key, m_stNodes.begin()));
            }
            catch (...)
            {
                m_stNodes.pop_front();
                throw;
            }
            InvalidateOutdated();

            return m_stNodes.front().second;
        }

    public:
        /**
         * 获取容量
         */
        [[nodiscard]] size_t GetCapacity() const noexcept { return Capacity; }

        /**
         * 添加元素
         * @param key 键
         * @param val 值
         */
        template <typename T, typename P>
        TValue* Emplace(T&& key, P&& val)
        {
            // 检查是否已经存在
            auto it = m_stLookupTable.find(key);
            if (it != m_stLookupTable.end())
            {
                if constexpr (std::is_move_assignable_v<TValue>)
                {
                    it->second->second = std::forward<P>(val);
                    m_stNodes.splice(m_stNodes.begin(), m_stNodes, it->second);
                    return &(it->second->second);
                }

                // 如果不能 move assignable，则删除原有节点
                m_stNodes.erase(it->second);
                m_stLookupTable.erase(it);
            }

            // 创建新节点
            m_stNodes.push_front(std::make_pair(key, std::forward<P>(val)));
            try
            {
                m_stLookupTable.emplace(std::make_pair(std::forward<T>(key), m_stNodes.begin()));
            }
            catch (...)
            {
                // 内存分配失败回退
                m_stNodes.pop_front();
                throw;
            }
            InvalidateOutdated();

            return &(m_stNodes.front().second);
        }

        /**
         * 检查是否存在元素
         * @param key 键
         * @return 是否存在
         */
        bool Contains(const TKey& key) const noexcept
        {
            return m_stLookupTable.find(key) != m_stLookupTable.end();
        }

        /**
         * 尝试获取值
         * @param key 键
         * @return 如果失败则返回 nullptr，否则返回值的指针
         */
        const TValue* TryGet(const TKey& key) const noexcept
        {
            auto it = m_stLookupTable.find(key);
            if (it == m_stLookupTable.end())
                return nullptr;
            m_stNodes.splice(m_stNodes.begin(), m_stNodes, it->second);
            return &(it->second->second);
        }

        TValue* TryGet(const TKey& key) noexcept
        {
            auto value = const_cast<const LRUCache*>(this)->TryGet(key);
            return const_cast<TValue*>(value);
        }

        /**
         * 清空
         */
        void Clear() noexcept
        {
            m_stNodes.clear();
            m_stLookupTable.clear();
        }

    private:
        void RebuildCache()
        {
            m_stLookupTable.clear();
            for (auto it = m_stNodes.begin(); it != m_stNodes.end(); ++it)
                m_stLookupTable.emplace(std::make_pair(it->first, it));
        }

        void RefreshCache() noexcept
        {
            assert(m_stNodes.size() == m_stLookupTable.size());
            for (auto it = m_stNodes.begin(); it != m_stNodes.end(); ++it)
            {
                auto jt = m_stLookupTable.find(it->first);
                assert(jt != m_stLookupTable.end());
                jt->second = it;
            }
        }

        void InvalidateOutdated() noexcept
        {
            while (m_stLookupTable.size() > Capacity)
            {
                auto last = m_stNodes.end();
                last--;
                m_stLookupTable.erase(last->first);
                m_stNodes.pop_back();
            }
        }

    private:
        mutable NodeContainer m_stNodes;
        std::unordered_map<TKey, NodeIterator> m_stLookupTable;
    };
}
