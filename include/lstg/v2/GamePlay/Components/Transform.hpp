/**
 * @file
 * @date 2022/7/24
 * @author 9chu
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#pragma once
#include "../../MathAlias.hpp"

namespace lstg::v2::GamePlay::Components
{
    /**
     * 变换组件
     */
    struct Transform
    {
        /**
         * 坐标
         */
        Vec2 Location { 0., 0. };

        /**
         * 上一帧坐标
         */
        Vec2 LastLocation { 0., 0. };

        /**
         * 距离上一帧的坐标差量
         */
        Vec2 LocationDelta { 0., 0. };

        /**
         * 旋转量
         */
        double Rotation = 0;

        void Reset() noexcept;
    };

    constexpr uint32_t GetComponentId(Transform*) noexcept
    {
        return 0u;
    }
}
