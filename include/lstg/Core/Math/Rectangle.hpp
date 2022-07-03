/**
 * @file
 * @author 9chu
 * @date 2022/5/15
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#pragma once
#include <cmath>
#include <cstdint>
#include <algorithm>
#include <tuple>
#include <type_traits>
#include <glm/vec2.hpp>

namespace lstg::Math
{
    struct BottomUpTag {};  // 向上
    struct TopDownTag {};  // 向下

    template <typename T = float, typename YAxisDirection = TopDownTag>
    class Rectangle
    {
    public:
        using Vector = glm::vec<2, T, glm::defaultp>;

    public:
        Rectangle()
            : m_stTopLeft(0, 0), m_stSize(0, 0) {}

        Rectangle(Vector topLeft, Vector size)
            : m_stTopLeft(topLeft), m_stSize(size) {}

        Rectangle(T x, T y, T w, T h)
            : m_stTopLeft(x, y), m_stSize(w, h) {}

        bool operator==(const Rectangle& rhs) const noexcept
        {
            return m_stTopLeft == rhs.m_stTopLeft && m_stSize == rhs.m_stSize;
        }

        bool operator!=(const Rectangle& rhs) const noexcept
        {
            return !operator==(rhs);
        }

    public:
        /**
         * 获取左侧距离
         */
        T Left() const noexcept { return m_stTopLeft.x; }

        /**
         * 设置左侧距离
         */
        void SetLeft(T v) noexcept { m_stTopLeft.x = v; }

        /**
         * 获取顶边距离
         */
        T Top() const noexcept { return m_stTopLeft.y; }

        /**
         * 设置顶边距离
         */
        void SetTop(T v) noexcept { m_stTopLeft.y = v; }

        /**
         * 获取宽度
         */
        T Width() const noexcept { return m_stSize.x; }

        /**
         * 设置宽度
         */
        void SetWidth(T v) noexcept { m_stSize.x = v; }

        /**
         * 获取高度
         */
        T Height() const noexcept { return m_stSize.y; }

        /**
         * 设置高度
         */
        void SetHeight(T v) noexcept { m_stSize.y = v; }

        /**
         * 获取左上角位置
         */
        Vector GetTopLeft() const noexcept { return m_stTopLeft; }

        /**
         * 获取右下角位置
         */
        Vector GetBottomRight() const noexcept
        {
            if constexpr (std::is_same_v<YAxisDirection, BottomUpTag>)
                return { m_stTopLeft.x + m_stSize.x, m_stTopLeft.y - m_stSize.y };
            else
                return { m_stTopLeft.x + m_stSize.x, m_stTopLeft.y + m_stSize.y };
        }

        /**
         * 获取中心点
         */
        Vector GetCenter() const noexcept
        {
            return (GetTopLeft() + GetBottomRight()) / T(2);
        }

        /**
         * 检查点是否在矩形内（包含边界）
         * @param point 被判断点
         * @return 是否在矩形内
         */
        bool ContainsPoint(glm::vec2 point) const noexcept
        {
            auto topLeft = GetTopLeft();
            auto bottomRight = GetBottomRight();
            if constexpr (std::is_same_v<YAxisDirection, BottomUpTag>)
                return topLeft.x <= point.x && point.x <= bottomRight.x && topLeft.y >= point.y && point.y >= bottomRight.y;
            else
                return topLeft.x <= point.x && point.x <= bottomRight.x && topLeft.y <= point.y && point.y <= bottomRight.y;
        }

        /**
         * 计算矩形相交
         * @param another 被相交矩形
         * @return 返回是否相交，若相交返回相交的矩形
         */
        std::tuple<bool, Rectangle> Intersect(const Rectangle& another) const noexcept
        {
            auto a = GetTopLeft();
            auto b = GetBottomRight();
            auto oa = another.GetTopLeft();
            auto ob = another.GetBottomRight();
            if constexpr (std::is_same_v<YAxisDirection, BottomUpTag>)
            {
                Vector na { std::max(a.x, oa.x), std::min(a.y, oa.y) };
                Vector nb { std::min(b.x, ob.x), std::max(b.y, ob.y) };
                if (na.x <= nb.x && na.y >= nb.y)
                    return { true, Rectangle(na, na - nb) };
            }
            else
            {
                Vector na { std::max(a.x, oa.x), std::max(a.y, oa.y) };
                Vector nb { std::min(b.x, ob.x), std::min(b.y, ob.y) };
                if (na.x <= nb.x && na.y <= nb.y)
                    return { true, Rectangle(na, nb - na) };
            }
            return { false, {} };
        }

        /**
         * 计算并集
         * @param another 被合并的矩形
         * @return 返回一个合并当前矩形和被合并矩形的矩形
         */
        Rectangle Union(const Rectangle& another) const noexcept
        {
            auto a = GetTopLeft();
            auto b = GetBottomRight();
            auto oa = another.GetTopLeft();
            auto ob = another.GetBottomRight();
            if constexpr (std::is_same_v<YAxisDirection, BottomUpTag>)
            {
                Vector na { std::min(a.x, oa.x), std::max(a.y, oa.y) };
                Vector nb { std::max(b.x, ob.x), std::min(b.y, ob.y) };
                return Rectangle(na.x, na.y, nb.x - na.x, na.y - nb.y);
            }
            else
            {
                Vector na { std::min(a.x, oa.x), std::min(a.y, oa.y) };
                Vector nb { std::max(b.x, ob.x), std::max(b.y, ob.y) };
                return Rectangle(na.x, na.y, nb.x - na.x, nb.y - na.y);
            }
        }

    private:
        Vector m_stTopLeft;
        Vector m_stSize;
    };
}
