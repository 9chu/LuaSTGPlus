/**
* @file
* @date 2022/6/26
* @author 9chu
* 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
*/
#pragma once
#include <memory>
#include <optional>
#include <string_view>
#include "../../../Result.hpp"
#include "../../../Span.hpp"
#include "FixedPoint.hpp"
#include "FontSize.hpp"

namespace lstg::Subsystem::Render::Font
{
    /**
     * 水平排版时度量数据
     * @see https://stuff.mit.edu/afs/athena/astaff/source/src-9.0/third/freetype/docs/tutorial/step2.html
     */
    struct FontFaceMetrics
    {
        Q26D6 Ascender = 0;
        Q26D6 Descender = 0;
        Q26D6 LineGap = 0;
    };

    /**
     * 字形度量值
     */
    struct FontGlyphMetrics
    {
        Q26D6 XBearing = 0;
        Q26D6 YBearing = 0;
        Q26D6 Width = 0;
        Q26D6 Height = 0;
    };

    /**
     * 字形内部ID
     */
    using FontGlyphId = uint32_t;

    /**
     * 排版方向
     */
    enum class FontLayoutDirection
    {
        Horizontal,  ///< @brief 横向排版
        Vertical,  ///< @brief 垂直排版
    };

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

    /**
     * 字形对
     */
    struct FontGlyphPair
    {
        FontGlyphId Left = 0;
        FontGlyphId Right = 0;

        bool operator==(const FontGlyphPair& rhs) const noexcept
        {
            return Left == rhs.Left && Right == rhs.Right;
        }

        bool operator!=(const FontGlyphPair& rhs) const noexcept
        {
            return !operator==(rhs);
        }
    };

    /**
     * 共享只读二进制数据
     */
    using SharedConstBlob = std::tuple<std::shared_ptr<const uint8_t[]>, size_t>;

    /**
     * 字体
     */
    class IFontFace
    {
    public:
        IFontFace() noexcept = default;
        virtual ~IFontFace() noexcept = default;

    public:
        /**
         * 获取 Face 在字体文件中的索引
         * @return 索引
         */
        virtual uint32_t GetFaceIndex() const noexcept = 0;

        /**
         * 返回一个 em 有多少个单位
         */
        virtual uint32_t GetUnitsPerEm() const noexcept = 0;

        /**
         * 检查是否有 Kerning 特性
         */
        virtual bool HasKerning() const noexcept = 0;

        /**
         * 构造字形光栅化参数
         */
        virtual uint32_t MakeGlyphRasterFlags() const noexcept = 0;

        /**
         * 计算缩放后的 em 单位
         * @param size 字体大小
         */
        virtual Result<std::tuple<uint32_t, uint32_t>> GetUnitsPerEmScaled(FontSize size) noexcept = 0;

        /**
         * 获取字形ID
         * @see hb_font_funcs_set_nominal_glyphs_func
         * @param glyphsOutput 字形ID输出
         * @param codePoints 码点输入
         * @param fallback 在无法显示的时候尝试使用其他字形
         * @return 处理的字形数量
         */
        virtual size_t BatchGetNominalGlyphs(Span<FontGlyphId, true> glyphsOutput, Span<const char32_t, true> codePoints,
            bool fallback = true) noexcept = 0;

        /**
         * 获取变体字形ID
         * @see hb_font_funcs_set_variation_glyph_func
         * @param codePoint 码点
         * @param variationSelector 变体
         * @return 如果有字形则返回字形ID，否则返回空
         */
        virtual std::optional<FontGlyphId> GetVariationGlyph(char32_t codePoint, uint32_t variationSelector) noexcept = 0;

        /**
         * 获取字形的名称
         * @param nameOutput 名称输出
         * @param glyphId 字形ID
         */
        virtual Result<void> GetGlyphName(Span<char> nameOutput, FontGlyphId glyphId) noexcept = 0;

        /**
         * 根据名称获取字形
         * @param name 名称
         * @return 字形ID
         */
        virtual std::optional<FontGlyphId> GetGlyphByName(std::string_view name) noexcept = 0;

        /**
         * 获取排版时的度量值
         * @see hb_font_funcs_set_font_h_extents_func
         * @see hb_font_funcs_set_font_v_extents_func
         * @param param 光栅化参数
         * @param direction 排版方向
         */
        virtual Result<FontFaceMetrics> GetLayoutMetrics(FontGlyphRasterParam param, FontLayoutDirection direction) noexcept = 0;

        /**
         * 批量获取字形前进量
         * @see hb_font_funcs_set_glyph_h_advances_func
         * @see hb_font_funcs_set_glyph_v_advances_func
         * @param advancesOutput 水平前进量输出
         * @param param 光栅化参数
         * @param direction 排版方向
         * @param glyphs 字形输入
         */
        virtual void BatchGetAdvances(Span<Q26D6, true> advancesOutput, FontGlyphRasterParam param, FontLayoutDirection direction,
            Span<const uint32_t, true> glyphs) noexcept = 0;

        /**
         * 获取字形排版时的原点
         * @see hb_font_funcs_set_glyph_h_origin_func
         * @see hb_font_funcs_set_glyph_v_origin_func
         * @param param 光栅化参数
         * @param glyphId 字形ID
         * @return 如果有数据，返回<X,Y>，否则返回空
         */
        virtual Result<std::tuple<Q26D6, Q26D6>> GetGlyphOrigin(FontGlyphRasterParam param, FontLayoutDirection direction,
            FontGlyphId glyphId) noexcept = 0;

        /**
         * 获取字形之间的间距调整量
         * @see hb_font_funcs_set_glyph_h_kerning_func
         * @see hb_font_funcs_set_glyph_v_kerning_func
         * @param param 光栅化参数
         * @param direction 排版方向
         * @param pair 左右侧的字形
         * @return 调整量
         */
        virtual Result<Q26D6> GetGlyphKerning(FontGlyphRasterParam param, FontLayoutDirection direction, FontGlyphPair pair) noexcept = 0;

        /**
         * 获取字形的度量值
         * @param param 光栅化参数
         * @param glyphId 字形ID
         * @return 度量值
         */
        virtual Result<FontGlyphMetrics> GetGlyphMetrics(FontGlyphRasterParam param, FontGlyphId glyphId) noexcept = 0;

        /**
         * 获取字形的边界点
         * @param param 光栅化参数
         * @param glyphId 字形ID
         * @param pointIndex 点的索引
         * @return 点的X,Y坐标
         */
        virtual Result<std::tuple<Q26D6, Q26D6>> GetGlyphOutlinePoint(FontGlyphRasterParam param, FontGlyphId glyphId,
            size_t pointIndex) = 0;

        /**
         * 加载 TrueType / OpenType 字体的 SFNT 表
         * @param tag 标签
         * @return 原始二进制数据
         */
        virtual Result<SharedConstBlob> LoadSfntTable(uint32_t tag) noexcept = 0;
    };

    using FontFacePtr = std::shared_ptr<IFontFace>;
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

template <>
struct std::hash<lstg::Subsystem::Render::Font::FontGlyphPair>
{
    std::size_t operator()(const lstg::Subsystem::Render::Font::FontGlyphPair& p) const noexcept
    {
        return p.Left ^ p.Right;
    }
};
