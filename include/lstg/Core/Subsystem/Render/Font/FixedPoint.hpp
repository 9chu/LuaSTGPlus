/**
 * @file
 * @date 2022/6/26
 * @author 9chu
 * 此文件为 LuaSTGPlus 项目的一部分，版权与许可声明详见 COPYRIGHT.txt。
 */
#pragma once
#include <cmath>
#include <cstdint>

namespace lstg::Subsystem::Render::Font
{
    /**
     * 这里的定点数用于字体渲染的内部计算和表示，与 FreeType 类型保持一致。
     * 其中，Q26D6 用于所有度量值的场合，Q16D16 用于存储缩放值。
     * 定点数采取 Q-Format（AMD Version） 进行表示。
     * https://en.wikipedia.org/wiki/Q_(number_format)
     */

    /**
     * Q25.6 定点数
     *   - 整数部分带符号位一共 26 位
     *   - 小数部分 6 位
     */
    using Q26D6 = int32_t;

    /**
     * Q16.16 定点数
     *   - 整数部分带符号位一共 16 位
     *   - 小数部分 16 位
     */
    using Q16D16 = int32_t;

    /**
     * 从整数像素值转 Q26.6 定点数
     * @param val 整数值
     * @return 定点数
     */
    constexpr inline Q26D6 PixelToQ26D6(int32_t val) noexcept
    {
        return static_cast<Q26D6>(val << 6);
    }

    /**
     * 从浮点像素值转 Q26.6 定点数
     * @param val 浮点值
     * @return 定点数
     */
    constexpr inline Q26D6 PixelToQ26D6(float val) noexcept
    {
        return static_cast<Q26D6>(val * 64.f);
    }

    /**
     * Q26.6 定点数转整数像素值
     * @param val 定点数
     * @return 整数像素值
     */
    constexpr inline int32_t Q26D6ToPixel(Q26D6 val) noexcept
    {
        return static_cast<int32_t>((val + (1 << 5)) >> 6);
    }

    /**
     * Q26.6 定点数转浮点像素值
     * @param val 定点数
     * @return 浮点像素值
     */
    inline float Q26D6ToPixelF(Q26D6 val) noexcept
    {
        return std::round(static_cast<float>(val) / 64.f);
    }

    /**
     * 从整数像素值转 Q16.16 定点数
     * @param val 整数值
     * @return 定点数
     */
    constexpr inline Q16D16 PixelToQ16D16(int32_t val) noexcept
    {
        return static_cast<Q16D16>(val << 16);
    }

    /**
     * 从浮点像素值转 Q16.16 定点数
     * @param val 浮点值
     * @return 定点数
     */
    constexpr inline Q16D16 PixelToQ16D16(float val) noexcept
    {
        return static_cast<Q16D16>(val * 65536.f);
    }

    /**
     * Q16.16 定点数转整数像素值
     * @param val 定点数
     * @return 整数像素值
     */
    constexpr inline int32_t Q16D16ToPixel(Q16D16 val) noexcept
    {
        return static_cast<int32_t>((val + (1 << 15)) >> 16);
    }

    /**
     * Q16.16 定点数转浮点像素值
     * @param val 定点数
     * @return 浮点像素值
     */
    inline float Q16D16ToPixelF(Q16D16 val) noexcept
    {
        return std::round(static_cast<float>(val) / 65536.f);
    }
}
