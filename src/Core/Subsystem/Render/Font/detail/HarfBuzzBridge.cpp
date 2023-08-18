/**
 * @file
 * @date 2022/6/27
 * @author 9chu
 * 此文件为 LuaSTGPlus 项目的一部分，版权与许可声明详见 COPYRIGHT.txt。
 */
#include "HarfBuzzBridge.hpp"

#include <vector>

using namespace std;
using namespace lstg;
using namespace lstg::Subsystem::Render::Font;
using namespace lstg::Subsystem::Render::Font::detail;

static hb_user_data_key_t s_stFacePtrStorageKey;

namespace
{
    ::hb_blob_t* FaceTableLoader(::hb_face_t* face, ::hb_tag_t tag, void* userData) noexcept
    {
        auto p = static_cast<FontFacePtr*>(userData);

        // 加载 SFNT 表
        auto data = (*p)->LoadSfntTable(tag);
        if (!data)
            return nullptr;

        // 复制 SharedPtr
        SharedConstBlob* ud = nullptr;
        try
        {
            ud = new SharedConstBlob(*data);
        }
        catch (...)  // bad_alloc
        {
            return nullptr;
        }

        // 创建 Blob
        auto deleter = [](void* ud) { delete static_cast<SharedConstBlob*>(ud); };
        auto blob = ::hb_blob_create(reinterpret_cast<const char*>((*data)->data()), (*data)->size(), HB_MEMORY_MODE_READONLY,
            ud, deleter);
        return blob;
    }

    struct FontInfo
    {
        FontFacePtr FontFace;
        FontGlyphRasterParam Param;
        FontLayoutDirection LayoutDirection = FontLayoutDirection::Horizontal;
        HarfBuzzBridge::FontFuncsPtr Funcs;
    };

    using FontInfoPtr = std::shared_ptr<FontInfo>;

    // <editor-fold desc="字体函数">

    hb_bool_t GetNominalGlyphBridge(hb_font_t* font, void* fontData, hb_codepoint_t unicode, hb_codepoint_t* glyph, void* userData) noexcept
    {
        auto ud = static_cast<FontInfoPtr*>(fontData);

        FontGlyphId out[1];
        if (0 == (*ud)->FontFace->BatchGetNominalGlyphs(Span<FontGlyphId, true>(out, 1, sizeof(out[1])),
            Span<const char32_t, true>(reinterpret_cast<char32_t*>(&unicode), 1, sizeof(unicode)), true))
        {
            return false;
        }
        *glyph = out[0];
        return true;
    }

    unsigned int GetNominalGlyphsBridge(hb_font_t* font, void* fontData, unsigned int count, const hb_codepoint_t* firstUnicode,
        unsigned int unicodeStride, hb_codepoint_t* firstGlyph, unsigned int glyphStride, void* userData) noexcept
    {
        static_assert(sizeof(hb_codepoint_t) == sizeof(char32_t));

        auto ud = static_cast<FontInfoPtr*>(fontData);

        // 批量获取字形
        return (*ud)->FontFace->BatchGetNominalGlyphs(Span<FontGlyphId, true>(reinterpret_cast<FontGlyphId*>(firstGlyph), count,
            glyphStride), Span<const char32_t, true>(reinterpret_cast<const char32_t*>(firstUnicode), count, unicodeStride), true);
    }

    hb_bool_t GetVariationGlyphBridge(hb_font_t* font, void* fontData, hb_codepoint_t unicode, hb_codepoint_t variationSelector,
        hb_codepoint_t* glyph, void* userData) noexcept
    {
        auto ud = static_cast<FontInfoPtr*>(fontData);

        auto ret = (*ud)->FontFace->GetVariationGlyph(unicode, variationSelector);
        if (ret)
        {
            *glyph = *ret;
            return true;
        }
        return false;
    }

