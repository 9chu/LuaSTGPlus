/**
* @file
* @date 2022/6/26
* @author 9chu
* 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
*/
#pragma once
#include <memory>
#include "FontCollection.hpp"

namespace lstg::Subsystem::Render::Font
{
    /**
     * 文本方向
     */
    enum class TextDirection
    {
        LeftToRight,
        RightToLeft,
        Mixed,
    };

    /**
     * 排版后字形输出
     */
    struct FontShapedGlyph
    {
        IFontFace* FontFace = nullptr;
        FontGlyphId GlyphIndex = 0;
        size_t StartIndex = 0;
        uint32_t LineNumber = 0;
        float XAdvance = 0;
        float YAdvance = 0;
        float XOffset = 0;
        float YOffset = 0;
        float Kerning = 0;
        size_t GlyphCharacterCount = 0;
        size_t GlyphGraphemeClusterCount = 0;
        TextDirection Direction = TextDirection::LeftToRight;
        bool Visible = false;
    };

    /**
     * 文本整形器
     */
    class ITextShaper
    {
    public:
        ITextShaper() noexcept = default;
        virtual ~ITextShaper() noexcept = default;

    public:
        /**
         * 文本整形
         * @param output 输出字形数据
         * @param input 输入串
         * @param collection 字体集合
         * @param fontSize 字体大小（72DPI下的度量值）
         * @param baseDirection 基础书写方向
         * @return 是否成功
         */
        virtual Result<void> ShapeText(std::vector<FontShapedGlyph>& output, std::u16string_view input, FontCollection* collection,
            uint32_t fontSize, TextDirection baseDirection = TextDirection::LeftToRight) noexcept = 0;
    };

    using TextShaperPtr = std::shared_ptr<ITextShaper>;
}
