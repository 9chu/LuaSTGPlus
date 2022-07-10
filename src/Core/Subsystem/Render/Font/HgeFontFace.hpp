/**
 * @file
 * @date 2022/7/3
 * @author 9chu
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#pragma once
#include <unordered_map>
#include <lstg/Core/Subsystem/Render/Font/IFontFace.hpp>

namespace lstg::Subsystem::Render::Font
{
    /**
     * HGE 纹理化字体
     */
    class HgeFontFace :
        public IFontFace
    {
    public:
        HgeFontFace() = default;

    public:
        /**
         * 获取图集纹理
         */
        TexturePtr GetAtlasTexture() const noexcept;

        /**
         * 设置图集纹理并刷新 UV 坐标
         * @param tex 纹理
         */
        void SetAtlasTexture(TexturePtr tex) noexcept;

        /**
         * 增加字形
         * @param ch 字符
         * @param x 距离位图左边
         * @param y 距离位图顶边
         * @param w 高度
         * @param h 宽度
         * @param leftOffset 距离上一个字符的距离
         * @param rightOffset 距离下一个字符的距离
         * @return 是否成功
         */
        Result<void> AppendGlyph(char32_t ch, float x, float y, float w, float h, float leftOffset, float rightOffset) noexcept;

    public:  // IFontFace
        uint32_t GetFaceIndex() const noexcept override;
        uint32_t GetUnitsPerEm() const noexcept override;
        bool HasKerning() const noexcept override;
        uint32_t MakeGlyphRasterFlags() const noexcept override;
        Result<std::tuple<uint32_t, uint32_t>> GetUnitsPerEmScaled(FontSize size) noexcept override;
        size_t BatchGetNominalGlyphs(Span<FontGlyphId, true> glyphsOutput, Span<const char32_t, true> codePoints,
            bool fallback = true) noexcept override;
        std::optional<FontGlyphId> GetVariationGlyph(char32_t codePoint, uint32_t variationSelector) noexcept override;
        Result<void> GetGlyphName(Span<char> nameOutput, FontGlyphId glyphId) noexcept override;
        std::optional<FontGlyphId> GetGlyphByName(std::string_view name) noexcept override;
        Result<FontFaceMetrics> GetLayoutMetrics(FontGlyphRasterParam param, FontLayoutDirection direction) noexcept override;
        void BatchGetAdvances(Span<Q26D6, true> advancesOutput, FontGlyphRasterParam param, FontLayoutDirection direction,
            Span<const uint32_t, true> glyphs) noexcept override;
        Result<std::tuple<Q26D6, Q26D6>> GetGlyphOrigin(FontGlyphRasterParam param, FontLayoutDirection direction,
            FontGlyphId glyphId) noexcept override;
        Result<Q26D6> GetGlyphKerning(FontGlyphRasterParam param, FontLayoutDirection direction, FontGlyphPair pair) noexcept override;
        Result<FontGlyphMetrics> GetGlyphMetrics(FontGlyphRasterParam param, FontGlyphId glyphId) noexcept override;
        Result<std::tuple<Q26D6, Q26D6>> GetGlyphOutlinePoint(FontGlyphRasterParam param, FontGlyphId glyphId, size_t pointIndex) override;
        Result<SharedConstBlob> LoadSfntTable(uint32_t tag) noexcept override;
        Result<FontGlyphAtlasInfo> GetGlyphAtlas(FontGlyphRasterParam param, FontGlyphId glyphId,
            DynamicFontGlyphAtlas* dynamicAtlas) noexcept override;

    private:
        struct GlyphInfo
        {
            char32_t CodePoint = '\0';
            Math::Rectangle<float, Math::TopDownTag> Rectangle;
            glm::vec2 DrawOffset;
            float Advance = 0.f;
            Math::UVRectangle UVRectangle;

            void RefreshUV(float texWidth, float texHeight) noexcept
            {
                if (texWidth == 0.f || texHeight == 0.f)
                {
                    UVRectangle = {
                        0.f, 0.f, 1.f, 1.f,
                    };
                }
                else
                {
                    UVRectangle = {
                        Rectangle.Left() / texWidth, Rectangle.Top() / texHeight,
                        Rectangle.Width() / texWidth, Rectangle.Height() / texHeight,
                    };
                }
            }
        };

        TexturePtr m_pImageTexture;
        std::vector<GlyphInfo> m_stGlyphs;
        std::unordered_map<char32_t, FontGlyphId> m_stCodePoint2GlyphId;
        float m_fMaxHeight = 0.f;
    };
}
