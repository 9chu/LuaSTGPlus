/**
 * @file
 * @date 2022/8/3
 * @author 9chu
 * 此文件为 LuaSTGPlus 项目的一部分，版权与许可声明详见 COPYRIGHT.txt。
 */
#pragma once
#include <glm/gtx/norm.hpp>
#include "ColliderShape.hpp"
#include "../VectorHelper.hpp"

namespace lstg::Math::Collider2D
{
    namespace detail
    {
        template <typename T>
        inline T PointToLineDist(const glm::vec<2, T, glm::defaultp>& p0, const glm::vec<2, T, glm::defaultp>& p1, T cos0,
            T sin0) noexcept
        {
            // https://github.com/Xrysnow/lstgx_Math/blob/d63939b5b5cd69ffeaa908f2ec38708a62ad3139/XDistance.h#L24
            return sin0 * (p0.x - p1.x) - cos0 * (p0.y - p1.y);
        }

        template <typename T>
        inline T PointToParallelogramDist(const glm::vec<2, T, glm::defaultp>& p0, const glm::vec<2, T, glm::defaultp>& p1,
            const glm::vec<2, T, glm::defaultp>& a, const glm::vec<2, T, glm::defaultp>& b) noexcept
        {
            // https://github.com/Xrysnow/lstgx_Math/blob/d63939b5b5cd69ffeaa908f2ec38708a62ad3139/XDistance.cpp
            const auto p = p0 - p1;
            const auto ab = b - a;
            const auto ad = -b - a;
            const auto ax = p - a;
            const auto nA2B = glm::dot(ab, ax) < T(0);
            const auto nA2D = glm::dot(ad, ax) < T(0);
            if (nA2B && nA2D)
                return glm::length(ax);
            const auto bx = p - b;
            const auto nB2A = glm::dot(ab, bx) > T(0);
            const auto nB2C = glm::dot(ad, bx) < T(0);
            if (nB2A && nB2C)
                return glm::length(bx);
            const auto cx = p + a;
            const auto nC2B = glm::dot(ad, cx) > T(0);
            const auto nC2D = glm::dot(ab, cx) > T(0);
            if (nC2B && nC2D)
                return glm::length(cx);
            const auto dx = p + b;
            const auto nD2A = glm::dot(ad, dx) > T(0);
            const auto nD2C = glm::dot(ab, dx) < T(0);
            if (nD2A && nD2C)
                return glm::length(dx);

            if (!nA2B && !nB2A && Cross(ab, a) * Cross(ab, ax) > T(0))
            {
                const auto e = glm::normalize(ab);
                return std::abs(PointToLineDist(p, a, e.x, e.y));
            }
            if (!nC2D && !nD2C && Cross(ab, a) * Cross(ab, cx) < T(0))
            {
                const auto e = glm::normalize(ab);
                return std::abs(PointToLineDist(p, -a, e.x, e.y));
            }
            if (!nA2D && !nD2A && Cross(ad, a) * Cross(ad, ax) > T(0))
            {
                const auto e = glm::normalize(ad);
                return std::abs(PointToLineDist(p, a, e.x, e.y));
            }
            if (!nC2B && !nB2C && Cross(ad, a) * Cross(ad, bx) < T(0))
            {
                const auto e = glm::normalize(ad);
                return std::abs(PointToLineDist(p, b, e.x, e.y));
            }
            return T(0);
        }

        template <typename T>
        inline T PointToEllipseDistFast(const glm::vec<2, T, glm::defaultp>& p0, const glm::vec<2, T, glm::defaultp>& p1, T a, T b,
            T rotation) noexcept
        {
            // https://github.com/Xrysnow/lstgx_Math/blob/d63939b5b5cd69ffeaa908f2ec38708a62ad3139/XDistance.cpp#L276
            const auto p = p0 - p1;
            T cos0 = ::cos(-rotation), sin0 = ::sin(-rotation);
            const auto x = std::abs(p.x * cos0 - p.y * sin0);
            const auto y = std::abs(p.y * cos0 + p.x * sin0);
            if (x * x / (a * a) + y * y / (b * b) <= T(1))
                return T(0);
            const auto a2 = a * a;
            const auto b2 = b * b;
            const auto ax = a * x;
            const auto by = b * y;
            const auto tmp = b2 - a2;
            auto theta = glm::pi<T>() / T(4) - (((b2 - a2) / ::sqrt(T(2))) + ax - by) / (ax + by);
            theta = std::max<T>(T(0), std::min(theta, glm::pi<T>() / T(2)));
            T ct = ::cos(theta), st = ::sin(theta);
            for (auto i = 0; i < 2; ++i)
            {
                const auto dtheta = (tmp * st * ct + ax * st - by * ct) / (tmp * (ct * ct - st * st) + ax * ct + by * st);
                if (std::abs(dtheta) < T(1e-5f))
                    break;
                theta = theta - dtheta;
                ct = ::cos(theta); st = ::sin(theta);
            }
            const auto dx = a * ct - x;
            const auto dy = b * st - y;
            return std::sqrt(dx * dx + dy * dy);
        }
    }

