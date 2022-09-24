/**
 * @file
 * @date 2022/6/27
 * @author 9chu
 * 此文件为 LuaSTGPlus 项目的一部分，版权与许可声明详见 COPYRIGHT.txt。
 */
#pragma once
#include "../../../detail/IcuService.hpp"
#include <lstg/Core/Subsystem/Render/Font/ITextShaper.hpp>
#include "detail/HarfBuzzBridge.hpp"
#include "detail/CommonDefines.hpp"

namespace lstg::Subsystem::Render::Font
{
    /**
     * 字体缓存键
     */
    struct HarfBuzzFontCacheKey
    {
        IFontFace* Face = nullptr;
        FontGlyphRasterParam Param;

        bool operator==(const HarfBuzzFontCacheKey& rhs) const noexcept
        {
            return Face == rhs.Face && Param == rhs.Param;
        }

        bool operator!=(const HarfBuzzFontCacheKey& rhs) const noexcept
        {
            return !operator==(rhs);
        }
    };
}

// 必须在这里特化
template <>
struct std::hash<lstg::Subsystem::Render::Font::HarfBuzzFontCacheKey>
{
    std::size_t operator()(const lstg::Subsystem::Render::Font::HarfBuzzFontCacheKey& s) const noexcept
    {
        using namespace lstg::Subsystem::Render::Font;

        auto h1 = std::hash<IFontFace*>{}(s.Face);
        auto h2 = std::hash<FontGlyphRasterParam>{}(s.Param);
        return h1 ^ h2;
    }
};

namespace lstg::Subsystem::Render::Font
{
    /**
     * HarfBuzz 字体整形器
     */
    class HarfBuzzTextShaper :
        public ITextShaper
    {
        struct TextRun
        {
            // 相对于原始串的位置
            size_t StartIndex = 0;
            size_t Length = 0;
        };

        struct ScriptedTextRun :
            public TextRun
        {
            // 语言系统
            ::hb_script_t Script = HB_SCRIPT_COMMON;
        };

        struct ProcessingTextRun :
            public TextRun
        {
            // 行号
            uint32_t LineNumber = 0;

            // 书写方向
            TextDirection Direction = TextDirection::LeftToRight;

            // 字体信息
            FontFacePtr Font;
            float FontScaling = 0.f;

            // 子序列
            std::vector<ScriptedTextRun> SubSequences;  // TODO: 替换成 small_vector
        };

        static bool HandleSpecialCharacter(std::vector<FontShapedGlyph>& output, const ProcessingTextRun& run,
            const FontGlyphRasterParam& fontParam, const hb_glyph_info_t& glyphInfo, const hb_glyph_position_t& glyphPosition,
            size_t currentCharIndex, char32_t currentChar);

    public:
        HarfBuzzTextShaper();
        HarfBuzzTextShaper(const HarfBuzzTextShaper& rhs) = delete;

    public:  // ITextShaper
        Result<void> ShapeText(std::vector<FontShapedGlyph>& output, std::u16string_view input, FontCollection* collection,
            uint32_t fontSize, float fontScale, TextDirection baseDirection) noexcept override;

    private:
        void BreakParagraph(std::vector<ProcessingTextRun>& output, std::u16string_view text,
            const std::vector<ProcessingTextRun>& input);
        Result<void> BreakDirection(std::vector<ProcessingTextRun>& output, std::u16string_view text,
            const std::vector<ProcessingTextRun>& input, UBiDi* bidi, TextDirection baseDirection);
        void ChooseFont(std::vector<ProcessingTextRun>& output, std::u16string_view text,
            const std::vector<ProcessingTextRun>& input, FontCollection& collection, float fontScale);
        void SplitScript(std::vector<ProcessingTextRun>& output, std::u16string_view text,
            const std::vector<ProcessingTextRun>& input);

        Result<detail::HarfBuzzBridge::FontPtr*> CreateHBFont(FontFacePtr face, FontGlyphRasterParam param) noexcept;

    private:
        lstg::detail::IcuBidiPtr m_pBidi;
        lstg::detail::IcuBreakIteratorPtr m_pGraphemeBreakIter;
        detail::HarfBuzzBridge::BufferPtr m_pHBBuffer;
        LRUCache<HarfBuzzFontCacheKey, detail::HarfBuzzBridge::FontPtr, detail::kFontSizeCacheSize> m_stHBFontCache;
        std::vector<ProcessingTextRun> m_stTmpBuffers[2];
    };
}
