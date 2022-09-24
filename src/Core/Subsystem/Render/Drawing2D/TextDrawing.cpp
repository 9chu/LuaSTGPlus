/**
 * @file
 * @date 2022/7/2
 * @author 9chu
 * 此文件为 LuaSTGPlus 项目的一部分，版权与许可声明详见 COPYRIGHT.txt。
 */
#include <lstg/Core/Subsystem/Render/Drawing2D/TextDrawing.hpp>

#include <lstg/Core/Logging.hpp>
#include <lstg/Core/Encoding/Convert.hpp>
#include <lstg/Core/Encoding/Unicode.hpp>
#include <lstg/Core/Subsystem/Render/Drawing2D/SpriteDrawing.hpp>

using namespace std;
using namespace lstg;
using namespace lstg::Subsystem::Render::Drawing2D;

LSTG_DEF_LOG_CATEGORY(TextDrawing);

namespace
{
    float CalcLineGap(TextLineGap gap, float lineHeight)
    {
        switch (gap.Type)
        {
            default:
                assert(false);
            case TextLineGapTypes::Times:
                return lineHeight * gap.Value;
            case TextLineGapTypes::Fixed:
                return gap.Value;
        }
    }
}

// <editor-fold desc="TextDrawing::ShapedTextCache">

TextDrawing::ShapedTextInfo* TextDrawing::ShapedTextCache::FindCache(std::string_view text, Font::FontCollection* collection,
    uint32_t fontSize, float fontScale) noexcept
{
    try
    {
        m_stTmpKey.Text = text;
        m_stTmpKey.Collection = collection;
        m_stTmpKey.FontSize = fontSize;
        m_stTmpKey.FontScale = fontScale;
        return m_stShapedTextCache.TryGet(m_stTmpKey);
    }
    catch (...)  // std::bad_alloc
    {
        return nullptr;
    }
}

Result<TextDrawing::ShapedTextInfo*> TextDrawing::ShapedTextCache::Cache(std::string_view text, Font::FontCollection* collection,
    uint32_t fontSize, float fontScale, ShapedTextInfo&& info) noexcept
{
    try
    {
        m_stTmpKey.Text = text;
        m_stTmpKey.Collection = collection;
        m_stTmpKey.FontSize = fontSize;
        m_stTmpKey.FontScale = fontScale;
        return m_stShapedTextCache.Emplace(std::move(m_stTmpKey), std::move(info));
    }
    catch (...)  // std::bad_alloc
    {
        return make_error_code(errc::not_enough_memory);
    }
}

// </editor-fold>

// <editor-fold desc="TextDrawing">