    hb_bool_t GetGlyphExtentsBridge(hb_font_t* font, void* fontData, hb_codepoint_t glyph, hb_glyph_extents_t* extents,
        void* userData) noexcept
    {
        auto ud = static_cast<FontInfoPtr*>(fontData);

        int scaleX = 0, scaleY = 0;
        ::hb_font_get_scale(font, &scaleX, &scaleY);

        auto ret = (*ud)->FontFace->GetGlyphMetrics((*ud)->Param, glyph);
        if (!ret)
            return false;

        extents->width = ret->Width;
        extents->height = ret->Height;
        extents->x_bearing = ret->XBearing;
        extents->y_bearing = ret->YBearing;
        if (scaleX < 0)
        {
            extents->x_bearing = -extents->x_bearing;
            extents->width = -extents->width;
        }
        if (scaleY < 0)
        {
            extents->y_bearing = -extents->y_bearing;
            extents->height = -extents->height;
        }
        return true;
    }

    hb_bool_t GetGlyphContourPointBridge(hb_font_t* font, void* fontData, hb_codepoint_t glyph, unsigned int pointIndex, hb_position_t* x,
        hb_position_t* y, void* userData) noexcept
    {
        auto ud = static_cast<FontInfoPtr*>(fontData);

        auto ret = (*ud)->FontFace->GetGlyphOutlinePoint((*ud)->Param, glyph, pointIndex);
        if (!ret)
            return false;

        *x = std::get<0>(*ret);
        *y = std::get<1>(*ret);
        return true;
    }

    hb_bool_t GetGlyphNameBridge(hb_font_t* font, void* fontData, hb_codepoint_t glyph, char* name, unsigned int size,
        void* userData) noexcept
    {
        auto ud = static_cast<FontInfoPtr*>(fontData);
        return (*ud)->FontFace->GetGlyphName({name, size}, glyph);
    }

    hb_bool_t GetGlyphFromNameBridge(hb_font_t* font, void* fontData, const char* name, int len, hb_codepoint_t* glyph,
        void* userData) noexcept
    {
        auto ud = static_cast<FontInfoPtr*>(fontData);
        auto ret = (*ud)->FontFace->GetGlyphByName(len < 0 ? name : string_view{name, static_cast<size_t>(len)});
        if (ret)
        {
            *glyph = static_cast<hb_codepoint_t>(*ret);
            return true;
        }
        return false;
    }

    hb_bool_t GetFontHExtentsBridge(hb_font_t* font, void* fontData, hb_font_extents_t* metrics, void* userData) noexcept
    {
        auto ud = static_cast<FontInfoPtr*>(fontData);

        int scaleX = 0, scaleY = 0;
        ::hb_font_get_scale(font, &scaleX, &scaleY);

        auto m = (*ud)->FontFace->GetLayoutMetrics((*ud)->Param, FontLayoutDirection::Horizontal);
        if (!m)
            return false;
        metrics->ascender = m->Ascender;
        metrics->descender = m->Descender;
        metrics->line_gap = m->LineGap;
        if (scaleY < 0)
        {
            metrics->ascender = -metrics->ascender;
            metrics->descender = -metrics->descender;
            metrics->line_gap = -metrics->line_gap;
        }
        return true;
    }

    hb_bool_t GetFontVExtentsBridge(hb_font_t* font, void* fontData, hb_font_extents_t* metrics, void* userData) noexcept
    {
        auto ud = static_cast<FontInfoPtr*>(fontData);

        int scaleX = 0, scaleY = 0;
        ::hb_font_get_scale(font, &scaleX, &scaleY);

        auto m = (*ud)->FontFace->GetLayoutMetrics((*ud)->Param, FontLayoutDirection::Vertical);
        if (!m)
            return false;
        metrics->ascender = m->Ascender;
        metrics->descender = m->Descender;
        metrics->line_gap = m->LineGap;
        if (scaleY < 0)
        {
            metrics->ascender = -metrics->ascender;
            metrics->descender = -metrics->descender;
            metrics->line_gap = -metrics->line_gap;
        }
        return true;
    }

