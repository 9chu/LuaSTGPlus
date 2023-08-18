/**
 * @file
 * @date 2022/6/26
 * @author 9chu
 * 此文件为 LuaSTGPlus 项目的一部分，版权与许可声明详见 COPYRIGHT.txt。
 */
#include "FreeTypeFontFace.hpp"

#include <cassert>
#include <freetype/ftadvanc.h>
#include <freetype/tttables.h>
#include <lstg/Core/Logging.hpp>
#include "detail/Helper.hpp"
#include "detail/FreeTypeError.hpp"

using namespace std;
using namespace lstg;
using namespace lstg::Subsystem::Render::Font;

LSTG_DEF_LOG_CATEGORY(FreeTypeFontFace);

FreeTypeFontFace::FreeTypeFontFace(detail::FreeTypeObject::LibraryPtr library, detail::FreeTypeStreamPtr stream,
    detail::FreeTypeObject::FacePtr face)
    : m_pLibrary(std::move(library)), m_pStream(std::move(stream)), m_pFace(std::move(face))
{
    assert(m_pLibrary);
    assert(m_pFace);
    assert(m_pStream);
}

uint32_t FreeTypeFontFace::GetFaceIndex() const noexcept
{
    return m_pFace->face_index;
}

uint32_t FreeTypeFontFace::GetUnitsPerEm() const noexcept
{
    return m_pFace->units_per_EM;
}

bool FreeTypeFontFace::HasKerning() const noexcept
{
    return FT_HAS_KERNING(m_pFace);
}

uint32_t FreeTypeFontFace::MakeGlyphRasterFlags() const noexcept
{
    auto ret = FT_LOAD_COLOR;
    if (FT_IS_SCALABLE(m_pFace))
        ret |= FT_LOAD_NO_BITMAP;
    ret |= FT_LOAD_FORCE_AUTOHINT;  // 自动抗锯齿
    return ret;
}

Result<std::tuple<uint32_t, uint32_t>> FreeTypeFontFace::GetUnitsPerEmScaled(FontSize size) noexcept
{
    auto ret = ApplySizeAndScale(size);
    if (!ret)
        return ret.GetError();

    uint32_t scaleX = ::FT_MulFix(m_pFace->units_per_EM, m_pFace->size->metrics.x_scale);
    uint32_t scaleY = ::FT_MulFix(m_pFace->units_per_EM, m_pFace->size->metrics.y_scale);
    return tuple<uint32_t, uint32_t> { scaleX, scaleY };
}

size_t FreeTypeFontFace::BatchGetNominalGlyphs(Span<FontGlyphId, true> glyphsOutput, Span<const char32_t, true> codepoints,
    bool fallback) noexcept
{
    assert(glyphsOutput.GetSize() == codepoints.GetSize());
    auto symbolCharMap = m_pFace->charmap && m_pFace->charmap->encoding == FT_ENCODING_MS_SYMBOL;
    auto it = codepoints.Begin();
    auto jt = glyphsOutput.Begin();
    while (it != codepoints.End())
    {
        auto ch = *it;
        auto glyphIndex = ::FT_Get_Char_Index(m_pFace.get(), ch);

        // 特殊处理符号字体
        if (symbolCharMap && ch != '\0' && glyphIndex == 0 && ch <= 0x00FFu)
        {
            /**
             * 对于符号编码的 OpenType 字体，将 U+0000..00FF 复制到 U+F000..F0FF，以符合 Windows 的做法。
             * https://docs.microsoft.com/en-us/typography/opentype/spec/recom
             */
            glyphIndex = ::FT_Get_Char_Index(m_pFace.get(), 0xF000u + ch);
        }

        // 尝试 fallback 到空白或者 InvalidChar
        if (fallback && ch != '\0' && glyphIndex == 0)
        {
            auto fallbackCodePoint = detail::Helper::IsRenderAsWhitespace(ch) ? ' ' : detail::kInvalidCharCodePoint;
            glyphIndex = ::FT_Get_Char_Index(m_pFace.get(), fallbackCodePoint);
        }

        // 失败的时候返回处理了多少个字形
        if (ch != '\0' && glyphIndex == 0)
            return it - codepoints.Begin();

        *jt = glyphIndex;

        ++it;
        ++jt;
    }
    return codepoints.GetSize();
}

