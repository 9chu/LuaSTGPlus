/**
 * @file
 * @date 2022/7/9
 * @author 9chu
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#pragma once
#include <glm/vec2.hpp>
#include <glm/gtx/norm.hpp>

namespace lstg::Math
{
    /**
     * 安全规格化
     * @note 对于零长度向量，返回零长度向量
     * @tparam T 数值类型
     * @tparam N 向量维度
     * @param vec 向量
     * @return 规格化后向量
     */
    template <typename T = float, int N>
    inline glm::vec<N, T, glm::defaultp> Normalize(glm::vec<N, T, glm::defaultp> vec) noexcept
    {
        auto len2 = glm::length2(vec);
        if (len2 == 0.f)
            return glm::zero<glm::vec<N, T, glm::defaultp>>();
        return vec / ::sqrt(len2);
    }

    /**
     * 计算二维向量与 X 轴夹角度数
     * @tparam T 数值类型
     * @param vec 向量
     * @return 角度
     */
    template <typename T = float>
    inline T Angle(glm::vec<2, T, glm::defaultp> vec) noexcept
    {
        auto len2 = glm::length2(vec);
        if (len2 == 0.f)
            return 0.f;
        return ::atan2f(vec.y, vec.x);
    }

    /**
     * 计算二维向量之间的夹角
     * @tparam T 数值类型
     * @param vec1 向量1
     * @param vec2 向量2
     * @return 角度
     */
    template <typename T = float>
    inline T Angle(glm::vec<2, T, glm::defaultp> vec1, glm::vec<2, T, glm::defaultp> vec2) noexcept
    {
        vec1 = Normalize(vec1);
        vec2 = Normalize(vec2);
        return ::acosf(glm::dot(vec1, vec2));
    }

    /**
     * 计算叉积
     * @tparam T 数值类型
     * @param lhs 2D 向量1
     * @param rhs 2D 向量2
     * @return 叉积
     */
    template <typename T = float>
    inline T Cross(glm::vec<2, T, glm::defaultp> lhs, glm::vec<2, T, glm::defaultp> rhs) noexcept
    {
        return lhs.x * rhs.y - lhs.y * rhs.x;
    }
}