    void GetGlyphHAdvancesBridge(hb_font_t* font, void* fontData, unsigned count, const hb_codepoint_t* firstGlyph, unsigned glyphStride,
        hb_position_t* firstAdvance, unsigned advanceStride, void* userData) noexcept
    {
        auto ud = static_cast<FontInfoPtr*>(fontData);

        int scaleX = 0, scaleY = 0;
        ::hb_font_get_scale(font, &scaleX, &scaleY);

        auto advancesOutput = Span<Q16D16, true>(firstAdvance, count, advanceStride);
        (*ud)->FontFace->BatchGetAdvances(advancesOutput, (*ud)->Param, FontLayoutDirection::Horizontal,
            Span<const FontGlyphId, true>(reinterpret_cast<const FontGlyphId*>(firstGlyph), count,glyphStride));

        // 如果 scaleX < 0，则需要翻转
        if (scaleX < 0)
        {
            auto it = advancesOutput.Begin();
            while (it != advancesOutput.End())
                (*it) *= -1;
        }
    }

    hb_position_t GetGlyphHAdvanceBridge(hb_font_t* font, void* fontData, hb_codepoint_t glyph, void* userData) noexcept
    {
        hb_position_t adv = 0;
        GetGlyphHAdvancesBridge(font, fontData, 1, &glyph, sizeof(glyph), &adv, sizeof(adv), userData);
        return adv;
    }

    void GetGlyphVAdvancesBridge(hb_font_t* font, void* fontData, unsigned count, const hb_codepoint_t* firstGlyph, unsigned glyphStride,
        hb_position_t* firstAdvance, unsigned advanceStride, void* userData) noexcept
    {
        auto ud = static_cast<FontInfoPtr*>(fontData);

        int scaleX = 0, scaleY = 0;
        ::hb_font_get_scale(font, &scaleX, &scaleY);

        auto advancesOutput = Span<Q16D16, true>(firstAdvance, count, advanceStride);
        (*ud)->FontFace->BatchGetAdvances(advancesOutput, (*ud)->Param, FontLayoutDirection::Vertical,
            Span<const FontGlyphId, true>(reinterpret_cast<const FontGlyphId*>(firstGlyph), count,glyphStride));

        // 如果 scaleY < 0，则需要翻转
        if (scaleY < 0)
        {
            auto it = advancesOutput.Begin();
            while (it != advancesOutput.End())
                (*it) *= -1;
        }
    }

    hb_position_t GetGlyphVAdvanceBridge(hb_font_t* font, void* fontData, hb_codepoint_t glyph, void* userData) noexcept
    {
        hb_position_t adv = 0;
        GetGlyphVAdvancesBridge(font, fontData, 1, &glyph, sizeof(glyph), &adv, sizeof(adv), userData);
        return adv;
    }

    hb_bool_t GetGlyphHOriginBridge(hb_font_t* font, void* fontData, hb_codepoint_t glyph, hb_position_t* x, hb_position_t* y,
        void* userData) noexcept
    {
        auto ud = static_cast<FontInfoPtr*>(fontData);

        int scaleX = 0, scaleY = 0;
        ::hb_font_get_scale(font, &scaleX, &scaleY);

        auto ret = (*ud)->FontFace->GetGlyphOrigin((*ud)->Param, FontLayoutDirection::Horizontal, glyph);
        if (!ret)
            return false;

        *x = std::get<0>(*ret);
        *y = std::get<1>(*ret);

        if (scaleX < 0)
            *x = -*x;
        if (scaleY < 0)
            *y = -*y;
        return true;
    }

