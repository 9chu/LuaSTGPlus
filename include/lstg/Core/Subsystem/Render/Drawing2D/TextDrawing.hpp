/**
 * @file
 * @date 2022/7/2
 * @author 9chu
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#pragma once
#include "../../../Math/Rectangle.hpp"
#include "../Font/FontCollection.hpp"
#include "../Font/ITextShaper.hpp"
#include "CommandBuffer.hpp"

namespace lstg::Subsystem::Render::Drawing2D
{
    namespace detail
    {
        struct ShapedTextCacheKey
        {
            std::string Text;
            Font::FontCollection* Collection = nullptr;
            uint32_t FontSize = 0;
            float FontScale = 0.f;

            bool operator==(const ShapedTextCacheKey& rhs) const noexcept
            {
                return Text == rhs.Text && Collection == rhs.Collection && FontSize == rhs.FontSize && FontScale == rhs.FontScale;
            }

            bool operator!=(const ShapedTextCacheKey& rhs) const noexcept
            {
                return !operator==(rhs);
            }
        };
    }
}

template <>
struct std::hash<lstg::Subsystem::Render::Drawing2D::detail::ShapedTextCacheKey>
{
    std::size_t operator()(const lstg::Subsystem::Render::Drawing2D::detail::ShapedTextCacheKey& key) const noexcept
    {
        auto h1 = std::hash<std::string>{}(key.Text);
        auto h2 = std::hash<void*>{}(key.Collection);
        auto h3 = std::hash<uint32_t>{}(key.FontSize);
        auto h4 = std::hash<float>{}(key.FontScale);
        return h1 ^ h2 ^ h3 ^ h4;
    }
};

namespace lstg::Subsystem::Render::Drawing2D
{
    /**
     * 文本水平对齐
     */
    enum class TextHorizontalAlignment
    {
        Left = 0,
        Center = 1,
        Right = 2,
    };

    /**
     * 文本垂直对齐
     */
    enum class TextVerticalAlignment
    {
        Top = 0,
        Middle = 1,
        Bottom = 2,
    };

    /**
     * 行间距类型
     */
    enum class TextLineGapTypes
    {
        Times = 0,  ///< @brief 倍数
        Fixed = 1,  ///< @brief 固定
    };

    /**
     * 行间距
     */
    struct TextLineGap
    {
        TextLineGapTypes Type = TextLineGapTypes::Times;
        float Value = 0.f;

        bool operator==(const TextLineGap& rhs) const noexcept
        {
            return Type == rhs.Type && Value == rhs.Value;
        }

        bool operator!=(const TextLineGap& rhs) const noexcept
        {
            return !operator==(rhs);
        }
    };

    /**
     * 断行方式
     */
    enum class TextLineBreakTypes
    {
        None = 0,
        Anywhere = 1,
    };

    /**
     * 文本排版样式
     */
    struct TextLayoutStyle
    {
        /**
         * 水平对齐
         */
        TextHorizontalAlignment HorizontalAlignment = TextHorizontalAlignment::Left;

        /**
         * 垂直对齐
         */
        TextVerticalAlignment VerticalAlignment = TextVerticalAlignment::Top;

        /**
         * 断行
         */
        TextLineBreakTypes LineBreak = TextLineBreakTypes::None;

        /**
         * 行间距
         */
        TextLineGap ParagraphInnerLineGap;

        /**
         * 段前间距
         */
        TextLineGap MarginBeforeParagraph;

        /**
         * 段后间距
         */
        TextLineGap MarginAfterParagraph;

        bool operator==(const TextLayoutStyle& rhs) const noexcept
        {
            return HorizontalAlignment == rhs.HorizontalAlignment && VerticalAlignment == rhs.VerticalAlignment &&
                LineBreak == rhs.LineBreak && ParagraphInnerLineGap == rhs.ParagraphInnerLineGap &&
                MarginBeforeParagraph == rhs.MarginBeforeParagraph && MarginAfterParagraph == rhs.MarginAfterParagraph;
        }

        bool operator!=(const TextLayoutStyle& rhs) const noexcept
        {
            return !operator==(rhs);
        }
    };

    /**
     * 文本绘制样式
     */
    struct TextDrawingStyle
    {
        /**
         * 字体大小
         */
        uint32_t FontSize = 12;

        /**
         * 缩放
         */
        float FontScale = 1.f;

        /**
         * 排版样式
         */
        TextLayoutStyle LayoutStyle;

        /**
         * 文本颜色（加算）
         */
        ColorRGBA32 AdditiveTextColor;

        /**
         * 文本颜色（乘算）
         */
        ColorRGBA32 MultiplyTextColor;

        bool operator==(const TextDrawingStyle& rhs) const noexcept
        {
            return FontSize == rhs.FontSize && FontScale == rhs.FontScale && LayoutStyle == rhs.LayoutStyle &&
                AdditiveTextColor == rhs.AdditiveTextColor && MultiplyTextColor == rhs.MultiplyTextColor;
        }

        bool operator!=(const TextDrawingStyle& rhs) const noexcept
        {
            return !operator==(rhs);
        }
    };

    /**
     * 文本绘制工具
     */
    class TextDrawing
    {
    private:
        /**
         * 段落测量信息
         */
        struct ParagraphMeasureInfo
        {
            float Ascender = 0;
            float Descender = 0;
            float LineGap = 0;
            float LineHeight = 0;
            float LineWidth = 0;
            size_t GlyphStartIndex = 0;
            size_t GlyphCount = 0;
        };

        /**
         * 计算后行测量信息
         */
        struct ProcessedLineMeasureInfo
        {
            float MarginTop = 0;  ///< @brief 距离上一行的距离
            float LineWidth = 0;  ///< @brief 行宽
            float LineHeight = 0;  ///< @brief 行高
            float DrawPosTop = 0;  ///< @brief 绘制起笔位置距离顶端距离

            size_t GlyphRunCount = 0;  ///< @brief 包含的分段
            std::tuple<size_t, size_t> GlyphRuns[3];  ///< @brief 包含的字形分段，<开始索引, 长度>，理论最多不超过三个分段
        };

        /**
         * 整形后文本信息
         */
        struct ShapedTextInfo
        {
            Font::FontCollectionPtr Collection;  ///< @brief 关联的字形集合
            std::u16string Text;  ///< @brief 转换到 u16string 的文本
            std::vector<Font::FontShapedGlyph> ShapedGlyphs;  ///< @brief 整形后字形
            std::vector<ParagraphMeasureInfo> ParagraphMeasures;  ///< @brief 段落信息，由于整形后文本都是一个段落一行，所以这里等价于各行的度量信息

            // 这里缓存最后一次布局后的数据，避免帧间反复生成，同时作为 Buffer
            // 如果下述参数发生变化，则需要重新生成
            float LayoutMaxWidth = 0.f;
            TextLineBreakTypes LayoutLineBreak = TextLineBreakTypes::None;
            TextLineGap LayoutParagraphInnerLineGap;
            TextLineGap LayoutMarginBeforeParagraph;
            TextLineGap LayoutMarginAfterParagraph;

            // 布局中间结果
            float LayoutWidth = 0.f;
            float LayoutHeight = 0.f;
            std::vector<ProcessedLineMeasureInfo> LayoutProcessedLine;
        };

    public:
        static const uint32_t kShapedTextCacheSize = 100;  // 保存最近显示的多少个文本的整形信息

        /**
         * 整形缓存
         */
        class ShapedTextCache
        {
        public:
            /**
             * 寻找缓存
             * @param text 文本
             * @param collection 字体集合
             * @param fontSize 字体大小
             * @param fontScale 字体缩放
             * @return 缓存信息
             */
            ShapedTextInfo* FindCache(std::string_view text, Font::FontCollection* collection, uint32_t fontSize, float fontScale) noexcept;

            /**
             * 缓存字形
             * @param text 文本
             * @param collection 字体集合
             * @param fontSize 字体大小
             * @param fontScale 字体缩放
             * @param info 整形信息
             * @return 缓存信息
             */
            Result<ShapedTextInfo*> Cache(std::string_view text, Font::FontCollection* collection, uint32_t fontSize, float fontScale,
                ShapedTextInfo&& info) noexcept;

        private:
            detail::ShapedTextCacheKey m_stTmpKey;
            LRUCache<detail::ShapedTextCacheKey, ShapedTextInfo, kShapedTextCacheSize> m_stShapedTextCache;
        };

    public:
        /**
         * 绘制文本
         * @param cache 缓存器
         * @param cmdBuffer 命令缓冲
         * @param collection 字体集合
         * @param shaper 整形器
         * @param text 文本
         * @param rect 绘制范围
         * @param style 样式
         * @return
         */
        static Result<void> Draw(TextDrawing::ShapedTextCache& cache, CommandBuffer& cmdBuffer, Font::FontCollectionPtr collection,
            Font::DynamicFontGlyphAtlas* dynamicAtlas, Font::ITextShaper* shaper, std::string_view text, Math::XYRectangle rect,
            TextDrawingStyle style) noexcept;

    private:
        /**
         * 检查是不是所有的段落都可以在一行显示
         * @param paragraphs 段落
         * @param maxWidth 限制宽度
         * @return 可以在一行显示
         */
        static bool IsAllParagraphFitInLine(const std::vector<ParagraphMeasureInfo>& paragraphs, float maxWidth) noexcept;
    };
}