Result<void> TextDrawing::Draw(TextDrawing::ShapedTextCache& cache, CommandBuffer& cmdBuffer, Font::FontCollectionPtr collection,
    Font::DynamicFontGlyphAtlas* dynamicAtlas, Font::ITextShaper* shaper, std::string_view text, Math::XYRectangle rect,
    TextDrawingStyle style) noexcept
{
    assert(collection);
    assert(shaper);

    // Step1. 文本整形
    auto shapedRet = ShapeText(cache, std::move(collection), shaper, text, style);
    if (!shapedRet)
        return shapedRet.GetError();
    auto shapedTextInfo = *shapedRet;

    // Step2. 对整形后的文字进行简单的排版
    auto layoutRet = LayoutText(shapedTextInfo, rect, style);
    if (!layoutRet)
        return layoutRet.GetError();

    // Step3. 渲染
    // 此时已经完成排版的文字数据存储在 shapedTextInfo 上
    float yOffset = 0;
    if (style.LayoutStyle.VerticalAlignment != TextVerticalAlignment::Top)
    {
        yOffset = shapedTextInfo->LayoutHeight - rect.Height();
        if (style.LayoutStyle.VerticalAlignment == TextVerticalAlignment::Middle)
            yOffset /= 2.f;
    }
    float y = rect.Top() + yOffset;
    for (const auto& line : shapedTextInfo->LayoutProcessedLine)
    {
        y -= line.MarginTop;

        // 决定笔触位置
        auto drawPosY = y - line.DrawPosTop;
        auto drawPosX = 0.f;
        if (style.LayoutStyle.HorizontalAlignment != TextHorizontalAlignment::Left)
        {
            drawPosX = rect.Width() - line.LineWidth;
            if (style.LayoutStyle.HorizontalAlignment == TextHorizontalAlignment::Center)
                drawPosX /= 2.f;
        }
        drawPosX += rect.Left();

        // 逐个字形获取图集并绘制
        for (size_t i = 0; i < line.GlyphRunCount; ++i)
        {
            for (size_t j = 0; j < std::get<1>(line.GlyphRuns[i]); ++j)
            {
                auto glyphIndex = std::get<0>(line.GlyphRuns[i]) + j;
                const auto& glyph = shapedTextInfo->ShapedGlyphs[glyphIndex];

                // 特殊处理字符缺失的情况
                if (glyph.GlyphIndex == 0)
                {
                    LSTG_LOG_WARN_CAT(TextDrawing, "Lack of glyph for character near position {}, text=\"{}\"", glyph.StartIndex, text);
                }
                else
                {
                    // 获取图集
                    auto atlasInfo = glyph.FontFace->GetGlyphAtlas(glyph.Param, glyph.GlyphIndex, dynamicAtlas);
                    if (!atlasInfo)
                        return atlasInfo.GetError();

                    // 绘制精灵
                    auto drawer = SpriteDrawing::Draw(cmdBuffer, atlasInfo->Texture);
                    if (!drawer)
                        return drawer.GetError();

                    const auto& texRect = atlasInfo->TextureRect;
                    drawer->Texture(texRect.Left(), texRect.Top(), texRect.Width(), texRect.Height());
                    drawer->SetAdditiveColor(style.AdditiveTextColor);
                    drawer->SetMultiplyColor(style.MultiplyTextColor);
                    drawer->Shape(atlasInfo->DrawSize.x, atlasInfo->DrawSize.y, 0, 0);
                    drawer->Translate(drawPosX + atlasInfo->DrawOffset.x + glyph.XOffset,
                        drawPosY + atlasInfo->DrawOffset.y + glyph.YOffset, 0.5);
                }

                drawPosX += glyph.XAdvance;
                drawPosY += glyph.YAdvance;
            }
        }

        y -= line.LineHeight;
    }
    return {};
}

Result<glm::vec2> TextDrawing::MeasureNonBreakSize(TextDrawing::ShapedTextCache& cache, Font::FontCollectionPtr collection,
    Font::ITextShaper* shaper, std::string_view text, TextDrawingStyle style) noexcept
{
    assert(collection);
    assert(shaper);

    // Step1. 文本整形
    auto shapedRet = ShapeText(cache, std::move(collection), shaper, text, style);
    if (!shapedRet)
        return shapedRet.GetError();
    auto shapedTextInfo = *shapedRet;

    // Step2. 计算占用空间大小，相当于 Layout 的退化算法
    // 如果 shapedTextInfo 上已经缓存的类似情形的计算结果，则直接使用
    if (!shapedTextInfo->LayoutProcessedLine.empty() && shapedTextInfo->LayoutLineBreak == TextLineBreakTypes::None &&
        shapedTextInfo->LayoutParagraphInnerLineGap == style.LayoutStyle.ParagraphInnerLineGap &&
        shapedTextInfo->LayoutMarginBeforeParagraph == style.LayoutStyle.MarginBeforeParagraph &&
        shapedTextInfo->LayoutMarginAfterParagraph == style.LayoutStyle.MarginAfterParagraph)
    {
        return glm::vec2 { shapedTextInfo->LayoutWidth, shapedTextInfo->LayoutHeight };
    }

    // 否则，快速计算大小
    float totalWidth = 0.f;
    float totalHeight = 0.f;
    float lastParaMarginBottom = 0.f;
    for (const auto& para : shapedTextInfo->ParagraphMeasures)
    {
        auto marginTop = std::max(CalcLineGap(style.LayoutStyle.MarginBeforeParagraph, para.LineHeight),
            lastParaMarginBottom);

        // 宽度取最大
        totalWidth = std::max(totalWidth, para.LineWidth);

        // 高度，需要计算段前+行高+段后
        float paraBeforeAddition = marginTop - lastParaMarginBottom;  // 上一行的 MarginBottom 已经计算在内
        lastParaMarginBottom = CalcLineGap(style.LayoutStyle.MarginAfterParagraph, para.LineHeight);
        totalHeight += paraBeforeAddition + para.LineHeight + lastParaMarginBottom;
    }
    return glm::vec2 { totalWidth, totalHeight };
}