    // <editor-fold desc="OBB">

    template <typename T>
    inline bool IsIntersect(const glm::vec<2, T, glm::defaultp>& locA, T rotA, const OBBShape<T>& shapeA,
        const glm::vec<2, T, glm::defaultp>& locB, T rotB, const OBBShape<T>& shapeB) noexcept
    {
        // https://github.com/Xrysnow/lstgx_Math/blob/d63939b5b5cd69ffeaa908f2ec38708a62ad3139/XIntersect.cpp#L81
        auto sin0 = ::sin(rotA), cos0 = ::cos(rotA);
        auto sin1 = ::sin(rotB), cos1 = ::cos(rotB);
        glm::vec<2, T, glm::defaultp> e[] = {
            {  cos0, sin0 },
            { -sin0, cos0 },
            {  cos1, sin1 },
            { -sin1, cos1 },
        };
        T projOther[] = {
            shapeA.HalfSize.x,
            shapeA.HalfSize.y,
            shapeB.HalfSize.x,
            shapeB.HalfSize.y
        };
        const auto d = locA - locB;
        for (auto i = 0; i < 4; i++)
        {
            const auto ii = 2 - size_t(i / 2) * 2;
            const auto v0 = e[ii] * projOther[ii];
            const auto v1 = e[ii + 1] * projOther[ii + 1];
            const auto ex = e[i].x;
            const auto ey = e[i].y;
            const auto projHalfDiag = std::max(
                std::abs(ex * (v0.x + v1.x) + ey * (v0.y + v1.y)),
                std::abs(ex * (v0.x - v1.x) + ey * (v0.y - v1.y))
            );
            if (projHalfDiag + projOther[i] < std::abs(ex * d.x + ey * d.y))
                return false;
        }
        return true;
    }

    template <typename T>
    inline bool IsIntersect(const glm::vec<2, T, glm::defaultp>& locA, T rotA, const OBBShape<T>& shapeA,
        const glm::vec<2, T, glm::defaultp>& locB, T rotB, const CircleShape<T>& shapeB) noexcept
    {
        // https://github.com/Xrysnow/lstgx_Math/blob/d63939b5b5cd69ffeaa908f2ec38708a62ad3139/XIntersect.cpp#L70
        auto sin0 = ::sin(rotA), cos0 = ::cos(rotA);
        const auto d = locA - locB;
        const auto dw = std::max<T>(T(0), std::abs(cos0 * d.x + sin0 * d.y) - shapeA.HalfSize.x);
        const auto dh = std::max<T>(T(0), std::abs(-sin0 * d.x + cos0 * d.y) - shapeA.HalfSize.y);
        return shapeB.Radius * shapeB.Radius > dh * dh + dw * dw;
    }

    template <typename T>
    inline bool IsIntersect(const glm::vec<2, T, glm::defaultp>& locA, T rotA, const OBBShape<T>& shapeA,
        const glm::vec<2, T, glm::defaultp>& locB, T rotB, const EllipseShape<T>& shapeB) noexcept
    {
        // https://github.com/Xrysnow/lstgx_Math/blob/d63939b5b5cd69ffeaa908f2ec38708a62ad3139/XIntersect.cpp#L160
        // TODO：这里应该是退化到了平行四边形来近似计算，严格计算可以利用 GJK 算法来搞
        auto sin0 = ::sin(rotA), cos0 = ::cos(rotA);
        auto sin1 = ::sin(rotB), cos1 = ::cos(rotB);
        const auto e00 = glm::vec<2, T, glm::defaultp>(cos0, sin0);
        const auto e01 = glm::vec<2, T, glm::defaultp>(-sin0, cos0);
        const auto e11 = glm::vec<2, T, glm::defaultp>(-sin1, cos1);
        const auto f = e11 * (shapeB.A / shapeB.B - 1);
        const auto p0d = locA + detail::PointToLineDist(locA, locB, e11.x, e11.y) * f;
        const auto tmp = e00 * shapeA.HalfSize.x + locA;
        const auto vDiag0 = tmp + e01 * shapeA.HalfSize.y;
        const auto vDiag1 = tmp - e01 * shapeA.HalfSize.y;
        const auto vDiag0d = vDiag0 + detail::PointToLineDist(vDiag0, locB, e11.x, e11.y) * f;
        const auto vDiag1d = vDiag1 + detail::PointToLineDist(vDiag1, locB, e11.x, e11.y) * f;
        const auto halfDiag0d = vDiag0d - p0d;
        const auto halfDiag1d = vDiag1d - p0d;
        const auto d = detail::PointToParallelogramDist(locB, p0d, halfDiag0d, halfDiag1d);
        return d <= shapeB.A;
    }

