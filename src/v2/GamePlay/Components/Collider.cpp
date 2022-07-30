/**
 * @file
 * @date 2022/7/26
 * @author 9chu
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#include <lstg/v2/GamePlay/Components/Collider.hpp>

using namespace std;
using namespace lstg;
using namespace lstg::v2::GamePlay::Components;

// <editor-fold desc="Collider">

void Collider::Reset() noexcept
{
    // 从链表脱开
    if (PrevInChain)
    {
        assert(PrevInChain->NextInChain == this);
        PrevInChain->NextInChain = NextInChain;
    }
    if (NextInChain)
    {
        assert(NextInChain->PrevInChain == this);
        NextInChain->PrevInChain = PrevInChain;
    }

    // 重置
    Enabled = true;
    Shape = Math::Collider2D::CircleShape<double> { 0. };
    AABBHalfSize = { 0, 0 };
    Group = 0;
    BindingEntity = {};
    PrevInChain = nullptr;
    NextInChain = nullptr;
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

// </editor-fold>
// <editor-fold desc="ColliderRoot">

ColliderRoot::ColliderRoot() noexcept
{
    static_assert(std::extent_v<decltype(ColliderGroupHeaders)> == std::extent_v<decltype(ColliderGroupTailers)>);
    for (size_t i = 0; i < std::extent_v<decltype(ColliderGroupHeaders)>; ++i)
    {
        // Header <-> Tailer
        ColliderGroupHeaders[i].NextInChain = &ColliderGroupTailers[i];
        ColliderGroupTailers[i].PrevInChain = &ColliderGroupHeaders[i];
    }
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
        ColliderGroupHeaders[i].NextInChain = &ColliderGroupTailers[i];
        ColliderGroupTailers[i].PrevInChain = &ColliderGroupHeaders[i];
    }
}

// </editor-fold>
