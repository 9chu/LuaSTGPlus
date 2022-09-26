/**
 * @file
 * @date 2022/7/26
 * @author 9chu
 * 此文件为 LuaSTGPlus 项目的一部分，版权与许可声明详见 COPYRIGHT.txt。
 */
#include <lstg/v2/GamePlay/Components/Collider.hpp>

#include <cstddef>

using namespace std;
using namespace lstg;
using namespace lstg::v2::GamePlay::Components;

// <editor-fold desc="Collider">

Collider* Collider::FromSkipListNode(IntrusiveSkipListNode<kColliderSkipListNodeDepth>* n) noexcept
{
    if (n)
    {
        auto ret = reinterpret_cast<Collider*>(reinterpret_cast<uint8_t*>(n) - offsetof(Collider, SkipListNode));
        assert(&ret->SkipListNode == n);
        return ret;
    }
    return nullptr;
}

Collider::Collider(Collider&& org) noexcept
    : Enabled(org.Enabled), Shape(org.Shape), AABBHalfSize(org.AABBHalfSize), Group(org.Group), BindingEntity(org.BindingEntity),
    SkipListNode(std::move(org.SkipListNode))
{
}

void Collider::Reset() noexcept
{
    // 从链表脱开
    SkipListRemove(&SkipListNode);

    // 重置
    Enabled = true;
    Shape = Math::Collider2D::CircleShape<double> { 0. };
    AABBHalfSize = { 0, 0 };
    Group = 0;
    BindingEntity = {};
}

void Collider::RefreshAABB() noexcept
{
    switch (Shape.index())
    {
        case 0:
            AABBHalfSize.x = AABBHalfSize.y = glm::length(std::get<0>(Shape).HalfSize);
            break;
        case 1:
            AABBHalfSize.x = AABBHalfSize.y = std::get<1>(Shape).Radius;
            break;
        case 2:
            AABBHalfSize.x = AABBHalfSize.y = std::max(std::get<2>(Shape).A, std::get<2>(Shape).B);
            break;
        default:
            assert(false);
            break;
    }
}

Collider* Collider::NextNode() noexcept
{
    auto n = SkipListNode.Adj[0].Next;
    return FromSkipListNode(n);
}

Collider* Collider::PrevNode() noexcept
{
    auto n = SkipListNode.Adj[0].Prev;
    return FromSkipListNode(n);
}

// </editor-fold>
// <editor-fold desc="ColliderRoot">

ColliderRoot::ColliderRoot() noexcept
{
    static_assert(std::extent_v<decltype(ColliderGroupHeaders)> == std::extent_v<decltype(ColliderGroupTailers)>);
    for (size_t i = 0; i < std::extent_v<decltype(ColliderGroupHeaders)>; ++i)
    {
        // Header <-> Tailer
        for (size_t j = 0; j < kColliderSkipListNodeDepth; ++j)
        {
            ColliderGroupHeaders[i].SkipListNode.Adj[j].Next = &ColliderGroupTailers[i].SkipListNode;
            ColliderGroupTailers[i].SkipListNode.Adj[j].Prev = &ColliderGroupHeaders[i].SkipListNode;
        }
    }
}

ColliderRoot::ColliderRoot(ColliderRoot&&) noexcept
{
    // 不应该发生内存迁移
    assert(false);
}

void ColliderRoot::Reset() noexcept
{
    static_assert(std::extent_v<decltype(ColliderGroupHeaders)> == std::extent_v<decltype(ColliderGroupTailers)>);
    for (size_t i = 0; i < std::extent_v<decltype(ColliderGroupHeaders)>; ++i)
    {
        // 将链表脱开
        ColliderGroupHeaders[i].Reset();
        ColliderGroupTailers[i].Reset();

        // Header <-> Tailer
        for (size_t j = 0; j < kColliderSkipListNodeDepth; ++j)
        {
            ColliderGroupHeaders[i].SkipListNode.Adj[j].Next = &ColliderGroupTailers[i].SkipListNode;
            ColliderGroupTailers[i].SkipListNode.Adj[j].Prev = &ColliderGroupHeaders[i].SkipListNode;
        }
    }
}

// </editor-fold>
