/**
 * @file
 * @date 2022/7/3
 * @author 9chu
 * 此文件为 LuaSTGPlus 项目的一部分，版权与许可声明详见 COPYRIGHT.txt。
 */
#include "HgeFontFace.hpp"

#include "detail/CommonDefines.hpp"
#include "detail/Helper.hpp"

using namespace std;
using namespace lstg;
using namespace lstg::Subsystem::Render::Font;

static const uint32_t kFixedUnitsPerEm = 2048;

Subsystem::Render::TexturePtr HgeFontFace::GetAtlasTexture() const noexcept
{
    return m_pImageTexture;
}

void HgeFontFace::SetAtlasTexture(TexturePtr tex) noexcept
{
    auto texWidth = static_cast<float>(tex ? tex->GetWidth() : 0);
    auto texHeight = static_cast<float>(tex ? tex->GetHeight() : 0);
    for (auto& glyph : m_stGlyphs)
        glyph.RefreshUV(texWidth, texHeight);

    m_pImageTexture = std::move(tex);
}

Result<void> HgeFontFace::AppendGlyph(char32_t ch, float x, float y, float w, float h, float leftOffset, float rightOffset) noexcept
{
    auto texWidth = static_cast<float>(m_pImageTexture ? m_pImageTexture->GetWidth() : 0);
    auto texHeight = static_cast<float>(m_pImageTexture ? m_pImageTexture->GetHeight() : 0);

    GlyphInfo info;
    info.CodePoint = ch;
    info.Rectangle = { x, y, w, h };
    info.DrawOffset = { leftOffset, h };
    info.Advance = leftOffset + w + rightOffset;
    info.UVRectangle = {};
    info.RefreshUV(texWidth, texHeight);

    m_fMaxHeight = std::max(m_fMaxHeight, h);

    try
    {
        m_stGlyphs.push_back(info);
    }
    catch (...)  // bad_alloc
    {
        return make_error_code(errc::not_enough_memory);
    }

    try
    {
        assert(m_stGlyphs.size() != 0);
        m_stCodePoint2GlyphId.emplace(static_cast<char32_t>(info.CodePoint), static_cast<FontGlyphId>(m_stGlyphs.size()));
    }
    catch (...)  // bad_alloc
    {
        m_stGlyphs.pop_back();
        return make_error_code(errc::not_enough_memory);
    }
    return {};
}

uint32_t HgeFontFace::GetFaceIndex() const noexcept
{
    return 0;
}

uint32_t HgeFontFace::GetUnitsPerEm() const noexcept
{
    // 纹理化字体没有可变大小，总是以 2048 为单位
    // 为什么选择 2048? 取了微软文档中的样例值，一般字体也在这个取值量级。之所以不用 1，是为了给后面缩放留余量。
    // https://docs.microsoft.com/en-us/typography/opentype/spec/ttch01
    return kFixedUnitsPerEm;
}

bool HgeFontFace::HasKerning() const noexcept
{
    return false;
}

uint32_t HgeFontFace::MakeGlyphRasterFlags() const noexcept
{
    return 0;
}

Result<std::tuple<uint32_t, uint32_t>> HgeFontFace::GetUnitsPerEmScaled(FontSize size) noexcept
{
    auto scaleX = static_cast<uint32_t>(kFixedUnitsPerEm * size.Scale);
    auto scaleY = static_cast<uint32_t>(kFixedUnitsPerEm * size.Scale);
    return tuple<uint32_t, uint32_t> { scaleX, scaleY };
}

size_t HgeFontFace::BatchGetNominalGlyphs(Span<FontGlyphId, true> glyphsOutput, Span<const char32_t, true> codePoints,
    bool fallback) noexcept
{
    assert(glyphsOutput.GetSize() == codePoints.GetSize());
    size_t cnt = 0;
    auto it = glyphsOutput.begin();
    for (auto jt = codePoints.begin(); jt != codePoints.end(); ++jt)
    {
        char32_t ch = *jt;
        auto glyph = m_stCodePoint2GlyphId.find(ch);
        if (glyph == m_stCodePoint2GlyphId.end() && fallback)
        {
            auto fallbackCodePoint = detail::Helper::IsRenderAsWhitespace(ch) ? ' ' : detail::kInvalidCharCodePoint;
            glyph = m_stCodePoint2GlyphId.find(fallbackCodePoint);
        }

        if (glyph != m_stCodePoint2GlyphId.end())
            *it = glyph->second;
        else
            break;

        ++cnt;
        ++it;
    }
    return cnt;
}

