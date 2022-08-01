/**
 * @file
 * @date 2022/7/24
 * @author 9chu
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#pragma once
#include "../../MathAlias.hpp"
#include "../../../Core/ECS/Entity.hpp"

namespace lstg::v2::GamePlay::Components
{
    static constexpr size_t kColliderGroupCount = 16;

    /**
     * 碰撞体
     */
    struct Collider
    {
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
        Collider* PrevInChain = nullptr;
        Collider* NextInChain = nullptr;

        Collider() noexcept = default;
        Collider(Collider&& org) noexcept;

        void Reset() noexcept;
        void RefreshAABB() noexcept;
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
        void Reset() noexcept;
    };

    constexpr uint32_t GetComponentId(ColliderRoot*) noexcept
    {
        return 2u;
    }
}
