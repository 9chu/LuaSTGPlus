/**
 * @file
 * @author 9chu
 * @date 2022/2/17
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#pragma once
#include <cassert>
#include <algorithm>

namespace lstg
{
    template <class T, class Comparer>
    class IntrusiveHeap;

    /**
     * 侵入式堆节点
     * 通过继承该基类实现自定义数据结构。
     */
    struct IntrusiveHeapNode
    {
        template <class T, class Comparer>
        friend class IntrusiveHeap;

    public:
        IntrusiveHeapNode() = default;

        IntrusiveHeapNode(const IntrusiveHeapNode&) = delete;

        IntrusiveHeapNode(IntrusiveHeapNode&& org) noexcept
            : m_pLeft(org.m_pLeft), m_pRight(org.m_pRight), m_pParent(org.m_pParent)
        {
            org.m_pLeft = nullptr;
            org.m_pRight = nullptr;
            org.m_pParent = nullptr;

            if (m_pParent)
            {
                if (m_pParent->m_pLeft == &org)
                {
                    m_pParent->m_pLeft = this;
                }
                else
                {
                    assert(m_pParent->m_pRight == &org);
                    m_pParent->m_pRight = this;
                }
            }
            if (m_pLeft)
            {
                assert(m_pLeft->m_pParent == &org);
                m_pLeft->m_pParent = this;
            }
            if (m_pRight)
            {
                assert(m_pRight->m_pParent == &org);
                m_pRight->m_pParent = this;
            }
        }

        IntrusiveHeapNode& operator=(const IntrusiveHeapNode&) = delete;
        IntrusiveHeapNode& operator=(IntrusiveHeapNode&& rhs) noexcept = delete;

    private:
        IntrusiveHeapNode* m_pLeft = nullptr;
        IntrusiveHeapNode* m_pRight = nullptr;
        IntrusiveHeapNode* m_pParent = nullptr;
    };

    /**
     * 侵入式堆
     * 需要注意对内存的管理由被调用方进行。
     */
    template <class T, class Comparer>
    class IntrusiveHeap
    {
    public:
        IntrusiveHeap() = default;

        IntrusiveHeap(const IntrusiveHeap&) = delete;

        IntrusiveHeap(IntrusiveHeap&& rhs) noexcept
            : m_pMin(rhs.m_pMin), m_uNelts(rhs.m_uNelts)
        {
            rhs.m_pMin = nullptr;
            rhs.m_uNelts = 0;
        }

        IntrusiveHeap& operator=(const IntrusiveHeap&) = delete;

        IntrusiveHeap& operator=(IntrusiveHeap&& rhs) noexcept
        {
            if (&rhs == this)
                return *this;

            std::swap(m_pMin, rhs.m_pMin);
            std::swap(m_uNelts, rhs.m_uNelts);

            rhs.m_uNelts = nullptr;
            rhs.m_pMin = 0;
            return *this;
        }

    public:
        /**
         * 获取堆顶元素
         */
        T* GetTop() noexcept { return static_cast<T*>(m_pMin); }
        const T* GetTop() const noexcept { return const_cast<const T*>(static_cast<T*>(m_pMin)); }

        /**
         * 插入元素
         * @param newNode 节点
         */
        void Insert(T* newNode) noexcept
        {
            newNode->m_pLeft = nullptr;
            newNode->m_pRight = nullptr;
            newNode->m_pParent = nullptr;

            unsigned int path = 0, k = 0, n = 1 + m_uNelts;
            for (; n >= 2; k += 1, n /= 2)
                path = (path << 1) | (n & 1);

            auto child = &m_pMin;
            auto parent = child;
            while (k > 0)
            {
                parent = child;
                if (path & 1)
                    child = &(*child)->m_pRight;
                else
                    child = &(*child)->m_pLeft;
                path >>= 1;
                k -= 1;
            }

            newNode->m_pParent = *parent;
            *child = newNode;
            m_uNelts += 1;

            while (newNode->m_pParent != nullptr &&
                Comparer()(const_cast<const T*>(newNode), const_cast<const T*>(static_cast<T*>(newNode->m_pParent))))
            {
                NodeSwap(newNode->m_pParent, newNode);
            }
        }

        /**
         * 从堆中删除节点
         * @note Heap 不会主动释放 node 的内存。
         * @param node 节点
         */
        void Remove(T* node) noexcept
        {
            if (m_uNelts == 0)
                return;

            unsigned int path = 0, k = 0, n = m_uNelts;
            for (; n >= 2; k += 1, n /= 2)
                path = (path << 1) | (n & 1);

            auto max = &m_pMin;
            while (k > 0)
            {
                if (path & 1)
                    max = &(*max)->m_pRight;
                else
                    max = &(*max)->m_pLeft;
                path >>= 1;
                k -= 1;
            }

            m_uNelts -= 1;

            auto child = *max;
            *max = nullptr;
            if (child == node)
            {
                if (child == m_pMin)
                    m_pMin = nullptr;
                return;
            }

            child->m_pLeft = node->m_pLeft;
            child->m_pRight = node->m_pRight;
            child->m_pParent = node->m_pParent;

            if (child->m_pLeft != nullptr)
                child->m_pLeft->m_pParent = child;

            if (child->m_pRight != nullptr)
                child->m_pRight->m_pParent = child;

            if (node->m_pParent == nullptr)
                m_pMin = child;
            else if (node->m_pParent->m_pLeft == node)
                node->m_pParent->m_pLeft = child;
            else
                node->m_pParent->m_pRight = child;

            while (true)
            {
                auto smallest = child;
                if (child->m_pLeft != nullptr &&
                    Comparer()(const_cast<const T*>(static_cast<T*>(child->m_pLeft)), const_cast<const T*>(static_cast<T*>(smallest))))
                {
                    smallest = child->m_pLeft;
                }
                if (child->m_pRight != nullptr &&
                    Comparer()(const_cast<const T*>(static_cast<T*>(child->m_pRight)), const_cast<const T*>(static_cast<T*>(smallest))))
                {
                    smallest = child->m_pRight;
                }
                if (smallest == child)
                    break;
                NodeSwap(child, smallest);
            }

            while (child->m_pParent != nullptr &&
                Comparer()(const_cast<const T*>(static_cast<T*>(child)), const_cast<const T*>(static_cast<T*>(child->m_pParent))))
            {
                NodeSwap(child->m_pParent, child);
            }
        }

        /**
         * 弹出栈顶元素
         */
        void Pop() noexcept
        {
            Remove(m_pMin);
        }

    private:
        void NodeSwap(IntrusiveHeapNode* parent, IntrusiveHeapNode* child) noexcept
        {
            IntrusiveHeapNode* sibling = nullptr;

            std::swap(parent->m_pLeft, child->m_pLeft);
            std::swap(parent->m_pRight, child->m_pRight);
            std::swap(parent->m_pParent, child->m_pParent);

            parent->m_pParent = child;
            if (child->m_pLeft == child)
            {
                child->m_pLeft = parent;
                sibling = child->m_pRight;
            }
            else
            {
                child->m_pRight = parent;
                sibling = child->m_pLeft;
            }
            if (sibling != nullptr)
                sibling->m_pParent = child;

            if (parent->m_pLeft != nullptr)
                parent->m_pLeft->m_pParent = parent;
            if (parent->m_pRight != nullptr)
                parent->m_pRight->m_pParent = parent;

            if (child->m_pParent == nullptr)
                m_pMin = child;
            else if (child->m_pParent->m_pLeft == parent)
                child->m_pParent->m_pLeft = child;
            else
                child->m_pParent->m_pRight = child;
        }

    private:
        IntrusiveHeapNode* m_pMin = nullptr;
        unsigned int m_uNelts = 0;
    };
}
