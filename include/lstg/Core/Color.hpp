/**
 * @file
 * @author 9chu
 * @date 2022/3/5
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#pragma once
#include <cmath>
#include <cstdint>
#include <cstring>
#include <algorithm>

namespace lstg
{
    /**
     * RGBA 32 颜色表示
     */
    class RGBA32Color
    {
    public:
        RGBA32Color() noexcept
        {
            ::memset(m_stRGBA32, 0, sizeof(m_stRGBA32));
        }

        RGBA32Color(uint32_t rgba32) noexcept
        {
            m_stRGBA32[0] = (rgba32 & 0xFF000000) >> 24;
            m_stRGBA32[1] = (rgba32 & 0x00FF0000) >> 16;
            m_stRGBA32[2] = (rgba32 & 0x0000FF00) >> 8;
            m_stRGBA32[3] = (rgba32 & 0x000000FF);
        }

        RGBA32Color& operator=(uint32_t rgba32) noexcept
        {
            m_stRGBA32[0] = (rgba32 & 0xFF000000) >> 24;
            m_stRGBA32[1] = (rgba32 & 0x00FF0000) >> 16;
            m_stRGBA32[2] = (rgba32 & 0x0000FF00) >> 8;
            m_stRGBA32[3] = (rgba32 & 0x000000FF);
            return *this;
        }

        friend bool operator==(RGBA32Color a, RGBA32Color b) noexcept;
        friend bool operator!=(RGBA32Color a, RGBA32Color b) noexcept;
        friend RGBA32Color operator+(RGBA32Color a, RGBA32Color b) noexcept;
        friend RGBA32Color operator-(RGBA32Color a, RGBA32Color b) noexcept;
        friend RGBA32Color operator*(RGBA32Color a, RGBA32Color b) noexcept;
        friend RGBA32Color operator*(RGBA32Color c, double f) noexcept;
        friend RGBA32Color operator*(double f, RGBA32Color c) noexcept;

    public:
        inline auto& rgba32() const noexcept { return m_stRGBA32; }

        inline uint8_t r() const noexcept { return m_stRGBA32[0]; }
        inline uint8_t g() const noexcept { return m_stRGBA32[1]; }
        inline uint8_t b() const noexcept { return m_stRGBA32[2]; }
        inline uint8_t a() const noexcept { return m_stRGBA32[3]; }

        inline RGBA32Color& r(uint8_t v) noexcept
        {
            m_stRGBA32[0] = v;
            return *this;
        }

        inline RGBA32Color& g(uint8_t v) noexcept
        {
            m_stRGBA32[1] = v;
            return *this;
        }

        inline RGBA32Color& b(uint8_t v) noexcept
        {
            m_stRGBA32[2] = v;
            return *this;
        }

        inline RGBA32Color& a(uint8_t v) noexcept
        {
            m_stRGBA32[3] = v;
            return *this;
        }

    private:
        uint8_t m_stRGBA32[4];
    };

    inline bool operator==(RGBA32Color a, RGBA32Color b) noexcept
    {
        return ::memcmp(a.m_stRGBA32, b.m_stRGBA32, sizeof(a.m_stRGBA32)) == 0;
    }

    inline bool operator!=(RGBA32Color a, RGBA32Color b) noexcept
    {
        return ::memcmp(a.m_stRGBA32, b.m_stRGBA32, sizeof(a.m_stRGBA32)) != 0;
    }

    inline RGBA32Color operator+(RGBA32Color a, RGBA32Color b) noexcept
    {
        RGBA32Color ret {};
        ret.m_stRGBA32[0] = static_cast<uint8_t>(std::min<int>(255, a.m_stRGBA32[0] + b.m_stRGBA32[0]));
        ret.m_stRGBA32[1] = static_cast<uint8_t>(std::min<int>(255, a.m_stRGBA32[1] + b.m_stRGBA32[1]));
        ret.m_stRGBA32[2] = static_cast<uint8_t>(std::min<int>(255, a.m_stRGBA32[2] + b.m_stRGBA32[2]));
        ret.m_stRGBA32[3] = static_cast<uint8_t>(std::min<int>(255, a.m_stRGBA32[3] + b.m_stRGBA32[3]));
        return ret;
    }

    inline RGBA32Color operator-(RGBA32Color a, RGBA32Color b) noexcept
    {
        RGBA32Color ret {};
        ret.m_stRGBA32[0] = static_cast<uint8_t>(std::max<int>(0, a.m_stRGBA32[0] - b.m_stRGBA32[0]));
        ret.m_stRGBA32[1] = static_cast<uint8_t>(std::max<int>(0, a.m_stRGBA32[1] - b.m_stRGBA32[1]));
        ret.m_stRGBA32[2] = static_cast<uint8_t>(std::max<int>(0, a.m_stRGBA32[2] - b.m_stRGBA32[2]));
        ret.m_stRGBA32[3] = static_cast<uint8_t>(std::max<int>(0, a.m_stRGBA32[3] - b.m_stRGBA32[3]));
        return ret;
    }

    inline RGBA32Color operator*(RGBA32Color a, RGBA32Color b) noexcept
    {
        RGBA32Color ret {};
        ret.m_stRGBA32[0] = static_cast<uint8_t>(std::clamp<double>(a.m_stRGBA32[0] * (b.m_stRGBA32[0] / 255.), 0, 255));
        ret.m_stRGBA32[1] = static_cast<uint8_t>(std::clamp<double>(a.m_stRGBA32[1] * (b.m_stRGBA32[1] / 255.), 0, 255));
        ret.m_stRGBA32[2] = static_cast<uint8_t>(std::clamp<double>(a.m_stRGBA32[2] * (b.m_stRGBA32[2] / 255.), 0, 255));
        ret.m_stRGBA32[3] = static_cast<uint8_t>(std::clamp<double>(a.m_stRGBA32[3] * (b.m_stRGBA32[3] / 255.), 0, 255));
        return ret;
    }

    inline RGBA32Color operator*(double f, RGBA32Color c) noexcept
    {
        double r = f * c.m_stRGBA32[0];
        double g = f * c.m_stRGBA32[1];
        double b = f * c.m_stRGBA32[2];
        double a = f * c.m_stRGBA32[3];

        RGBA32Color ret {};
        ret.m_stRGBA32[0] = static_cast<uint8_t>(std::clamp<double>(r, 0, 255));
        ret.m_stRGBA32[1] = static_cast<uint8_t>(std::clamp<double>(g, 0, 255));
        ret.m_stRGBA32[2] = static_cast<uint8_t>(std::clamp<double>(b, 0, 255));
        ret.m_stRGBA32[3] = static_cast<uint8_t>(std::clamp<double>(a, 0, 255));
        return ret;
    }

    inline RGBA32Color operator*(RGBA32Color c, double f) noexcept
    {
        return f * c;
    }
}
