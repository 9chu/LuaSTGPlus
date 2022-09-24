/**
 * @file
 * @date 2022/5/31
 * @author 9chu
 * 此文件为 LuaSTGPlus 项目的一部分，版权与许可声明详见 COPYRIGHT.txt。
 */
#pragma once
#include <lstg/Core/Math/Rectangle.hpp>
#include <lstg/Core/Subsystem/Render/ColorRGBA32.hpp>
#include <lstg/Core/Math/Collider2D/ColliderShape.hpp>

namespace lstg::v2
{
    // 这里定义 GamePlay 所用的数据结构
    using Vec2 = glm::vec<2, double, glm::defaultp>;
    using WorldRectangle = Math::Rectangle<double, Math::BottomUpTag>;
    using ColliderShape = Math::Collider2D::ColliderShape<double>;
}
