/**
 * @file
 * @author 9chu
 * @date 2022/3/5
 * 此文件为 LuaSTGPlus 项目的一部分，版权与许可声明详见 COPYRIGHT.txt。
 */
#pragma once
#include <cmath>
#include <cstdint>
#include <cstring>
#include <algorithm>

namespace lstg::Subsystem::Render
{
    /**
     * R8G8B8A8 颜色表示
     */
    class ColorRGBA32
    {
    public:
        ColorRGBA32() noexcept
        {
            ::memset(m_stRGBA32, 0, sizeof(m_stRGBA32));
        }

        ColorRGBA32(uint8_t r, uint8_t g, uint8_t b, uint8_t a) noexcept
        {
            m_stRGBA32[0] = r;
            m_stRGBA32[1] = g;
            m_stRGBA32[2] = b;
            m_stRGBA32[3] = a;
        }

        ColorRGBA32(uint32_t rgba32) noexcept
        {
            m_stRGBA32[0] = (rgba32 & 0xFF000000) >> 24;
            m_stRGBA32[1] = (rgba32 & 0x00FF0000) >> 16;
            m_stRGBA32[2] = (rgba32 & 0x0000FF00) >> 8;
            m_stRGBA32[3] = (rgba32 & 0x000000FF);
        }

        ColorRGBA32(const ColorRGBA32&) noexcept = default;

        ColorRGBA32& operator=(uint32_t rgba32) noexcept
        {
            m_stRGBA32[0] = (rgba32 & 0xFF000000) >> 24;
            m_stRGBA32[1] = (rgba32 & 0x00FF0000) >> 16;
            m_stRGBA32[2] = (rgba32 & 0x0000FF00) >> 8;
            m_stRGBA32[3] = (rgba32 & 0x000000FF);
            return *this;
        }

        friend bool operator==(ColorRGBA32 a, ColorRGBA32 b) noexcept;
        friend bool operator!=(ColorRGBA32 a, ColorRGBA32 b) noexcept;
        friend ColorRGBA32 operator+(ColorRGBA32 a, ColorRGBA32 b) noexcept;
        friend ColorRGBA32 operator-(ColorRGBA32 a, ColorRGBA32 b) noexcept;
        friend ColorRGBA32 operator*(ColorRGBA32 a, ColorRGBA32 b) noexcept;
        friend ColorRGBA32 operator*(ColorRGBA32 c, double f) noexcept;
        friend ColorRGBA32 operator*(double f, ColorRGBA32 c) noexcept;

    public:
        inline uint32_t rgba32() const noexcept
        {
            return (m_stRGBA32[0] << 24u) | (m_stRGBA32[1] << 16u) | (m_stRGBA32[2] << 8u) | m_stRGBA32[3];
        }

        inline uint8_t r() const noexcept { return m_stRGBA32[0]; }
        inline uint8_t g() const noexcept { return m_stRGBA32[1]; }
        inline uint8_t b() const noexcept { return m_stRGBA32[2]; }
        inline uint8_t a() const noexcept { return m_stRGBA32[3]; }

        inline ColorRGBA32& r(uint8_t v) noexcept
        {
            m_stRGBA32[0] = v;
            return *this;
        }

        inline ColorRGBA32& g(uint8_t v) noexcept
        {
            m_stRGBA32[1] = v;
            return *this;
        }

        inline ColorRGBA32& b(uint8_t v) noexcept
        {
            m_stRGBA32[2] = v;
            return *this;
        }

        inline ColorRGBA32& a(uint8_t v) noexcept
        {
            m_stRGBA32[3] = v;
            return *this;
        }

    private:
        uint8_t m_stRGBA32[4];
    };

    inline bool operator==(ColorRGBA32 a, ColorRGBA32 b) noexcept
    {
        return ::memcmp(a.m_stRGBA32, b.m_stRGBA32, sizeof(a.m_stRGBA32)) == 0;
    }

    inline bool operator!=(ColorRGBA32 a, ColorRGBA32 b) noexcept
    {
        return ::memcmp(a.m_stRGBA32, b.m_stRGBA32, sizeof(a.m_stRGBA32)) != 0;
    }

    inline ColorRGBA32 operator+(ColorRGBA32 a, ColorRGBA32 b) noexcept
    {
        ColorRGBA32 ret {};
        ret.m_stRGBA32[0] = static_cast<uint8_t>(std::min<int>(255, a.m_stRGBA32[0] + b.m_stRGBA32[0]));
        ret.m_stRGBA32[1] = static_cast<uint8_t>(std::min<int>(255, a.m_stRGBA32[1] + b.m_stRGBA32[1]));
        ret.m_stRGBA32[2] = static_cast<uint8_t>(std::min<int>(255, a.m_stRGBA32[2] + b.m_stRGBA32[2]));
        ret.m_stRGBA32[3] = static_cast<uint8_t>(std::min<int>(255, a.m_stRGBA32[3] + b.m_stRGBA32[3]));
        return ret;
    }

    inline ColorRGBA32 operator-(ColorRGBA32 a, ColorRGBA32 b) noexcept
    {
        ColorRGBA32 ret {};
        ret.m_stRGBA32[0] = static_cast<uint8_t>(std::max<int>(0, a.m_stRGBA32[0] - b.m_stRGBA32[0]));
        ret.m_stRGBA32[1] = static_cast<uint8_t>(std::max<int>(0, a.m_stRGBA32[1] - b.m_stRGBA32[1]));
        ret.m_stRGBA32[2] = static_cast<uint8_t>(std::max<int>(0, a.m_stRGBA32[2] - b.m_stRGBA32[2]));
        ret.m_stRGBA32[3] = static_cast<uint8_t>(std::max<int>(0, a.m_stRGBA32[3] - b.m_stRGBA32[3]));
        return ret;
    }

    inline ColorRGBA32 operator*(ColorRGBA32 a, ColorRGBA32 b) noexcept
    {
        ColorRGBA32 ret {};
        ret.m_stRGBA32[0] = static_cast<uint8_t>(std::clamp<double>(a.m_stRGBA32[0] * (b.m_stRGBA32[0] / 255.), 0, 255));
        ret.m_stRGBA32[1] = static_cast<uint8_t>(std::clamp<double>(a.m_stRGBA32[1] * (b.m_stRGBA32[1] / 255.), 0, 255));
        ret.m_stRGBA32[2] = static_cast<uint8_t>(std::clamp<double>(a.m_stRGBA32[2] * (b.m_stRGBA32[2] / 255.), 0, 255));
        ret.m_stRGBA32[3] = static_cast<uint8_t>(std::clamp<double>(a.m_stRGBA32[3] * (b.m_stRGBA32[3] / 255.), 0, 255));
        return ret;
    }

    inline ColorRGBA32 operator*(double f, ColorRGBA32 c) noexcept
    {
        double r = f * c.m_stRGBA32[0];
        double g = f * c.m_stRGBA32[1];
        double b = f * c.m_stRGBA32[2];
        double a = f * c.m_stRGBA32[3];

        ColorRGBA32 ret {};
        ret.m_stRGBA32[0] = static_cast<uint8_t>(std::clamp<double>(r, 0, 255));
        ret.m_stRGBA32[1] = static_cast<uint8_t>(std::clamp<double>(g, 0, 255));
        ret.m_stRGBA32[2] = static_cast<uint8_t>(std::clamp<double>(b, 0, 255));
        ret.m_stRGBA32[3] = static_cast<uint8_t>(std::clamp<double>(a, 0, 255));
        return ret;
    }

    inline ColorRGBA32 operator*(ColorRGBA32 c, double f) noexcept
    {
        return f * c;
    }
}
