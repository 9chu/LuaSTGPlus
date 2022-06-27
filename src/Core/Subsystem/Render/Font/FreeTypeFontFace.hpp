/**
 * @file
 * @date 2022/6/26
 * @author 9chu
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#pragma once
#include <vector>
#include <freetype/freetype.h>
#include <lstg/Core/LRUCache.hpp>
#include <lstg/Core/Subsystem/Render/Font/IFontFace.hpp>
#include "detail/CommonDefines.hpp"
#include "detail/FreeTypeStream.hpp"

namespace lstg::Subsystem::Render::Font
{
    /**
     * FreeType 字体
     */
    class FreeTypeFontFace :
        public IFontFace
    {
        struct CachedGlyphData
        {
            FT_Short Height = 0;
            FT_Glyph_Metrics GlyphMetrics;
            FT_Size_Metrics SizeMetrics;
            std::vector<FT_Vector> OutlinePoints;
        };

        using GlyphCacheContainer = LRUCache<FontGlyphId, CachedGlyphData, detail::kGlyphCacheCount>;
        using KerningCacheContainer = LRUCache<FontGlyphPair, FT_Vector, detail::kKerningCacheCount>;
        using AdvanceCacheContainer = LRUCache<FontGlyphId, FT_Fixed, detail::kAdvanceCacheCount>;

    public:
        FreeTypeFontFace(detail::FreeTypeStreamPtr stream, FT_Face face);
        ~FreeTypeFontFace();

        // Non-copyable
        FreeTypeFontFace(const FreeTypeFontFace&) = delete;
        FreeTypeFontFace& operator=(const FreeTypeFontFace&) = delete;

    public:  // IFontFace
        uint32_t GetFaceIndex() const noexcept override;
        uint32_t GetUnitsPerEm() const noexcept override;
        bool HasKerning() const noexcept override;
        uint32_t MakeGlyphRasterFlags() const noexcept override;
        Result<std::tuple<uint32_t, uint32_t>> GetUnitsPerEmScaled(FontSize size) noexcept override;
        size_t BatchGetNominalGlyphs(Span<FontGlyphId, true> glyphsOutput, Span<const char32_t, true> codePoints,
            bool fallback) noexcept override;
        std::optional<FontGlyphId> GetVariationGlyph(char32_t codePoints, uint32_t variationSelector) noexcept override;
        Result<void> GetGlyphName(Span<char> nameOutput, FontGlyphId glyphId) noexcept override;
        std::optional<FontGlyphId> GetGlyphByName(std::string_view name) noexcept override;
        Result<FontFaceMetrics> GetLayoutMetrics(FontGlyphRasterParam param, FontLayoutDirection direction) noexcept override;
        void BatchGetAdvances(Span<Q26D6, true> advancesOutput, FontGlyphRasterParam param, FontLayoutDirection direction,
            Span<const uint32_t, true> glyphs) noexcept override;
        Result<std::tuple<Q26D6, Q26D6>> GetGlyphOrigin(FontGlyphRasterParam param, FontLayoutDirection direction,
            FontGlyphId glyphId) noexcept override;
        Result<Q26D6> GetGlyphKerning(FontGlyphRasterParam param, FontLayoutDirection direction, FontGlyphPair pair) noexcept override;
        Result<FontGlyphMetrics> GetGlyphMetrics(FontGlyphRasterParam param, FontGlyphId glyphId) noexcept override;
        Result<std::tuple<Q26D6, Q26D6>> GetGlyphOutlinePoint(FontGlyphRasterParam param, FontGlyphId glyphId,
            size_t pointIndex) noexcept override;
        Result<SharedConstBlob> LoadSfntTable(uint32_t tag) noexcept override;

    private:
        Result<void> ApplySizeAndScale(FontSize size) noexcept;
        Result<void> LoadGlyph(FontGlyphRasterParam param, FontGlyphId id) noexcept;

        // Cache access
        Result<const CachedGlyphData*> FindOrCacheGlyph(FontGlyphRasterParam param, FontGlyphId id) noexcept;
        Result<FT_Vector> FindOrCacheKerning(FontGlyphRasterParam param, FontGlyphPair pair) noexcept;
        Result<FT_Fixed> FindOrCacheAdvance(FontGlyphRasterParam param, FontGlyphId id) noexcept;

    private:
        // 字体
        detail::FreeTypeStreamPtr m_pStream;
        FT_Face m_pFace = nullptr;

        // 缓存
        // FIXME: 展平 FontGlyphRasterParam+X 可以节约内存
        // FIXME: 当瞬间使用的不同大小字体数量超过 kFontSizeCache 时性能会很糟糕
        LRUCache<FontGlyphRasterParam, GlyphCacheContainer, detail::kFontSizeCacheSize> m_stGlyphCache;  // 字形缓存
        LRUCache<FontGlyphRasterParam, KerningCacheContainer, detail::kFontSizeCacheSize> m_stKerningCache;  // 字符间距调整量缓存
        LRUCache<FontGlyphRasterParam, AdvanceCacheContainer, detail::kFontSizeCacheSize> m_stAdvanceCache;  // 字符前进量缓存
    };
}