    hb_bool_t GetGlyphVOriginBridge(hb_font_t* font, void* fontData, hb_codepoint_t glyph, hb_position_t* x, hb_position_t* y,
        void* userData) noexcept
    {
        auto ud = static_cast<FontInfoPtr*>(fontData);

        int scaleX = 0, scaleY = 0;
        ::hb_font_get_scale(font, &scaleX, &scaleY);

        auto ret = (*ud)->FontFace->GetGlyphOrigin((*ud)->Param, FontLayoutDirection::Vertical, glyph);
        if (!ret)
            return false;

        *x = std::get<0>(*ret);
        *y = std::get<1>(*ret);

        if (scaleX < 0)
            *x = -*x;
        if (scaleY < 0)
            *y = -*y;
        return true;
    }

    hb_position_t GetGlyphHKerningBridge(hb_font_t* font, void* fontData, hb_codepoint_t leftGlyph, hb_codepoint_t rightGlyph,
        void* userData) noexcept
    {
        auto ud = static_cast<FontInfoPtr*>(fontData);

        auto ret = (*ud)->FontFace->GetGlyphKerning((*ud)->Param, FontLayoutDirection::Horizontal, {leftGlyph, rightGlyph});
        if (!ret)
            return 0;
        return *ret;
    }

    hb_position_t GetGlyphVKerningBridge(hb_font_t* font, void* fontData, hb_codepoint_t leftGlyph, hb_codepoint_t rightGlyph,
        void* userData) noexcept
    {
        auto ud = static_cast<FontInfoPtr*>(fontData);

        auto ret = (*ud)->FontFace->GetGlyphKerning((*ud)->Param, FontLayoutDirection::Vertical, {leftGlyph, rightGlyph});
        if (!ret)
            return 0;
        return *ret;
    }

    // </editor-fold>
}

Result<HarfBuzzBridge::BufferPtr> HarfBuzzBridge::CreateBuffer() noexcept
{
    // 创建缓冲区
    BufferPtr ret { ::hb_buffer_create() };
    if (!::hb_buffer_allocation_successful(ret.get()))
    {
        ret.release();
        return make_error_code(errc::not_enough_memory);
    }
    return ret;
}

Result<HarfBuzzBridge::FacePtr> HarfBuzzBridge::CreateFace(FontFacePtr face) noexcept
{
    FontFacePtr* ud = nullptr;
    try
    {
        ud = new FontFacePtr(face);
    }
    catch (...)  // bad_alloc
    {
        return make_error_code(errc::not_enough_memory);
    }
    auto deleter = [](void* ud) { delete static_cast<FontFacePtr*>(ud); };

    // 创建字体
    FacePtr ret { ::hb_face_create_for_tables(FaceTableLoader, ud, deleter) };
    if (::hb_face_get_empty() == ret.get())
    {
        ret.release();
        return make_error_code(errc::not_enough_memory);
    }
    assert(ret);

    // 绑定 UserData
    // 我们并不能直接访问 hb_face_t::user_data，故只能再塞一个到 hb_face_t::header
    try
    {
        ud = new FontFacePtr(face);
    }
    catch (...)  // bad_alloc
    {
        return make_error_code(errc::not_enough_memory);
    }
    auto err = ::hb_face_set_user_data(ret.get(), &s_stFacePtrStorageKey, ud, deleter, true);
    if (!err)
        return make_error_code(errc::not_enough_memory);

    ::hb_face_set_index(ret.get(), face->GetFaceIndex());
    ::hb_face_set_upem(ret.get(), face->GetUnitsPerEm());
    return ret;
}

