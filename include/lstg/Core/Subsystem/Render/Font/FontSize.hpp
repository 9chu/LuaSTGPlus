/**
 * @file
 * @date 2022/6/26
 * @author 9chu
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#pragma once
#include <algorithm>

namespace lstg::Subsystem::Render::Font
{
    /**
     * 带缩放字体大小
     */
    struct FontSize
    {
        int32_t Size = 0;  // 字体大小，DPI变换前
        float Scale = 0.f;  // 额外的缩放量

        bool operator==(const FontSize& rhs) const noexcept
        {
            return Size == rhs.Size && Scale == rhs.Scale;
        }

        bool operator!=(const FontSize& rhs) const noexcept
        {
            return !operator==(rhs);
        }
    };
}

template <>
struct std::hash<lstg::Subsystem::Render::Font::FontSize>
{
    std::size_t operator()(const lstg::Subsystem::Render::Font::FontSize& s) const noexcept
    {
        auto h1 = std::hash<int32_t>{}(s.Size);
        auto h2 = std::hash<float>{}(s.Scale);
        return h1 ^ h2;
    }
};
