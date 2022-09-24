/**
 * @file
 * @date 2022/5/16
 * @author 9chu
 * 此文件为 LuaSTGPlus 项目的一部分，版权与许可声明详见 COPYRIGHT.txt。
 */
#pragma once
#include <variant>
#include <glm/glm.hpp>

namespace lstg::Math::Collider2D
{
    template <typename T>
    struct OBBShape
    {
        glm::vec<2, T, glm::defaultp> HalfSize { 0, 0 };
    };

    template <typename T>
    struct CircleShape
    {
        T Radius = static_cast<T>(0);
    };

    template <typename T>
    struct EllipseShape
    {
        T A = static_cast<T>(0);  // 半长轴
        T B = static_cast<T>(0);  // 半短轴
    };

    template <typename T>
    using ColliderShape = std::variant<OBBShape<T>, CircleShape<T>, EllipseShape<T>>;
}
