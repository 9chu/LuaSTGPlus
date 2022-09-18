/**
 * @file
 * @date 2022/9/18
 * @author 9chu
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#pragma once
#include <cassert>
#include <cmath>

namespace lstg::Math
{
    template <typename T>
    inline T LinearToDecibel(T linear) noexcept
    {
        assert(linear > static_cast<T>(0));
        // https://en.wikipedia.org/wiki/Neper
        return ::log(linear) * static_cast<T>(8.6858896380650365530225783783321);
    }

    template <typename T>
    inline T DecibelToLinear(T decibel) noexcept
    {
        return ::exp(decibel * static_cast<T>(0.11512925464970228420089957273422));
    }

    template <typename T>
    inline T LinearToDecibelSafe(T linear) noexcept
    {
        assert(linear >= static_cast<T>(0));
        linear += static_cast<T>(0.0000000001);  // -200 dB
        return ::log(linear) * static_cast<T>(8.6858896380650365530225783783321);
    }

    template <typename T>
    inline T DecibelToLinearSafe(T decibel) noexcept
    {
        if (decibel <= static_cast<T>(-200))
            return static_cast<T>(0);
        auto linear = ::exp(decibel * static_cast<T>(0.11512925464970228420089957273422));
        if (linear <= static_cast<T>(0.0000000001))
            return static_cast<T>(0);
        return linear - static_cast<T>(0.0000000001);
    }
}
