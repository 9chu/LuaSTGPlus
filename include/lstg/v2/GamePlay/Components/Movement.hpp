/**
 * @file
 * @date 2022/7/26
 * @author 9chu
 * 此文件为 LuaSTGPlus 项目的一部分，版权与许可声明详见 COPYRIGHT.txt。
 */
#pragma once
#include "../../MathAlias.hpp"

namespace lstg::v2::GamePlay::Components
{
    /**
     * 简单移动
     */
    struct Movement
    {
        /**
         * 角速度
         */
        double AngularVelocity = 0;

        /**
         * 线速度
         */
        Vec2 Velocity { 0, 0 };

        /**
         * 加速度
         */
        Vec2 AccelVelocity { 0, 0 };

        /**
         * 自动转向
         */
        bool RotateToSpeedDirection = false;

        void Reset() noexcept;
    };

    constexpr uint32_t GetComponentId(Movement*) noexcept
    {
        return 3u;
    }
}