std::optional<FontGlyphId> FreeTypeFontFace::GetVariationGlyph(char32_t codepoint, uint32_t variationSelector) noexcept
{
    auto glyphIndex = ::FT_Face_GetCharVariantIndex(m_pFace.get(), codepoint, variationSelector);
    if (glyphIndex == 0)
        return {};
    return glyphIndex;
}

Result<void> FreeTypeFontFace::GetGlyphName(Span<char> nameOutput, FontGlyphId glyphId) noexcept
{
    auto ret = ::FT_Get_Glyph_Name(m_pFace.get(), glyphId, nameOutput.data(), nameOutput.size());
    if (ret != 0)
        return make_error_code(static_cast<detail::FreeTypeError>(ret));
    if (!nameOutput.IsEmpty() && nameOutput[0] == '\0')
        return make_error_code(static_cast<detail::FreeTypeError>(FT_Err_Invalid_Glyph_Index));
    return {};
}

std::optional<FontGlyphId> FreeTypeFontFace::GetGlyphByName(std::string_view name) noexcept
{
    char buf[128];
    auto len = std::min(sizeof(buf) - 1, name.length());
    ::strncpy(buf, name.data(), len);
    buf[len] = '\0';
    auto glyph = ::FT_Get_Name_Index(m_pFace.get(), buf);
    if (glyph == 0)
    {
        // 检查是否确实是 '\0' 的名称
        if (0 == ::FT_Get_Glyph_Name(m_pFace.get(), 0, buf, sizeof(buf)) && name == buf)
            return glyph;
        return {};
    }
    return glyph;
}

Result<FontFaceMetrics> FreeTypeFontFace::GetLayoutMetrics(FontGlyphRasterParam param, FontLayoutDirection direction) noexcept
{
    auto ret = ApplySizeAndScale(param.Size);
    if (!ret)
        return ret.GetError();

    FontFaceMetrics metrics = {};
    metrics.Ascender = static_cast<Q26D6>(::FT_MulFix(m_pFace->ascender, m_pFace->size->metrics.y_scale));
    metrics.Descender = static_cast<Q26D6>(::FT_MulFix(m_pFace->descender, m_pFace->size->metrics.y_scale));
    metrics.LineGap = static_cast<Q26D6>(::FT_MulFix(m_pFace->height, m_pFace->size->metrics.y_scale) -
        (metrics.Ascender - metrics.Descender));
    return metrics;
}

void FreeTypeFontFace::BatchGetAdvances(Span<Q26D6, true> advancesOutput, FontGlyphRasterParam param, FontLayoutDirection direction,
    Span<const uint32_t, true> glyphs) noexcept
{
    assert(advancesOutput.size() == glyphs.size());

    if (direction == FontLayoutDirection::Horizontal)
        param.Flags &= ~FT_LOAD_VERTICAL_LAYOUT;
    else
        param.Flags |= FT_LOAD_VERTICAL_LAYOUT;

    int flipMultiplier = 1;
    if (direction == FontLayoutDirection::Vertical)
        flipMultiplier = -1;  // 乘以 -1：FreeType 坐标轴 Y 向下，修正成 Y 向上

    auto it = glyphs.Begin();
    auto jt = advancesOutput.Begin();
    while (it != glyphs.End())
    {
        auto adv = FindOrCacheAdvance(param, *it);
        if (adv)
        {
            // Advance 存储 Q16.16，需要转换到 Q26.6
            *jt = static_cast<Q26D6>(((flipMultiplier * *adv) + (1 << 9)) >> 10);
        }
        else
        {
            *jt = 0;
        }

        ++it;
        ++jt;
    }
}

