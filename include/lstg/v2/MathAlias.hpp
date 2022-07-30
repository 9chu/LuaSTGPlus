/**
 * @file
 * @date 2022/5/31
 * @author 9chu
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
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
