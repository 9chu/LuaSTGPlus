/**
 * @file
 * @date 2022/6/29
 * @author 9chu
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#pragma once
#include <cstdint>
#include "FontSize.hpp"

namespace lstg::Subsystem::Render::Font
{
    /**
     * 字形内部ID
     */
    using FontGlyphId = uint32_t;

    /**
     * 字形光栅化参数
     */
    struct FontGlyphRasterParam
    {
        FontSize Size;  ///< @brief 字体大小
        uint32_t Flags = 0;  ///< @brief 光栅化参数，透传、内部使用，例如用于存储 FT_LOAD_*

        bool operator==(const FontGlyphRasterParam& rhs) const noexcept
        {
            return Size == rhs.Size && Flags == rhs.Flags;
        }

        bool operator!=(const FontGlyphRasterParam& rhs) const noexcept
        {
            return !operator==(rhs);
        }
    };
}

template <>
struct std::hash<lstg::Subsystem::Render::Font::FontGlyphRasterParam>
{
    std::size_t operator()(const lstg::Subsystem::Render::Font::FontGlyphRasterParam& s) const noexcept
    {
        auto h1 = std::hash<lstg::Subsystem::Render::Font::FontSize>{}(s.Size);
        auto h2 = std::hash<uint32_t>{}(s.Flags);
        return h1 ^ h2;
    }
};