std::optional<FontGlyphId> HgeFontFace::GetVariationGlyph(char32_t codePoint, uint32_t variationSelector) noexcept
{
    return {};
}

Result<void> HgeFontFace::GetGlyphName(Span<char> nameOutput, FontGlyphId glyphId) noexcept
{
    return make_error_code(errc::not_supported);
}

std::optional<FontGlyphId> HgeFontFace::GetGlyphByName(std::string_view name) noexcept
{
    return {};
}

Result<FontFaceMetrics> HgeFontFace::GetLayoutMetrics(FontGlyphRasterParam param, FontLayoutDirection direction) noexcept
{
    FontFaceMetrics ret;
    ret.Ascender = PixelToQ26D6(m_fMaxHeight * param.Size.Scale);
    ret.Descender = ret.LineGap = 0;
    return ret;
}

void HgeFontFace::BatchGetAdvances(Span<Q26D6, true> advancesOutput, FontGlyphRasterParam param, FontLayoutDirection direction,
    Span<const uint32_t, true> glyphs) noexcept
{
    assert(advancesOutput.size() == glyphs.size());
    auto it = advancesOutput.begin();
    for (auto jt = glyphs.begin(); jt != glyphs.end(); ++jt)
    {
        auto glyphId = *jt;
        Q26D6 advance = 0;
        if (glyphId != 0 && glyphId <= m_stGlyphs.size())
        {
            auto& glyph = m_stGlyphs[glyphId - 1];
            advance = PixelToQ26D6((direction == FontLayoutDirection::Horizontal ? glyph.Advance : glyph.Rectangle.Height()) *
                param.Size.Scale);
        }
        *it = advance;
        ++it;
    }
}

Result<std::tuple<Q26D6, Q26D6>> HgeFontFace::GetGlyphOrigin(FontGlyphRasterParam param, FontLayoutDirection direction,
    FontGlyphId glyphId) noexcept
{
    return tuple<Q26D6, Q26D6> {0, 0};
}

Result<Q26D6> HgeFontFace::GetGlyphKerning(FontGlyphRasterParam param, FontLayoutDirection direction, FontGlyphPair pair) noexcept
{
    return Q26D6(0);
}

Result<FontGlyphMetrics> HgeFontFace::GetGlyphMetrics(FontGlyphRasterParam param, FontGlyphId glyphId) noexcept
{
    if (glyphId == 0 || glyphId > m_stGlyphs.size())
        return make_error_code(errc::invalid_argument);

    auto& glyph = m_stGlyphs[glyphId - 1];
    FontGlyphMetrics metrics;
    metrics.Width = PixelToQ26D6(glyph.Rectangle.Width() * param.Size.Scale);
    metrics.Height = PixelToQ26D6(glyph.Rectangle.Height() * param.Size.Scale);
    metrics.XBearing = PixelToQ26D6(glyph.DrawOffset.x * param.Size.Scale);
    metrics.YBearing = PixelToQ26D6(glyph.Rectangle.Height() * param.Size.Scale);
    return metrics;
}

Result<std::tuple<Q26D6, Q26D6>> HgeFontFace::GetGlyphOutlinePoint(FontGlyphRasterParam param, FontGlyphId glyphId, size_t pointIndex)
{
    return make_error_code(errc::not_supported);
}

Result<SharedConstBlob> HgeFontFace::LoadSfntTable(uint32_t tag) noexcept
{
    return make_error_code(errc::not_supported);
}

Result<FontGlyphAtlasInfo> HgeFontFace::GetGlyphAtlas(FontGlyphRasterParam param, FontGlyphId glyphId,
    DynamicFontGlyphAtlas* dynamicAtlas) noexcept
{
    if (glyphId == 0 || glyphId > m_stGlyphs.size())
        return make_error_code(errc::invalid_argument);

    auto& glyph = m_stGlyphs[glyphId - 1];

    FontGlyphAtlasInfo atlasInfo;
    atlasInfo.Texture = m_pImageTexture;
    atlasInfo.TextureRect = glyph.UVRectangle;
    atlasInfo.DrawOffset = glyph.DrawOffset * param.Size.Scale;
    atlasInfo.DrawSize = { glyph.Rectangle.Width() * param.Size.Scale, glyph.Rectangle.Height() * param.Size.Scale };
    return atlasInfo;
}
