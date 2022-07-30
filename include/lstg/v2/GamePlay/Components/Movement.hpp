/**
 * @file
 * @date 2022/7/26
 * @author 9chu
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
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
