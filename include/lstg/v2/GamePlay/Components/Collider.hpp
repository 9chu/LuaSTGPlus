/**
 * @file
 * @date 2022/7/24
 * @author 9chu
 * 此文件为 LuaSTGPlus 项目的一部分，版权与许可声明详见 COPYRIGHT.txt。
 */
#pragma once
#include <lstg/Core/IntrusiveSkipList.hpp>
#include "../../MathAlias.hpp"
#include "../../../Core/ECS/Entity.hpp"

namespace lstg::v2::GamePlay::Components
{
    static constexpr size_t kColliderSkipListNodeDepth = 3;
    static constexpr size_t kColliderGroupCount = 16;

    /**
     * 碰撞体
     */
    struct Collider
    {
        static Collider* FromSkipListNode(IntrusiveSkipListNode<kColliderSkipListNodeDepth>* n) noexcept;

        /**
         * 是否启用碰撞体
         */
        bool Enabled = true;

        /**
         * 碰撞外形
         */
        ColliderShape Shape = Math::Collider2D::CircleShape<double> { 0. };

        /**
         * 外接矩形半大小
         * 这里的外接矩形考虑了任意旋转下的大小，故总是有 X = Y。
         * 受 Shape 影响。
         */
        Vec2 AABBHalfSize { 0, 0 };

        /**
         * 碰撞组
         * 用于快速遍历碰撞组。
         */
        uint32_t Group = 0;
        ECS::Entity BindingEntity;
        IntrusiveSkipListNode<kColliderSkipListNodeDepth> SkipListNode;

        Collider() noexcept {} /* = default; */  // g++ won't compile, make it happy
        Collider(Collider&& org) noexcept;

        void Reset() noexcept;
        void RefreshAABB() noexcept;

        Collider* NextNode() noexcept;
        Collider* PrevNode() noexcept;
    };

    constexpr uint32_t GetComponentId(Collider*) noexcept
    {
        return 1u;
    }

    /**
     * 根元素
     */
    struct ColliderRoot
    {
        Collider ColliderGroupHeaders[kColliderGroupCount];
        Collider ColliderGroupTailers[kColliderGroupCount];

        ColliderRoot() noexcept;
        ColliderRoot(ColliderRoot&&) noexcept;
        void Reset() noexcept;
    };

    constexpr uint32_t GetComponentId(ColliderRoot*) noexcept
    {
        return 2u;
    }
}