Result<std::tuple<Q26D6, Q26D6>> FreeTypeFontFace::GetGlyphOrigin(FontGlyphRasterParam param, FontLayoutDirection direction,
    FontGlyphId glyphId) noexcept
{
    if (direction == FontLayoutDirection::Horizontal)
        return std::tuple<Q26D6, Q26D6> { 0, 0 };

    auto ret = FindOrCacheGlyph(param, glyphId);
    if (!ret)
        return ret.GetError();

    auto& cache = *ret;
    // FIXME: from hb-ft.cc, why?
    auto x = cache->GlyphMetrics.horiBearingX - cache->GlyphMetrics.vertBearingX;
    auto y = cache->GlyphMetrics.horiBearingY - (-cache->GlyphMetrics.vertBearingY);  // 修正坐标轴，使得 Y 向上
    return std::tuple<Q26D6, Q26D6> { static_cast<Q26D6>(x), static_cast<Q26D6>(y) };
}

Result<Q26D6> FreeTypeFontFace::GetGlyphKerning(FontGlyphRasterParam param, FontLayoutDirection direction, FontGlyphPair pair) noexcept
{
    if (direction == FontLayoutDirection::Vertical)
        return 0;  // FreeType 不支持纵向 Kerning

    assert(direction == FontLayoutDirection::Horizontal);
    auto ret = FindOrCacheKerning(param, pair);
    if (!ret)
        return ret.GetError();
    return static_cast<Q26D6>(ret->x);
}

Result<FontGlyphMetrics> FreeTypeFontFace::GetGlyphMetrics(FontGlyphRasterParam param, FontGlyphId glyphId) noexcept
{
    auto ret = FindOrCacheGlyph(param, glyphId);
    if (!ret)
        return ret.GetError();

    auto& cache = *ret;

    FontGlyphMetrics metrics {};
    metrics.Width = static_cast<Q26D6>(cache->GlyphMetrics.width);
    metrics.Height = -static_cast<Q26D6>(cache->GlyphMetrics.height);  // 乘以 -1：FreeType 坐标轴 Y 向下，修正成 Y 向上
    metrics.XBearing = static_cast<Q26D6>(cache->GlyphMetrics.horiBearingX);
    metrics.YBearing = static_cast<Q26D6>(cache->GlyphMetrics.horiBearingY);
    return metrics;
}

Result<std::tuple<Q26D6, Q26D6>> FreeTypeFontFace::GetGlyphOutlinePoint(FontGlyphRasterParam param, FontGlyphId glyphId,
    size_t pointIndex) noexcept
{
    auto ret = FindOrCacheGlyph(param, glyphId);
    if (!ret)
        return ret.GetError();

    auto& cache = *ret;

    if (pointIndex >= cache->OutlinePoints.size())
        return make_error_code(static_cast<detail::FreeTypeError>(FT_Err_Bad_Argument));

    auto x = cache->OutlinePoints[pointIndex].x;
    auto y = cache->OutlinePoints[pointIndex].y;
    return std::tuple<Q26D6, Q26D6> {static_cast<Q26D6>(x), static_cast<Q26D6>(y)};
}

Result<SharedConstBlob> FreeTypeFontFace::LoadSfntTable(uint32_t tag) noexcept
{
    FT_ULong length = 0;
    auto err = ::FT_Load_Sfnt_Table(m_pFace.get(), tag, 0, nullptr, &length);
    if (err != 0)
        return make_error_code(static_cast<detail::FreeTypeError>(err));

    shared_ptr<vector<uint8_t>> ret;
    try
    {
        ret = make_shared<vector<uint8_t>>(length);
    }
    catch (...)
    {
        return make_error_code(errc::not_enough_memory);
    }

    err = ::FT_Load_Sfnt_Table(m_pFace.get(), tag, 0, reinterpret_cast<FT_Byte*>(ret->data()), &length);
    if (err != 0)
        return make_error_code(static_cast<detail::FreeTypeError>(err));
    return static_pointer_cast<const vector<uint8_t>>(ret);
}