bool TextDrawing::IsAllParagraphFitInLine(const std::vector<ParagraphMeasureInfo>& paragraphs, float maxWidth) noexcept
{
    for (const auto& e : paragraphs)
    {
        if (e.LineWidth > maxWidth)
            return false;
    }
    return true;
}

Result<TextDrawing::ShapedTextInfo*> TextDrawing::ShapeText(TextDrawing::ShapedTextCache& cache, Font::FontCollectionPtr collection,
    Font::ITextShaper* shaper, std::string_view text, const TextDrawingStyle& style) noexcept
{
    // 先查 Cache 看有没有预先整形的数据
    auto shapedTextInfo = cache.FindCache(text, collection.get(), style.FontSize, style.FontScale);
    if (!shapedTextInfo)  // 没有命中 Cache，进行整形
    {
        ShapedTextInfo info;
        info.Collection = std::move(collection);  // 持有对相关字体的引用，防止缓存数据过期

        // 转换到 u16string 才能进行进一步处理
        auto ret = Encoding::Convert<Encoding::Utf8, Encoding::Utf16>(info.Text, text, Encoding::DefaultUnicodeFallbackHandler);
        if (!ret)
            return ret.GetError();

        // 整形
        ret = shaper->ShapeText(info.ShapedGlyphs, info.Text, info.Collection.get(), style.FontSize, style.FontScale);
        if (!ret)
            return ret.GetError();

        // 计算段落度量值
        try
        {
            auto lastLineId = static_cast<uint32_t>(0);
            Font::IFontFace* lastFontFace = nullptr;
            ParagraphMeasureInfo measureInfo;
            for (size_t i = 0; i < info.ShapedGlyphs.size(); ++i)
            {
                const auto& glyph = info.ShapedGlyphs[i];
                auto currentLine = glyph.LineNumber;
                if (currentLine != lastLineId)
                {
                    assert(currentLine >= lastLineId + 1);
                    measureInfo.GlyphCount = i - measureInfo.GlyphStartIndex;
                    info.ParagraphMeasures.push_back(measureInfo);
                    while (info.ParagraphMeasures.size() < currentLine)  // 出现行号跳变的时候补全中间的行，一般情况下应该不会出现
                        info.ParagraphMeasures.push_back({ 0, 0, 0, 0, 0, i, 0});

                    lastLineId = currentLine;
                    lastFontFace = nullptr;
                    measureInfo = {};
                    measureInfo.GlyphStartIndex = i;
                }

                // 获取字体度量信息
                assert(glyph.FontFace);
                if (lastFontFace != glyph.FontFace)  // 这里用 lastFontFace 减少连续字体的度量信息检查，如果一行内出现多个字体切换则效率低下
                {
                    lastFontFace = glyph.FontFace;
                    auto metrics = lastFontFace->GetLayoutMetrics(glyph.Param, Font::FontLayoutDirection::Horizontal);
                    if (metrics)
                    {
                        auto ascender = Font::Q26D6ToPixelF(metrics->Ascender);
                        auto descender = Font::Q26D6ToPixelF(metrics->Descender);
                        auto lineGap = Font::Q26D6ToPixelF(metrics->LineGap);
                        measureInfo.Ascender = std::max(measureInfo.Ascender, ascender);
                        measureInfo.Descender = std::max(measureInfo.Descender, descender);
                        measureInfo.LineGap = std::max(measureInfo.LineGap, lineGap);
                        auto lineHeight = measureInfo.Ascender + measureInfo.Descender + measureInfo.LineGap;
                        measureInfo.LineHeight = lineHeight;
                    }
                }

                // 使用前进量来计算行宽
                measureInfo.LineWidth += glyph.XAdvance;
            }
            measureInfo.GlyphCount = info.ShapedGlyphs.size() - measureInfo.GlyphStartIndex;
            info.ParagraphMeasures.push_back(measureInfo);
        }
        catch (...)  // bad_alloc
        {
            return make_error_code(errc::not_enough_memory);
        }

        auto cacheRet = cache.Cache(text, info.Collection.get(), style.FontSize, style.FontScale, std::move(info));
        if (!cacheRet)
            return cacheRet.GetError();
        shapedTextInfo = *cacheRet;
    }
    assert(shapedTextInfo);
    return shapedTextInfo;
}

