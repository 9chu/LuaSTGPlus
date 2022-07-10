/**
 * @file
 * @date 2022/7/7
 * @author 9chu
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#pragma once
#include <glm/vec2.hpp>
#include <glm/vec4.hpp>
#include "../ColorRGBA32.hpp"

namespace lstg::Subsystem::Render::Drawing2D
{
    /**
     * 粒子对象
     */
    struct Particle
    {
        float Age = 0.f;  ///< @brief 存活时间
        float AliveTime = 0.f;  ///< @brief 总生存时间
        glm::vec2 Location { 0.f, 0.f };  ///< @brief 位置
        glm::vec2 Velocity { 0.f, 0.f };  ///< @brief 速度
        float Gravity = 0.f;  ///< @brief 重力
        float RadialAcceleration = 0.f;  ///< @brief 线加速度
        float TangentialAcceleration = 0.f;  ///< @brief 角加速度
        float Spin = 0.f;  ///< @brief 自旋
        float SpinDelta = 0.f;  ///< @brief 自旋增量
        float Size = 0.f;  ///< @brief 大小
        float SizeDelta = 0.f;  ///< @brief 大小增量
        glm::vec4 Color { 0.f, 0.f, 0.f, 0.f };  ///< @brief 颜色
        glm::vec4 ColorDelta { 0.f, 0.f, 0.f, 0.f };  ///< @brief 颜色增量
    };
}