Result<FontGlyphAtlasInfo> FreeTypeFontFace::GetGlyphAtlas(FontGlyphRasterParam param, FontGlyphId glyphId,
    DynamicFontGlyphAtlas* dynamicAtlas) noexcept
{
    // 查缓存
    FontGlyphAtlasInfo info;
    if (dynamicAtlas->FindGlyph(info, this, param, glyphId))
        return info;

    FontFacePtr self;
    try
    {
        self = static_pointer_cast<IFontFace>(shared_from_this());
    }
    catch (...)
    {
        return make_error_code(std::errc::not_supported);
    }
    assert(self.get() == this);

    // 渲染文字
    auto ret = LoadGlyph(param, glyphId);
    if (!ret)
        return ret.GetError();

    auto err = ::FT_Render_Glyph(m_pFace->glyph, FT_RENDER_MODE_NORMAL);
    if (err != 0)
        return make_error_code(static_cast<detail::FreeTypeError>(err));

    // 转换格式
    bool isGrayscale = true;
    FT_Bitmap* finalBitmap = &m_pFace->glyph->bitmap;
    detail::FreeTypeObject::Bitmap tmpBitmap(m_pLibrary);
    if (m_pFace->glyph->bitmap.pixel_mode == FT_PIXEL_MODE_BGRA)
    {
        isGrayscale = false;
    }
    else if (m_pFace->glyph->bitmap.pixel_mode != FT_PIXEL_MODE_GRAY)
    {
        // 转换到灰度图
        auto ret = tmpBitmap.ConvertFrom(finalBitmap, 4);
        if (!ret)
            return ret.GetError();
        finalBitmap = *tmpBitmap;
    }
    assert((isGrayscale && finalBitmap->pixel_mode == FT_PIXEL_MODE_GRAY) ||
        (!isGrayscale && finalBitmap->pixel_mode == FT_PIXEL_MODE_BGRA));

    // 缓存
    BitmapSource source;
    source.Buffer = finalBitmap->buffer;
    source.Width = finalBitmap->width;
    source.Height = finalBitmap->rows;
    source.Stride = finalBitmap->pitch;
    source.DrawLeftOffset = m_pFace->glyph->bitmap_left;
    source.DrawTopOffset = m_pFace->glyph->bitmap_top;
    if (isGrayscale)
    {
        auto ret = dynamicAtlas->CacheGlyphFromGrayscale(self, param, glyphId, source, finalBitmap->num_grays);
        if (!ret)
            return ret.GetError();
        info = *ret;
    }
    else
    {
        auto ret = dynamicAtlas->CacheGlyphFromBGRA(self, param, glyphId, source);
        if (!ret)
            return ret.GetError();
        info = *ret;
    }
    return info;
}

