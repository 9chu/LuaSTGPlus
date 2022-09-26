/**
 * @file
 * @date 2022/7/26
 * @author 9chu
 * 此文件为 LuaSTGPlus 项目的一部分，版权与许可声明详见 COPYRIGHT.txt。
 */
#pragma once
#include <cstdint>
#include <lstg/Core/IntrusiveList.hpp>
#include "../../../Core/ECS/Entity.hpp"

namespace lstg::v2::GamePlay::Components
{
    enum class LifeTimeStatus
    {
        Alive,
        Killed,
        Deleted,
    };

    const char* ToString(LifeTimeStatus status) noexcept;

    /**
     * 生命周期
     */
    struct LifeTime
    {
        static LifeTime* FromListNode(IntrusiveListNode* n) noexcept;

        /**
         * 对象状态
         */
        LifeTimeStatus Status = LifeTimeStatus::Alive;

        /**
         * 越界行为
         */
        bool OutOfBoundaryAutoRemove = true;

        /**
         * 计时器
         * 可以被修改为负数。
         */
        int32_t Timer = 0;

        /**
         * 自增 ID
         */
        uint64_t UniqueId = 0;

        /**
         * 链表域
         * 用于保持对象更新顺序。
         */
        ECS::Entity BindingEntity;
        IntrusiveListNode ListNode;

        LifeTime() noexcept = default;
        LifeTime(LifeTime&& org) noexcept;

        void Reset() noexcept;

        LifeTime* NextNode() noexcept;
        LifeTime* PrevNode() noexcept;
    };

    constexpr uint32_t GetComponentId(LifeTime*) noexcept
    {
        return 6u;
    }

    /**
     * 根元素
     */
    struct LifeTimeRoot
    {
        LifeTime LifeTimeHeader;
        LifeTime LifeTimeTailer;

        LifeTimeRoot() noexcept;
        LifeTimeRoot(LifeTimeRoot&&) noexcept;
        void Reset() noexcept;
    };

    constexpr uint32_t GetComponentId(LifeTimeRoot*) noexcept
    {
        return 7u;
    }
}
