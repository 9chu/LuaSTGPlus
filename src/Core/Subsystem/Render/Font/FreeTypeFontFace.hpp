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
#include "detail/FreeTypeObject.hpp"

template <>
struct std::hash<std::tuple<lstg::Subsystem::Render::Font::FontGlyphRasterParam, lstg::Subsystem::Render::Font::FontGlyphId>>
{
    std::size_t operator()(const std::tuple<lstg::Subsystem::Render::Font::FontGlyphRasterParam,
        lstg::Subsystem::Render::Font::FontGlyphId>& s) const noexcept
    {
        using namespace lstg::Subsystem::Render::Font;

        auto h1 = std::hash<FontGlyphRasterParam>{}(std::get<0>(s));
        auto h2 = std::hash<FontGlyphId>{}(std::get<1>(s));
        return h1 ^ h2;
    }
};

template <>
struct std::hash<std::tuple<lstg::Subsystem::Render::Font::FontGlyphRasterParam, lstg::Subsystem::Render::Font::FontGlyphPair>>
{
    std::size_t operator()(const std::tuple<lstg::Subsystem::Render::Font::FontGlyphRasterParam,
        lstg::Subsystem::Render::Font::FontGlyphPair>& s) const noexcept
    {
        using namespace lstg::Subsystem::Render::Font;

        auto h1 = std::hash<FontGlyphRasterParam>{}(std::get<0>(s));
        auto h2 = std::hash<FontGlyphPair>{}(std::get<1>(s));
        return h1 ^ h2;
    }
};

namespace lstg::Subsystem::Render::Font
{
    /**
     * FreeType 字体
     */
    class FreeTypeFontFace :
        public IFontFace,
        public std::enable_shared_from_this<FreeTypeFontFace>
    {
        struct CachedGlyphData
        {
            FT_Short Height = 0;
            FT_Glyph_Metrics GlyphMetrics;
            FT_Size_Metrics SizeMetrics;
            std::vector<FT_Vector> OutlinePoints;
        };

        using GlyphCacheContainer = LRUCache<std::tuple<FontGlyphRasterParam, FontGlyphId>, CachedGlyphData, detail::kGlyphCacheCount>;
        using KerningCacheContainer = LRUCache<std::tuple<FontGlyphRasterParam, FontGlyphPair>, FT_Vector, detail::kKerningCacheCount>;
        using AdvanceCacheContainer = LRUCache<std::tuple<FontGlyphRasterParam, FontGlyphId>, FT_Fixed, detail::kAdvanceCacheCount>;

    public:
        FreeTypeFontFace(detail::FreeTypeObject::LibraryPtr library, detail::FreeTypeStreamPtr stream,
            detail::FreeTypeObject::FacePtr face);

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
        Result<FontGlyphAtlasInfo> GetGlyphAtlas(FontGlyphRasterParam param, FontGlyphId glyphId,
            DynamicFontGlyphAtlas* dynamicAtlas) noexcept override;

    private:
        Result<void> ApplySizeAndScale(FontSize size) noexcept;
        Result<void> LoadGlyph(FontGlyphRasterParam param, FontGlyphId id) noexcept;

        // Cache access
        Result<const CachedGlyphData*> FindOrCacheGlyph(FontGlyphRasterParam param, FontGlyphId id) noexcept;
        Result<FT_Vector> FindOrCacheKerning(FontGlyphRasterParam param, FontGlyphPair pair) noexcept;
        Result<FT_Fixed> FindOrCacheAdvance(FontGlyphRasterParam param, FontGlyphId id) noexcept;

    private:
        // 字体
        detail::FreeTypeObject::LibraryPtr m_pLibrary;
        detail::FreeTypeStreamPtr m_pStream;
        detail::FreeTypeObject::FacePtr m_pFace;

        // 缓存
        GlyphCacheContainer m_stGlyphCache;  // 字形缓存
        KerningCacheContainer m_stKerningCache;  // 字符间距调整量缓存
        AdvanceCacheContainer m_stAdvanceCache;  // 字符前进量缓存
    };
}