    // </editor-fold>
    // <editor-fold desc="Circle">

    template <typename T>
    inline bool IsIntersect(const glm::vec<2, T, glm::defaultp>& locA, T rotA, const CircleShape<T>& shapeA,
        const glm::vec<2, T, glm::defaultp>& locB, T rotB, const OBBShape<T>& shapeB) noexcept
    {
        return IsIntersect(locB, rotB, shapeB, locA, rotA, shapeA);
    }

    template <typename T>
    inline bool IsIntersect(const glm::vec<2, T, glm::defaultp>& locA, T rotA, const CircleShape<T>& shapeA,
        const glm::vec<2, T, glm::defaultp>& locB, T rotB, const CircleShape<T>& shapeB) noexcept
    {
        auto delta = glm::length2(locA - locB);
        auto dist = shapeA.Radius + shapeB.Radius;
        return delta <= dist * dist;
    }

    template <typename T>
    inline bool IsIntersect(const glm::vec<2, T, glm::defaultp>& locA, T rotA, const CircleShape<T>& shapeA,
        const glm::vec<2, T, glm::defaultp>& locB, T rotB, const EllipseShape<T>& shapeB) noexcept
    {
        return detail::PointToEllipseDistFast(locA, locB, shapeB.A, shapeB.B, rotB) <= shapeA.Radius;
    }

    // </editor-fold>
    // <editor-fold desc="Ellipse">

    template <typename T>
    inline bool IsIntersect(const glm::vec<2, T, glm::defaultp>& locA, T rotA, const EllipseShape<T>& shapeA,
        const glm::vec<2, T, glm::defaultp>& locB, T rotB, const OBBShape<T>& shapeB) noexcept
    {
        return IsIntersect(locB, rotB, shapeB, locA, rotA, shapeA);
    }

    template <typename T>
    inline bool IsIntersect(const glm::vec<2, T, glm::defaultp>& locA, T rotA, const EllipseShape<T>& shapeA,
        const glm::vec<2, T, glm::defaultp>& locB, T rotB, const CircleShape<T>& shapeB) noexcept
    {
        return IsIntersect(locB, rotB, shapeB, locA, rotA, shapeA);
    }

    template <typename T>
    inline bool IsIntersect(const glm::vec<2, T, glm::defaultp>& locA, T rotA, const EllipseShape<T>& shapeA,
        const glm::vec<2, T, glm::defaultp>& locB, T rotB, const EllipseShape<T>& shapeB) noexcept
    {
        // https://github.com/Xrysnow/lstgx_Math/blob/d63939b5b5cd69ffeaa908f2ec38708a62ad3139/XIntersect.cpp#L201
        T s = ::sin(rotB - rotA), c = ::cos(rotB - rotA);
        const auto c2 = c * c;
        const auto s2 = s * s;
        const auto sc = s * c;
        const auto ai = 1 / (shapeB.A * shapeB.A);
        const auto bi = 1 / (shapeB.B * shapeB.B);
        const auto m00 = (ai * c2 + bi * s2) * (shapeA.A * shapeA.A);
        const auto m11 = (bi * c2 + ai * s2) * (shapeA.B * shapeA.B);
        const auto m01 = (ai - bi) * sc * (shapeA.A * shapeA.B);
        const auto sum = m00 + m11;
        const auto tmp = m00 - m11;
        const auto dif = std::sqrt(tmp * tmp + T(4) * m01 * m01);
        const auto tanv = T(2) * m01 / (dif + m00 - m11);
        T s0 = ::sin(-rotA), c0 = ::cos(-rotA);
        const auto d = locB - locA;
        auto dd = glm::vec<2, T, glm::defaultp>(d.x * c0 - d.y * s0, d.y * c0 + d.x * s0);
        dd.x /= shapeA.A;
        dd.y /= shapeA.B;
        return detail::PointToEllipseDistFast({0, 0}, dd, std::sqrt(T(2) / (sum + dif)), std::sqrt(T(2) / (sum - dif)), std::atan(tanv))
               <= T(1);
    }

    // </editor-fold>

    template <typename T>
    inline bool IsIntersect(const glm::vec<2, T, glm::defaultp>& locA, T rotA, const ColliderShape<T>& shapeA,
        const glm::vec<2, T, glm::defaultp>& locB, T rotB, const ColliderShape<T>& shapeB) noexcept
    {
        return std::visit([&](auto& a) {
            return std::visit([&](auto& b) {
                return IsIntersect(locA, rotA, a, locB, rotB, b);
            }, shapeB);
        }, shapeA);
    }
}
