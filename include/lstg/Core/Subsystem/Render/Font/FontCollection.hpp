/**
 * @file
 * @date 2022/6/27
 * @author 9chu
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#pragma once
#include <vector>
#include "../../../LRUCache.hpp"
#include "IFontFace.hpp"

namespace lstg::Subsystem::Render::Font
{
    static const size_t kCharFontMappingCacheSize = 1024;

    /**
     * 字体集合
     */
    class FontCollection
    {
    public:
        FontCollection(FontFacePtr primaryFont) noexcept;
        FontCollection(const FontCollection& org) = default;
        FontCollection(FontCollection&& org) noexcept = default;

        FontCollection& operator=(const FontCollection& rhs) = default;
        FontCollection& operator=(FontCollection&& rhs) noexcept = default;

    public:
        /**
         * 为指定的字符选定字体
         * @param ch 字符
         * @return <字体指针, 字形ID, 缩放>
         */
        std::tuple<FontFacePtr, FontGlyphId, float> ChooseFontForChar(char32_t ch) const noexcept;

        /**
         * 添加备选字体
         * 先添加的字体会先被选中。
         * @param face 字体
         * @param scale 缩放，用于调整不同字体的大小（相对于主字体）
         */
        void AppendFallbackFont(FontFacePtr face, float scale = 1.f);

    private:
        FontFacePtr m_pPrimaryFont;
        std::vector<std::tuple<FontFacePtr, float>> m_stFallbackFont;
        mutable LRUCache<char32_t, std::tuple<FontFacePtr, FontGlyphId, float>, kCharFontMappingCacheSize> m_pCharToFontCache;
    };

    using FontCollectionPtr = std::shared_ptr<FontCollection>;
}