Result<void> FreeTypeFontFace::ApplySizeAndScale(FontSize size) noexcept
{
    // 计算字体的像素大小
    Q26D6 requiredFontPixelSizeFixed = 0;
    {
        Q26D6 fontSizeFixed = PixelToQ26D6(std::max(0, size.Size));
        Q16D16 fontScaleFixed = PixelToQ16D16(std::max<float>(0.f, size.Scale));

        // see: FT_REQUEST_WIDTH、FT_REQUEST_HEIGHT
        // 为什么要 + 36 / 72 ？如果在 DPI=72 的情况下，似乎等价于 +0.5
        requiredFontPixelSizeFixed = ((fontSizeFixed * detail::kFontRenderDPI) + 36) / 72;

        // 缩放量
        // NOTE: FreeType 用 signed long 表示定点数，由于 FT_Long 在各个平台大小不一样，实际上是 Q?D16，我们通过强制转换不需要额外处理小数位
        // 虽然乘数是一个 Q16.16，但是乘完后依旧是一个 Q26.6
        requiredFontPixelSizeFixed = static_cast<Q26D6>(::FT_MulFix(static_cast<FT_Long>(requiredFontPixelSizeFixed),
            static_cast<FT_Long>(fontScaleFixed)));
    }

    if (FT_IS_SCALABLE(m_pFace))
    {
        // 调用
        auto requiredFontPixelSize = Q26D6ToPixel(requiredFontPixelSizeFixed);

        auto ret = ::FT_Set_Pixel_Sizes(m_pFace.get(), requiredFontPixelSize, requiredFontPixelSize);
        if (ret != 0)
        {
            LSTG_LOG_ERROR_CAT(FreeTypeFontFace, "FT_Set_Pixel_Sizes({}) fail: {}", requiredFontPixelSize, ret);
            return make_error_code(static_cast<detail::FreeTypeError>(ret));
        }
    }
    else if (FT_HAS_FIXED_SIZES(m_pFace))
    {
        // 针对固定大小的字体，找到最合适的点阵图大小。基于高度来作为参照。
        auto bestStrikeIndex = -1;
        {
            Q26D6 currentBestFixedStrikeHeight = 0;
            for (auto i = 0; i < m_pFace->num_fixed_sizes; ++i)
            {
                auto height = m_pFace->available_sizes[i].y_ppem;

                // 完全匹配
                if (height == requiredFontPixelSizeFixed)
                {
                    currentBestFixedStrikeHeight = static_cast<Q26D6>(height);
                    bestStrikeIndex = i;
                    break;
                }

                if (bestStrikeIndex == -1)
                {
                    currentBestFixedStrikeHeight = static_cast<Q26D6>(height);
                    bestStrikeIndex = i;
                    continue;
                }

                // 总是选择一个比期望大小更大的最小高度的点图
                if (currentBestFixedStrikeHeight < requiredFontPixelSizeFixed)
                {
                    if (height > currentBestFixedStrikeHeight)
                    {
                        currentBestFixedStrikeHeight = static_cast<Q26D6>(height);
                        bestStrikeIndex = i;
                    }
                }
                else
                {
                    assert(currentBestFixedStrikeHeight > requiredFontPixelSizeFixed);
                    if (height < currentBestFixedStrikeHeight && height > requiredFontPixelSizeFixed)
                    {
                        currentBestFixedStrikeHeight = static_cast<Q26D6>(height);
                        bestStrikeIndex = i;
                    }
                }
            }
        }

        assert(bestStrikeIndex != -1);
        auto ret = ::FT_Select_Size(m_pFace.get(), bestStrikeIndex);
        if (ret != 0)
        {
            LSTG_LOG_ERROR_CAT(FreeTypeFontFace, "FT_Select_Size({}) fail: {}", bestStrikeIndex, ret);
            return make_error_code(static_cast<detail::FreeTypeError>(ret));
        }

        // 计算缩放量
        Q16D16 strikeScaleFixed = 0;
        {
            FT_Long requiredFontPixelSizeFixed16 = requiredFontPixelSizeFixed << 10;
            FT_Long bestStrikeHeightFixed16 = m_pFace->available_sizes[bestStrikeIndex].y_ppem << 10;
            strikeScaleFixed = static_cast<Q16D16>(::FT_DivFix(requiredFontPixelSizeFixed16, bestStrikeHeightFixed16));
        }

        // 固定大小字体不适用 x_scale 和 y_scale 字段，我们在这里复用下填充这个缩放值
        // 需要注意原则上不应该修改这个 metrics 字段，如果出现问题应该挪到当前的类中存储
        m_pFace->size->metrics.x_scale = strikeScaleFixed;
        m_pFace->size->metrics.y_scale = strikeScaleFixed;
    }
    else
    {
        assert(false);
        return make_error_code(errc::not_supported);
    }
    return {};
}

Result<void> FreeTypeFontFace::LoadGlyph(FontGlyphRasterParam param, FontGlyphId id) noexcept
{
    auto ret = ApplySizeAndScale(param.Size);
    if (!ret)
        return ret.GetError();

    auto err = ::FT_Load_Glyph(m_pFace.get(), id, static_cast<FT_Int32>(param.Flags));
    if (err != 0)
        return make_error_code(static_cast<detail::FreeTypeError>(err));
    return {};
}