Result<void> TextDrawing::LayoutText(ShapedTextInfo* shapedTextInfo, const Math::XYRectangle& rect, const TextDrawingStyle& style) noexcept
{
    assert(shapedTextInfo);

    // 文本简单整形
    // 施加行间距、段落间距、处理折行
    if (shapedTextInfo->LayoutProcessedLine.empty() || shapedTextInfo->LayoutMaxWidth != rect.Width() ||
        shapedTextInfo->LayoutLineBreak != style.LayoutStyle.LineBreak ||
        shapedTextInfo->LayoutParagraphInnerLineGap != style.LayoutStyle.ParagraphInnerLineGap ||
        shapedTextInfo->LayoutMarginBeforeParagraph != style.LayoutStyle.MarginBeforeParagraph ||
        shapedTextInfo->LayoutMarginAfterParagraph != style.LayoutStyle.MarginAfterParagraph)
    {
        // 如果上次缓存的结果不是期望的或者没有缓存，则重新计算
        try
        {
            float totalWidth = 0.f;
            float totalHeight = 0.f;
            vector<ProcessedLineMeasureInfo> processedLines;
            processedLines.reserve(shapedTextInfo->ParagraphMeasures.size());

            if (style.LayoutStyle.LineBreak == TextLineBreakTypes::None ||
                IsAllParagraphFitInLine(shapedTextInfo->ParagraphMeasures, rect.Width()))
            {
                // 无折行时快速计算
                float lastParaMarginBottom = 0.f;
                for (const auto& para : shapedTextInfo->ParagraphMeasures)
                {
                    ProcessedLineMeasureInfo measureInfo;
                    measureInfo.MarginTop = std::max(CalcLineGap(style.LayoutStyle.MarginBeforeParagraph, para.LineHeight),
                        lastParaMarginBottom);
                    measureInfo.LineWidth = para.LineWidth;
                    measureInfo.LineHeight = para.LineHeight;
                    measureInfo.DrawPosTop = para.Ascender + para.LineGap / 2.f;
                    measureInfo.GlyphRunCount = 1;
                    measureInfo.GlyphRuns[0] = { para.GlyphStartIndex, para.GlyphCount };

                    // 宽度取最大
                    totalWidth = std::max(totalWidth, measureInfo.LineWidth);

                    // 高度，需要计算段前+行高+段后
                    float paraBeforeAddition = measureInfo.MarginTop - lastParaMarginBottom;  // 上一行的 MarginBottom 已经计算在内
                    lastParaMarginBottom = CalcLineGap(style.LayoutStyle.MarginAfterParagraph, para.LineHeight);
                    totalHeight += paraBeforeAddition + para.LineHeight + lastParaMarginBottom;

                    processedLines.push_back(measureInfo);
                }
            }
            else
            {
                const auto& glyphs = shapedTextInfo->ShapedGlyphs;

                // 带折行的时候需要处理 LTR、RTL 不同的文本走向并进行折行处理
                float lastParaMarginBottom = 0.f;
                for (const auto& para : shapedTextInfo->ParagraphMeasures)
                {
                    ProcessedLineMeasureInfo measureInfo;
                    measureInfo.MarginTop = std::max(CalcLineGap(style.LayoutStyle.MarginBeforeParagraph, para.LineHeight),
                        lastParaMarginBottom);
                    measureInfo.LineWidth = 0;
                    measureInfo.LineHeight = para.LineHeight;
                    measureInfo.DrawPosTop = para.Ascender + para.LineGap / 2.f;
                    measureInfo.GlyphRunCount = 0;

                    auto appendGlyphRun = [&](std::tuple<size_t, size_t> run) {
                        // 看看能不能直接接到上一个 Run
                        if (measureInfo.GlyphRunCount > 0)
                        {
                            auto& lastRun = measureInfo.GlyphRuns[measureInfo.GlyphRunCount - 1];
                            if (std::get<0>(run) == std::get<0>(lastRun) + std::get<1>(lastRun))
                            {
                                std::get<1>(lastRun) += std::get<1>(run);
                                return;
                            }
                        }

                        // 创建一个新的 Run
                        assert(measureInfo.GlyphRunCount < 3);
                        auto& nextRun = measureInfo.GlyphRuns[measureInfo.GlyphRunCount++];
                        nextRun = run;
                    };
                    auto newInnerLine = [&]() {
                        // 计入上一行
                        totalWidth = std::max(totalWidth, measureInfo.LineWidth);  // 宽度取最大

                        // 高度，需要计算段前+行高+段后
                        float paraBeforeAddition = measureInfo.MarginTop - lastParaMarginBottom;  // 上一行的 MarginBottom 已经计算在内
                        lastParaMarginBottom = CalcLineGap(style.LayoutStyle.ParagraphInnerLineGap, para.LineHeight);  // 注意这里是行间距
                        totalHeight += paraBeforeAddition + para.LineHeight + lastParaMarginBottom;

                        processedLines.push_back(measureInfo);

                        // 创建新的内部行
                        measureInfo.MarginTop = std::max(CalcLineGap(style.LayoutStyle.ParagraphInnerLineGap, para.LineHeight),
                            lastParaMarginBottom);
                        measureInfo.LineWidth = 0;
                        measureInfo.LineHeight = para.LineHeight;
                        measureInfo.DrawPosTop = para.Ascender + para.LineGap / 2.f;
                        measureInfo.GlyphRunCount = 0;
                    };
                    auto fillGlyphs = [&](size_t startIndex, size_t count) -> tuple<float, tuple<size_t, size_t>> {
                        assert(count > 0);
                        auto direction = glyphs[startIndex].Direction;
                        assert(direction != Font::TextDirection::Mixed);

                        float width = 0.f;
                        if (direction == Font::TextDirection::LeftToRight)
                        {
                            for (size_t i = 0; i < count; ++i)
                            {
                                const auto& glyph = glyphs[startIndex + i];

                                // 检查放下这个 glyph 之后是否会写满这行
                                // 为了防止无限循环，我们总是保证这个行至少放下一个字符
                                if (!(width == 0.f && measureInfo.LineWidth == 0.f) &&
                                    glyph.XAdvance + width + measureInfo.LineWidth > rect.Width())
                                {
                                    return {width, {startIndex, i}};
                                }

                                width += glyph.XAdvance;
                            }
                        }
                        else
                        {
                            for (size_t i = 0; i < count; ++i)  // RTL 需要倒着处理
                            {
                                const auto& glyph = glyphs[startIndex + count - (i + 1)];

                                // 检查放下这个 glyph 之后是否会写满这行
                                // 为了防止无限循环，我们总是保证这个行至少放下一个字符
                                if (!(width == 0.f && measureInfo.LineWidth == 0.f) &&
                                    glyph.XAdvance + width + measureInfo.LineWidth > rect.Width())
                                {
                                    return {width, {startIndex + count - i, i}};
                                }

                                width += glyph.XAdvance;
                            }
                        }
                        return {width, {startIndex, count}};
                    };

                    size_t restGlyphs = para.GlyphCount;
                    size_t nextRunStart = para.GlyphStartIndex;
                    size_t nextRunLength = 0;
                    while (restGlyphs > 0)
                    {
                        // 先按文本方向切割
                        for (size_t i = 0; i < restGlyphs; ++i)
                        {
                            if (glyphs[nextRunStart].Direction != glyphs[nextRunStart + i].Direction)
                                break;
                            ++nextRunLength;
                        }
                        assert(nextRunLength > 0);
                        assert(nextRunLength <= restGlyphs);

                        // 尝试排列文字
                        size_t subRunStart = nextRunStart;
                        size_t subRunLength = nextRunLength;
                        while (subRunLength > 0)
                        {
                            auto [filledWidth, filledRun] = fillGlyphs(subRunStart, subRunLength);
                            bool newLine = std::get<1>(filledRun) < subRunLength;
                            if (std::get<1>(filledRun) > 0)
                            {
                                measureInfo.LineWidth += filledWidth;
                                appendGlyphRun(filledRun);

                                // 从 subRun 中剔除
                                if (std::get<0>(filledRun) == subRunStart)
                                {
                                    assert(std::get<1>(filledRun) <= subRunLength);
                                    subRunStart += std::get<1>(filledRun);
                                    subRunLength -= std::get<1>(filledRun);
                                }
                                else
                                {
                                    // 如果不是开头对齐的，一定是 RTL 走向，此时消费的是右端开始的 Run
                                    assert(glyphs[nextRunStart].Direction == Font::TextDirection::RightToLeft);
                                    assert(subRunStart == nextRunStart);
                                    assert(std::get<0>(filledRun) + std::get<1>(filledRun) == subRunStart + subRunLength);
                                    subRunLength -= std::get<1>(filledRun);
                                }
                            }
                            if (newLine)
                            {
                                // 没填够，此时需要创建新行
                                newInnerLine();
                            }
                        }

                        // 继续下一个文本分段
                        nextRunStart += nextRunLength;
                        restGlyphs -= nextRunLength;
                        nextRunLength = 0;
                    }

                    // 把最后的这行计入
                    totalWidth = std::max(totalWidth, measureInfo.LineWidth);  // 宽度取最大

                    // 高度，需要计算段前+行高+段后
                    float paraBeforeAddition = measureInfo.MarginTop - lastParaMarginBottom;  // 上一行的 MarginBottom 已经计算在内
                    lastParaMarginBottom = CalcLineGap(style.LayoutStyle.MarginAfterParagraph, para.LineHeight);
                    totalHeight += paraBeforeAddition + para.LineHeight + lastParaMarginBottom;

                    processedLines.push_back(measureInfo);
                }
            }

            // 上面计算成功后，替换 Cache
            shapedTextInfo->LayoutMaxWidth = rect.Width();
            shapedTextInfo->LayoutLineBreak = style.LayoutStyle.LineBreak;
            shapedTextInfo->LayoutParagraphInnerLineGap = style.LayoutStyle.ParagraphInnerLineGap;
            shapedTextInfo->LayoutMarginBeforeParagraph = style.LayoutStyle.MarginBeforeParagraph;
            shapedTextInfo->LayoutMarginAfterParagraph = style.LayoutStyle.MarginAfterParagraph;
            shapedTextInfo->LayoutWidth = totalWidth;
            shapedTextInfo->LayoutHeight = totalHeight;
            shapedTextInfo->LayoutProcessedLine = std::move(processedLines);
        }
        catch (...)  // bad_alloc
        {
            return make_error_code(errc::not_enough_memory);
        }
    }
    return {};
}

// </editor-fold>
