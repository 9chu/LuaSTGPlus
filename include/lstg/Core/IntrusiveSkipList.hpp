/**
 * @file
 * @author 9chu
 * @date 2022/9/25
 * 此文件为 LuaSTGPlus 项目的一部分，版权与许可声明详见 COPYRIGHT.txt。
 */
#pragma once
#include <cassert>
#include <cstdint>
#include <cstdlib>
#include <cstddef>
#include <cstring>
#include "Math/Randomizer.hpp"

namespace lstg
{
    /**
     * 侵入式跳表节点
     * @tparam Depth 深度
     */
    template <size_t Depth>
    struct IntrusiveSkipListNode
    {
        struct LinkListNode
        {
            IntrusiveSkipListNode* Prev;
            IntrusiveSkipListNode* Next;
        };

        LinkListNode Adj[Depth];

        IntrusiveSkipListNode() noexcept
        {
            ::memset(Adj, 0, sizeof(Adj));
        }

        IntrusiveSkipListNode(IntrusiveSkipListNode&& org) noexcept
        {
            for (size_t i = 0; i < Depth; ++i)
            {
                Adj[i] = org.Adj[i];
                if (Adj[i].Prev)
                    Adj[i].Prev->Adj[i].Next = this;
                if (Adj[i].Next)
                    Adj[i].Next->Adj[i].Prev = this;
            }
            ::memset(org.Adj, 0, sizeof(org.Adj));
        }
    };

    /**
     * 删除跳表节点
     * @tparam Depth 深度
     * @param node 节点
     */
    template <size_t Depth>
    inline void SkipListRemove(IntrusiveSkipListNode<Depth>* node) noexcept
    {
        for (size_t i = 0; i < Depth; ++i)
        {
            if (node->Adj[i].Prev)
                node->Adj[i].Prev->Adj[i].Next = node->Adj[i].Next;
            if (node->Adj[i].Next)
                node->Adj[i].Next->Adj[i].Prev = node->Adj[i].Prev;
        }
        ::memset(node->Adj, 0, sizeof(node->Adj));
    }

    /**
     * 插入跳表节点
     * @tparam Depth 深度
     * @tparam TComparer 比较器类型
     * @tparam TDepthRandomizer 深度随机数发生器类型
     * @param tail 尾节点
     * @param node 节点
     * @param comparer 比较器
     * @param randomizer 随机数发生器类型
     */
    template <size_t Depth, typename TComparer, typename TDepthRandomizer>
    inline void SkipListInsert(IntrusiveSkipListNode<Depth>* tail, IntrusiveSkipListNode<Depth>* node, TComparer&& comparer,
        TDepthRandomizer& randomizer) noexcept
    {
        static_assert(Depth > 0, "Invalid link list");

        auto insertDepth = randomizer();
        assert(0u < insertDepth && insertDepth <= Depth);
        size_t curDepth = Depth - 1;
        bool searchForward = true;
        auto p = tail;

        // 找到每一层要插入的位置
        while (true)
        {
            // 找到插入点
            if (searchForward)  // 向前搜索
            {
                // 此时，p可能是尾结点，也可能是一个有效的节点，且满足 node < p
                assert(p->Adj[curDepth].Next == nullptr || comparer(node, p));

                // 往前搜索，此时我们需要保证本行的插入点在 p 前面
                while (true)
                {
                    // 此时需要检查前一个节点是否为链表头结点
                    auto prev = p->Adj[curDepth].Prev;
                    if (prev->Adj[curDepth].Prev == nullptr)  // 头结点
                        break;

                    // 如果前面一个节点比 node 大，则可以往前走
                    if (comparer(node, prev))
                    {
                        p = prev;
                        continue;
                    }
                    break;
                }
                auto prev = p->Adj[curDepth].Prev;  // p 一定不是头结点
                assert(prev != nullptr && (prev->Adj[curDepth].Prev == nullptr || comparer(prev, node)));

                // 检查是否满足插入条件
                if (curDepth < insertDepth)
                {
                    // 此时总是在 p 前面插入
                    node->Adj[curDepth].Prev = p->Adj[curDepth].Prev;
                    node->Adj[curDepth].Next = p;
                    p->Adj[curDepth].Prev->Adj[curDepth].Next = node;
                    p->Adj[curDepth].Prev = node;
                }

                // 进入下一层
                if (curDepth == 0)
                    break;
                p = prev;  // 搜索起点需要调整为本层的前一个节点的下一层
                --curDepth;
                searchForward = false;  // 改为向后搜索
            }
            else  // 向后搜索
            {
                // 此时，p可能是头结点，也可能是一个有效的节点，且满足 p < node
                assert(p->Adj[curDepth].Prev == nullptr || comparer(p, node));

                // 往后搜索，此时我们需要保证本行的插入点在 p 后面
                while (true)
                {
                    // 此时需要检查下一个节点是否为链表尾结点
                    auto next = p->Adj[curDepth].Next;
                    if (next->Adj[curDepth].Next == nullptr)  // 尾结点
                        break;

                    // node >= next
                    if (!comparer(node, next))
                    {
                        p = next;
                        continue;
                    }
                    break;
                }
                auto next = p->Adj[curDepth].Next;  // p 一定不是尾结点
                assert(next != nullptr && (next->Adj[curDepth].Next == nullptr || comparer(node, next)));

                // 检查是否满足插入条件
                if (curDepth < insertDepth)
                {
                    // 此时总是在 p 后面插入
                    node->Adj[curDepth].Prev = p;
                    node->Adj[curDepth].Next = p->Adj[curDepth].Next;
                    p->Adj[curDepth].Next->Adj[curDepth].Prev = node;
                    p->Adj[curDepth].Next = node;
                }

                // 进入下一层
                if (curDepth == 0)
                    break;
                p = next;  // 搜索起点需要调整为本层的后一个节点的下一层
                --curDepth;
                searchForward = true;  // 改为向前搜索
            }
        }
    }

    /**
     * 跳表深度计算器
     * 通过随机的方式决定如何产生某个节点的跳表深度。
     * @tparam Depth 深度
     * @tparam Factor 系数，设置为2，则有0.5的概率进入下一层
     */
    template <size_t Depth, int Factor>
    struct SkipListDepthRandomizer
    {
        Math::Randomizer Rand;

        size_t operator()() noexcept
        {
            static const float kFactor = 1.f / Factor;

            size_t ret = 0;
            float c = 1.f;
            float r = Rand.NextFloat();
            while (r <= c && ret < Depth)
            {
                ++ret;
                c *= kFactor;
            }
            assert(0 < ret && ret <= Depth);
            return ret;
        }
    };
}