Result<const FreeTypeFontFace::CachedGlyphData*> FreeTypeFontFace::FindOrCacheGlyph(FontGlyphRasterParam param, FontGlyphId id) noexcept
{
    // 获取缓存
    auto key = std::make_tuple(param, id);
    auto cache = m_stGlyphCache.TryGet(key);
    if (!cache)
    {
        // 没有命中缓存，加载字形
        auto ret = LoadGlyph(param, id);
        if (!ret)
            return ret.GetError();

        try
        {
            CachedGlyphData cachedData {};
            cachedData.Height = m_pFace->height;
            cachedData.GlyphMetrics = m_pFace->glyph->metrics;
            cachedData.SizeMetrics = m_pFace->size->metrics;
            if (m_pFace->glyph->outline.n_points > 0)
            {
                auto points = m_pFace->glyph->outline.n_points;
                cachedData.OutlinePoints.reserve(points);
                for (auto i = 0; i < points; ++i)
                    cachedData.OutlinePoints.push_back(m_pFace->glyph->outline.points[i]);
            }
            cache = m_stGlyphCache.Emplace(key, std::move(cachedData));
        }
        catch (...)  // bad_alloc
        {
            return make_error_code(errc::not_enough_memory);
        }
    }
    assert(cache);
    return cache;
}

Result<FT_Vector> FreeTypeFontFace::FindOrCacheKerning(FontGlyphRasterParam param, FontGlyphPair pair) noexcept
{
    // 获取缓存
    auto key = make_tuple(param, pair);
    auto cache = m_stKerningCache.TryGet(key);
    if (!cache)
    {
        // 没有命中缓存，加载 Kerning
        auto ret = ApplySizeAndScale(param.Size);
        if (!ret)
            return ret.GetError();

        FT_Vector kerning = { 0, 0 };
        auto err = ::FT_Get_Kerning(m_pFace.get(), pair.Left, pair.Right, FT_KERNING_DEFAULT, &kerning);
        if (err != 0)
            return make_error_code(static_cast<detail::FreeTypeError>(err));

        // 对于固定大小的字体，我们通过之前保存的 Scaling 对 Kerning 进行调整
        if (!FT_IS_SCALABLE(m_pFace) && FT_HAS_FIXED_SIZES(m_pFace))
        {
            kerning.x = ::FT_MulFix(kerning.x, m_pFace->size->metrics.x_scale);
            kerning.y = ::FT_MulFix(kerning.y, m_pFace->size->metrics.y_scale);
        }

        try
        {
            cache = m_stKerningCache.Emplace(key, kerning);
        }
        catch (...)  // bad_alloc
        {
            return make_error_code(errc::not_enough_memory);
        }
    }
    assert(cache);
    return *cache;
}

Result<FT_Fixed> FreeTypeFontFace::FindOrCacheAdvance(FontGlyphRasterParam param, FontGlyphId id) noexcept
{
    // 获取缓存
    auto key = make_tuple(param, id);
    auto cache = m_stAdvanceCache.TryGet(key);
    if (!cache)
    {
        // 没有命中缓存，加载 Advance
        auto ret = ApplySizeAndScale(param.Size);
        if (!ret)
            return ret.GetError();

        FT_Fixed advance = 0;
        auto err = ::FT_Get_Advance(m_pFace.get(), id, static_cast<FT_Int32>(param.Flags), &advance);
        if (err != 0)
            return make_error_code(static_cast<detail::FreeTypeError>(err));

        // 对于固定大小的字体，我们通过之前保存的 Scaling 对 Kerning 进行调整
        if (!FT_IS_SCALABLE(m_pFace) && FT_HAS_FIXED_SIZES(m_pFace))
        {
            auto scale = (param.Flags & FT_LOAD_VERTICAL_LAYOUT) ? m_pFace->size->metrics.y_scale : m_pFace->size->metrics.x_scale;
            advance = ::FT_MulFix(advance, scale);
        }

        try
        {
            cache = m_stAdvanceCache.Emplace(key, advance);
        }
        catch (...)  // bad_alloc
        {
            return make_error_code(errc::not_enough_memory);
        }
    }
    assert(cache);
    return *cache;
}