Result<HarfBuzzBridge::FontPtr> HarfBuzzBridge::CreateFont(FacePtr face, FontGlyphRasterParam param) noexcept
{
    // 获取 IFontFace 对象
    auto ud = static_cast<FontFacePtr*>(::hb_face_get_user_data(face.get(), &s_stFacePtrStorageKey));
    if (!ud)
    {
        assert(false);
        return make_error_code(errc::invalid_argument);
    }

    // 创建字体
    FontPtr ret { ::hb_font_create(face.get()) };
    if (ret.get() == ::hb_font_get_empty())
    {
        ret.release();
        return make_error_code(errc::not_enough_memory);
    }
    assert(ret);

    // 设置缩放
    auto scale = (*ud)->GetUnitsPerEmScaled(param.Size);
    if (!scale)
        return scale.GetError();
    ::hb_font_set_scale(ret.get(), std::get<0>(*scale), std::get<1>(*scale));

    // 保存字体信息
    auto info = make_shared<FontInfo>();
    info->FontFace = *ud;
    info->Param = param;
    info->Funcs.reset(::hb_font_funcs_create());
    if (info->Funcs.get() == ::hb_font_funcs_get_empty())
    {
        info->Funcs.release();
        return make_error_code(errc::not_enough_memory);
    }

    // 创建字体方法
    ::hb_font_funcs_set_nominal_glyph_func(info->Funcs.get(), GetNominalGlyphBridge, nullptr, nullptr);
    ::hb_font_funcs_set_nominal_glyphs_func(info->Funcs.get(), GetNominalGlyphsBridge, nullptr, nullptr);
    ::hb_font_funcs_set_variation_glyph_func(info->Funcs.get(), GetVariationGlyphBridge, nullptr, nullptr);
    ::hb_font_funcs_set_glyph_extents_func(info->Funcs.get(), GetGlyphExtentsBridge, nullptr, nullptr);
    ::hb_font_funcs_set_glyph_contour_point_func(info->Funcs.get(), GetGlyphContourPointBridge, nullptr, nullptr);
    ::hb_font_funcs_set_glyph_name_func(info->Funcs.get(), GetGlyphNameBridge, nullptr, nullptr);
    ::hb_font_funcs_set_glyph_from_name_func(info->Funcs.get(), GetGlyphFromNameBridge, nullptr, nullptr);
    ::hb_font_funcs_set_font_h_extents_func(info->Funcs.get(), GetFontHExtentsBridge, nullptr, nullptr);
    ::hb_font_funcs_set_font_v_extents_func(info->Funcs.get(), GetFontVExtentsBridge, nullptr, nullptr);
    ::hb_font_funcs_set_glyph_h_advances_func(info->Funcs.get(), GetGlyphHAdvancesBridge, nullptr, nullptr);
    ::hb_font_funcs_set_glyph_v_advances_func(info->Funcs.get(), GetGlyphVAdvancesBridge, nullptr, nullptr);
    ::hb_font_funcs_set_glyph_h_advance_func(info->Funcs.get(), GetGlyphHAdvanceBridge, nullptr, nullptr);
    ::hb_font_funcs_set_glyph_v_advance_func(info->Funcs.get(), GetGlyphVAdvanceBridge, nullptr, nullptr);
    ::hb_font_funcs_set_glyph_h_origin_func(info->Funcs.get(), GetGlyphHOriginBridge, nullptr, nullptr);
    ::hb_font_funcs_set_glyph_v_origin_func(info->Funcs.get(), GetGlyphVOriginBridge, nullptr, nullptr);
    ::hb_font_funcs_set_glyph_h_kerning_func(info->Funcs.get(), GetGlyphHKerningBridge, nullptr, nullptr);
    ::hb_font_funcs_set_glyph_v_kerning_func(info->Funcs.get(), GetGlyphVKerningBridge, nullptr, nullptr);
    ::hb_font_funcs_make_immutable(info->Funcs.get());

    // 绑定字体方法
    FontInfoPtr* ud2 = nullptr;
    try
    {
        ud2 = new FontInfoPtr(info);
    }
    catch (...)
    {
        return make_error_code(errc::not_enough_memory);
    }
    auto deleter = [](void* ud) { delete static_cast<FontInfoPtr*>(ud); };
    ::hb_font_set_funcs(ret.get(), info->Funcs.get(), ud2, deleter);
    return ret;
}
