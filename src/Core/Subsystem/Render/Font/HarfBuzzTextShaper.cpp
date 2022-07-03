/**
 * @file
 * @date 2022/6/27
 * @author 9chu
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#include "HarfBuzzTextShaper.hpp"

#include <ubidiimp.h>
#include <unicode/locid.h>
#include <unicode/ustring.h>
#include <unicode/brkiter.h>
#include "../../../detail/IcuError.hpp"
#include "../../../detail/IcuCharacterIteratorBridge.hpp"
#include "detail/Helper.hpp"

using namespace std;
using namespace lstg;
using namespace lstg::Subsystem::Render::Font;

namespace
{
    constexpr inline UBiDiLevel ToParagraphDirection(TextDirection direction) noexcept
    {
        assert(direction != TextDirection::Mixed);
        return direction == TextDirection::LeftToRight ? UBIDI_DEFAULT_LTR : UBIDI_DEFAULT_RTL;
    }

    inline TextDirection FromBidiDirection(UBiDiDirection direction) noexcept
    {
        switch (direction)
        {
            default:
            case UBIDI_LTR:
            case UBIDI_NEUTRAL:
                return TextDirection::LeftToRight;
            case UBIDI_RTL:
                return TextDirection::RightToLeft;
            case UBIDI_MIXED:
                return TextDirection::Mixed;
        }
    }

    constexpr inline bool IsSpecialScript(hb_script_t script) noexcept
    {
        return script == HB_SCRIPT_COMMON || script == HB_SCRIPT_INHERITED || script == HB_SCRIPT_UNKNOWN;
    }

    void UpdateGlyphCountsForRange(FontShapedGlyph& out, icu::BreakIterator* breaker, size_t startIndex, size_t endIndex) noexcept
    {
        assert(startIndex <= endIndex);

        if (out.GlyphCharacterCount == 0 && out.GlyphGraphemeClusterCount == 0)  // 这个判断似乎多余
        {
            out.GlyphCharacterCount = endIndex - startIndex;

            if (out.GlyphCharacterCount > 0)
            {
                auto previousBreak = startIndex;
                breaker->following(static_cast<int32_t>(startIndex));
                previousBreak = breaker->previous();

                // 只有在边界上才进行计算
                if (previousBreak == startIndex)
                {
                    auto currentBreak = static_cast<int32_t>(endIndex);
                    for (currentBreak = breaker->following(static_cast<int32_t>(startIndex)); currentBreak != UBRK_DONE;
                        currentBreak = breaker->next())
                    {
                        ++out.GlyphGraphemeClusterCount;
                        if (currentBreak >= static_cast<int32_t>(endIndex))
                            break;
                    }

                    // 必须在边界上才进行计算
                    if (currentBreak != static_cast<int32_t>(endIndex))
                        out.GlyphGraphemeClusterCount = 0;
                }
            }
        }
    }

    size_t FindNextGlyphInRenderIndex(const std::vector<FontShapedGlyph>& output, size_t index) noexcept
    {
        auto& entry = output[index];
        auto next = index + 1;

        for (; next < output.size(); ++next)
        {
            auto& nextEntry = output[next];
            if (nextEntry.StartIndex != entry.StartIndex)  // 由于书写顺序的关系，注意这里使用不等号
                break;
        }
        return next;
    }
}

bool HarfBuzzTextShaper::HandleSpecialCharacter(std::vector<FontShapedGlyph>& output, const ProcessingTextRun& run,
    const FontGlyphRasterParam& fontParam, const hb_glyph_info_t& glyphInfo, const hb_glyph_position_t& glyphPosition,
    size_t currentCharIndex, char32_t currentChar)
{
    // TAB
    if (currentChar == '\t')
    {
        // 获取空格字符
        char32_t spaceChar = ' ';
        FontGlyphId spaceGlyphId = 0;
        if (0 == run.Font->BatchGetNominalGlyphs({ &spaceGlyphId, 1, sizeof(spaceGlyphId) }, { &spaceChar, 1, sizeof(spaceChar) }))
            return false;  // 没有合适的字形

        // 查找前进量
        Q26D6 spaceAdvance = 0;
        run.Font->BatchGetAdvances({ &spaceAdvance, 1, sizeof(spaceAdvance) }, fontParam, FontLayoutDirection::Horizontal,
            { &spaceGlyphId, 1, sizeof(spaceGlyphId) });

        FontShapedGlyph shapedGlyph;
        shapedGlyph.FontFace = run.Font.get();
        shapedGlyph.Param = fontParam;
        shapedGlyph.GlyphIndex = spaceGlyphId;
        shapedGlyph.StartIndex = currentCharIndex;
        shapedGlyph.LineNumber = run.LineNumber;
        shapedGlyph.XAdvance = Q26D6ToPixelF(spaceAdvance) * 4;  // 直接按照 4 空格计算
        shapedGlyph.YAdvance = 0;
        shapedGlyph.XOffset = 0;
        shapedGlyph.YOffset = 0;
        shapedGlyph.Kerning = 0;
        shapedGlyph.GlyphCharacterCount = 1;
        shapedGlyph.GlyphGraphemeClusterCount = 1;
        shapedGlyph.Direction = run.Direction;
        shapedGlyph.Visible = false;
        output.push_back(shapedGlyph);
        return true;
    }

    // 控制字符
    if (detail::Helper::IsControlCharacter(currentChar))
    {
        FontShapedGlyph shapedGlyph;
        shapedGlyph.FontFace = run.Font.get();
        shapedGlyph.Param = fontParam;
        shapedGlyph.GlyphIndex = 0;
        shapedGlyph.StartIndex = currentCharIndex;
        shapedGlyph.LineNumber = run.LineNumber;
        shapedGlyph.XAdvance = 0;
        shapedGlyph.YAdvance = 0;
        shapedGlyph.XOffset = 0;
        shapedGlyph.YOffset = 0;
        shapedGlyph.Kerning = 0;
        shapedGlyph.GlyphCharacterCount = 1;
        shapedGlyph.GlyphGraphemeClusterCount = 1;
        shapedGlyph.Direction = run.Direction;
        shapedGlyph.Visible = false;
        output.push_back(shapedGlyph);
        return true;
    }
    return false;
}

HarfBuzzTextShaper::HarfBuzzTextShaper()
{
    auto& inst = lstg::detail::IcuService::GetInstance();

    // 创建 UBiDi
    m_pBidi = std::move(inst.CreateBidi().ThrowIfError());

    // 创建字符切分器
    m_pGraphemeBreakIter = std::move(inst.CreateGraphemeBreakIterator().ThrowIfError());

    // 创建 HarfBuzz 缓冲区
    auto buffer = detail::HarfBuzzBridge::CreateBuffer();
    m_pHBBuffer = std::move(buffer.ThrowIfError());
    assert(m_pHBBuffer);
}

Result<void> HarfBuzzTextShaper::ShapeText(std::vector<FontShapedGlyph>& output, std::u16string_view input,
    FontCollection* collection, uint32_t fontSize, TextDirection baseDirection) noexcept
{
    output.clear();

    // 对文本进行预处理
    try
    {
        ProcessingTextRun firstRun;
        firstRun.StartIndex = 0;
        firstRun.Length = input.length();

        m_stTmpBuffers[0].clear();
        m_stTmpBuffers[0].push_back(std::move(firstRun));

        // Step1: 文本分段
        BreakParagraph(m_stTmpBuffers[1], input, m_stTmpBuffers[0]);

        // Step2: 书写方向分段
        auto ret = BreakDirection(m_stTmpBuffers[0], input, m_stTmpBuffers[1], m_pBidi.get(), baseDirection);
        if (!ret)
            return ret.GetError();

        // Step3: 选择各个分块的字体
        ChooseFont(m_stTmpBuffers[1], input, m_stTmpBuffers[0], *collection);

        // Step4: 语言系统分段
        SplitScript(m_stTmpBuffers[0], input, m_stTmpBuffers[1]);

        // 最终结果位于 m_stTmpBuffers[0]

        // 准备 GraphemeBreak 用于后面计算字符数
        m_pGraphemeBreakIter->adoptText(new lstg::detail::IcuCharacterIteratorBridge(input));  // 由 BreakIterator 释放 CharacterIterator
    }
    catch (...)  // bad_alloc
    {
        return make_error_code(errc::not_enough_memory);
    }

    // 执行排版操作
    ::hb_buffer_clear_contents(m_pHBBuffer.get());
    for (const auto& run : m_stTmpBuffers[0])
    {
        // 准备字体
        FontGlyphRasterParam fontParam {};
        fontParam.Size = { static_cast<int32_t>(fontSize), run.FontScaling };
        fontParam.Flags = run.Font->MakeGlyphRasterFlags();
        auto hbFont = CreateHBFont(run.Font, fontParam);
        if (!hbFont)
            return hbFont.GetError();
        auto& font = **hbFont;

        // 排版特性
        hb_feature_t harfBuzzFeatures[] = {
            { HB_TAG('k','e','r','n'), run.Font->HasKerning(), 0, static_cast<uint32_t>(-1) },
            { HB_TAG('l','i','g','a'), true, 0, static_cast<uint32_t>(-1) },
        };
        auto harfBuzzFeaturesCount = std::extent_v<decltype(harfBuzzFeatures)>;

        for (const auto& seq : run.SubSequences)
        {
            auto lastShapedCount = output.size();

            // 准备HarfBuzz
            ::hb_buffer_set_cluster_level(m_pHBBuffer.get(), HB_BUFFER_CLUSTER_LEVEL_MONOTONE_GRAPHEMES);
            ::hb_buffer_set_direction(m_pHBBuffer.get(), (run.Direction == TextDirection::LeftToRight) ? HB_DIRECTION_LTR :
                HB_DIRECTION_RTL);
            ::hb_buffer_set_script(m_pHBBuffer.get(), seq.Script);

            // Shape it!
            ::hb_buffer_add_utf16(m_pHBBuffer.get(), reinterpret_cast<const uint16_t*>(input.data() + seq.StartIndex), seq.Length, 0,
                seq.Length);
            ::hb_shape(font.get(), m_pHBBuffer.get(), harfBuzzFeatures, harfBuzzFeaturesCount);

            // 获取结果
            uint32_t glyphCount = 0u;
            auto glyphInfoArray = ::hb_buffer_get_glyph_infos(m_pHBBuffer.get(), &glyphCount);
            auto glyphPositionArray = ::hb_buffer_get_glyph_positions(m_pHBBuffer.get(), &glyphCount);

            // 输出结果
            try
            {
                output.reserve(output.size() + glyphCount);
                for (uint32_t i = 0; i < glyphCount; ++i)
                {
                    const auto& glyphInfo = glyphInfoArray[i];
                    const auto& glyphPosition = glyphPositionArray[i];

                    // 一个cluster可能映射到单个codepoint，也可能多个codepoint映射到一个cluster
                    auto currentCharIndex = glyphInfo.cluster + seq.StartIndex;
                    auto currentChar = input[currentCharIndex];

                    // 处理特殊字符
                    if (!HandleSpecialCharacter(output, run, fontParam, glyphInfo, glyphPosition, currentCharIndex, currentChar))
                    {
                        FontShapedGlyph shapedGlyph;
                        shapedGlyph.FontFace = run.Font.get();
                        shapedGlyph.Param = fontParam;
                        shapedGlyph.GlyphIndex = glyphInfo.codepoint;
                        shapedGlyph.StartIndex = currentCharIndex;
                        shapedGlyph.LineNumber = run.LineNumber;
                        shapedGlyph.XAdvance = Q26D6ToPixelF(glyphPosition.x_advance);
                        shapedGlyph.YAdvance = Q26D6ToPixelF(glyphPosition.y_advance);
                        shapedGlyph.XOffset = Q26D6ToPixelF(glyphPosition.x_offset);
                        shapedGlyph.YOffset = Q26D6ToPixelF(glyphPosition.y_offset);
                        shapedGlyph.Kerning = 0;
                        shapedGlyph.GlyphCharacterCount = 0;
                        shapedGlyph.GlyphGraphemeClusterCount = 0;
                        shapedGlyph.Direction = run.Direction;
                        shapedGlyph.Visible = !detail::Helper::IsRenderAsWhitespace(currentChar);

                        // 计算Kerning
                        if (!output.empty() && run.Font->HasKerning())
                        {
                            auto& previousShapedGlyph = output.back();
                            if (previousShapedGlyph.Visible)
                            {
                                // FIXME: 支持纵排需要修改
                                auto kerning = run.Font->GetGlyphKerning(fontParam, FontLayoutDirection::Horizontal,
                                    FontGlyphPair{previousShapedGlyph.GlyphIndex, shapedGlyph.GlyphIndex});
                                shapedGlyph.Kerning = Q26D6ToPixelF(kerning);
                            }
                        }

                        output.emplace_back(shapedGlyph);
                    }
                }
            }
            catch (...)  // bad_alloc
            {
                return make_error_code(errc::not_enough_memory);
            }

            ::hb_buffer_clear_contents(m_pHBBuffer.get());

            // 计算每一个字形对应了多少个字符
            {
                auto shapedCount = output.size() - lastShapedCount;
                if (shapedCount > 0)
                {
                    // 在最终输出结果中，总是按照渲染顺序排序的，因此计算相邻两个字符的时候，计算方式有所不同
                    if (run.Direction == TextDirection::LeftToRight)
                    {
                        for (size_t i = lastShapedCount; i < output.size(); )
                        {
                            auto& shapedGlyph = output[i];
                            auto next = FindNextGlyphInRenderIndex(output, i);
                            if (next < output.size())
                            {
                                auto& nextShapedGlyph = output[next];
                                UpdateGlyphCountsForRange(shapedGlyph, m_pGraphemeBreakIter.get(), shapedGlyph.StartIndex,
                                    nextShapedGlyph.StartIndex);
                            }
                            else
                            {
                                UpdateGlyphCountsForRange(shapedGlyph, m_pGraphemeBreakIter.get(), shapedGlyph.StartIndex,
                                    seq.StartIndex + seq.Length);
                            }
                            i = next;
                        }
                    }
                    else
                    {
                        assert(run.Direction == TextDirection::RightToLeft);
                        auto lastIndex = seq.StartIndex + seq.Length;
                        for (size_t i = lastShapedCount; i < output.size(); )
                        {
                            // 注意这里是倒序
                            auto& shapedGlyph = output[i];
                            UpdateGlyphCountsForRange(shapedGlyph, m_pGraphemeBreakIter.get(), shapedGlyph.StartIndex, lastIndex);
                            i = FindNextGlyphInRenderIndex(output, i);
                            lastIndex = shapedGlyph.StartIndex;
                        }
                    }
                }
            }
        }
    }
    return {};
}

void HarfBuzzTextShaper::BreakParagraph(std::vector<ProcessingTextRun>& output, std::u16string_view text,
    const std::vector<ProcessingTextRun>& input)
{
    output.clear();

    size_t lineNumber = 0;
    for (const auto& run : input)
    {
        enum {
            STATE_LOOK_CR_OR_LF,
            STATE_LOOK_LF,
        } state = STATE_LOOK_CR_OR_LF;
        size_t currentStartIndex = run.StartIndex;
        size_t currentLength = 0;

        auto emitRun = [&](uint32_t line) {
            ProcessingTextRun info;
            info.StartIndex = currentStartIndex;
            info.Length = currentLength;
            info.LineNumber = line;
            output.push_back(info);
        };

        for (size_t i = 0; i < run.Length; )
        {
            auto ch = text[run.StartIndex + i];

            switch (state)
            {
                case STATE_LOOK_CR_OR_LF:
                    currentLength += 1;
                    if (ch == '\r')
                    {
                        state = STATE_LOOK_LF;
                    }
                    else if (ch == '\n')
                    {
                        emitRun(lineNumber++);
                        currentStartIndex = run.StartIndex + i + 1;
                        currentLength = 0;
                    }
                    break;
                case STATE_LOOK_LF:
                    if (ch == '\n')
                    {
                        currentLength += 1;

                        emitRun(lineNumber++);
                        state = STATE_LOOK_CR_OR_LF;
                        currentStartIndex = run.StartIndex + i + 1;
                        currentLength = 0;
                    }
                    else
                    {
                        emitRun(lineNumber++);
                        state = STATE_LOOK_CR_OR_LF;
                        currentStartIndex = run.StartIndex + i;
                        currentLength = 0;
                        continue;  // retry
                    }
                    break;
                default:
                    assert(false);
                    break;
            }

            ++i;
        }

        if (currentLength > 0)
            emitRun(lineNumber);
    }
}

Result<void> HarfBuzzTextShaper::BreakDirection(std::vector<ProcessingTextRun>& output, std::u16string_view text,
    const std::vector<ProcessingTextRun>& input, UBiDi* bidi, TextDirection baseDirection)
{
    assert(bidi);
    assert(baseDirection != TextDirection::Mixed);

    output.clear();
    auto defaultParaDirection = ToParagraphDirection(baseDirection);

    for (const auto& run : input)
    {
        UErrorCode status = U_ZERO_ERROR;
        ::ubidi_setPara(bidi, &text[run.StartIndex], run.Length, defaultParaDirection, nullptr, &status);
        if (U_FAILURE(status))
            return make_error_code(status);

        auto bidiRuns = ::ubidi_countRuns(bidi, &status);
        if (U_FAILURE(status))
            return make_error_code(status);

        for (auto i = 0; i < bidiRuns; ++i)
        {
            int32_t startIndex = 0;
            int32_t length = 0;
            auto runDirection = ::ubidi_getVisualRun(bidi, i, &startIndex, &length);

            ProcessingTextRun bidiRun;
            bidiRun.LineNumber = run.LineNumber;
            bidiRun.StartIndex = run.StartIndex + startIndex;
            bidiRun.Length = length;
            bidiRun.Direction = FromBidiDirection(runDirection);
            output.push_back(bidiRun);
        }
    }
    return {};
}

void HarfBuzzTextShaper::ChooseFont(std::vector<ProcessingTextRun>& output, std::u16string_view text,
    const std::vector<ProcessingTextRun>& input, FontCollection& collection)
{
    output.clear();

    // 切割输入文本， 使得切割后的文本段具备唯一的字体进行渲染
    for (const auto& run : input)
    {
        auto runText = std::u16string_view { text.data() + run.StartIndex, run.Length };
        m_pGraphemeBreakIter->adoptText(new lstg::detail::IcuCharacterIteratorBridge(runText));  // 由 BreakIterator 释放 CharacterIterator

        int32_t lastBreakIndex = 0;
        int32_t currentBreakIndex = 0;

        int32_t runningPosition = run.StartIndex;
        int32_t runningSegmentStartIndex = run.StartIndex;
        FontFacePtr runningSegmentFont;
        float runningSegmentFontScale = 1.f;

        auto emitRun = [&]() {
            // 极端情况下如果没有字体可以渲染这个字符和代替字符，将造成这个文本丢失掉
            if (runningSegmentFont)
            {
                ProcessingTextRun newRun;
                newRun.StartIndex = runningSegmentStartIndex;
                newRun.Length = runningPosition - runningSegmentStartIndex;
                newRun.LineNumber = run.LineNumber;
                newRun.Direction = run.Direction;
                newRun.Font = runningSegmentFont;
                newRun.FontScaling = runningSegmentFontScale;
                output.emplace_back(std::move(newRun));
            }
        };

        for (; (currentBreakIndex = m_pGraphemeBreakIter->next()) != UBRK_DONE; lastBreakIndex = currentBreakIndex)
        {
            auto clusterSize = currentBreakIndex - lastBreakIndex;

            // 获取字母码点，一般来说只要取 Cluster 的首字母进行检查即可
            char32_t codePoint = text[runningPosition];
            if (clusterSize > 1)
            {
                if (detail::Helper::IsHighSurrogate(codePoint) && detail::Helper::IsLowSurrogate(text[runningPosition + 1]))
                    codePoint = detail::Helper::EncodeSurrogate(codePoint, text[runningPosition + 1]);
            }

            // 让字体集合挑选可用于渲染的字体
            auto [selectedFont, selectedGlyphId, selectedFontScale] = collection.ChooseFontForChar(codePoint);
            if (!selectedFont)
            {
                // 如果没有找到合适的字体，尝试用其他字符代替
                codePoint = detail::Helper::IsRenderAsWhitespace(codePoint) ? ' ' : detail::kInvalidCharCodePoint;
                auto fallbackSelected = collection.ChooseFontForChar(codePoint);
                selectedFont = std::move(std::get<0>(fallbackSelected));
                selectedGlyphId = std::get<1>(fallbackSelected);
                selectedFontScale = std::get<2>(fallbackSelected);
            }

            // 如果字体出现了变化，我们创建一个新的 Segment
            if (selectedFont != runningSegmentFont || selectedFontScale != runningSegmentFontScale)
            {
                emitRun();
                runningSegmentStartIndex = runningPosition;
                runningSegmentFont = selectedFont;
                runningSegmentFontScale = selectedFontScale;
            }

            runningPosition += clusterSize;
        }

        if (runningPosition > runningSegmentStartIndex)
            emitRun();
    }
}

void HarfBuzzTextShaper::SplitScript(std::vector<ProcessingTextRun>& output, std::u16string_view text,
    const std::vector<ProcessingTextRun>& input)
{
    output.clear();

    auto unicodeFuncs = ::hb_unicode_funcs_get_default();
    for (const auto& run : input)
    {
        ProcessingTextRun newRun = run;

        size_t runningSegmentStartIndex = 0;
        std::optional<hb_script_t> runningSegmentScript;

        auto emitRun = [&](size_t lastIndex) {
            if (runningSegmentScript && lastIndex - runningSegmentStartIndex > 0)
            {
                ScriptedTextRun scriptRun;
                scriptRun.StartIndex = runningSegmentStartIndex;
                scriptRun.Length = lastIndex - runningSegmentStartIndex;
                scriptRun.Script = *runningSegmentScript;
                newRun.SubSequences.push_back(scriptRun);
            }
        };

        for (size_t i = 0; i < run.Length; ++i)
        {
            auto currentPos = run.StartIndex + i;

            char32_t codePoint = text[currentPos];
            if ((run.Length - currentPos) > 1)
            {
                if (detail::Helper::IsHighSurrogate(codePoint) && detail::Helper::IsLowSurrogate(text[currentPos + 1]))
                {
                    codePoint = detail::Helper::EncodeSurrogate(codePoint, text[currentPos + 1]);
                    ++i;
                }
            }

            auto script = ::hb_unicode_script(unicodeFuncs, codePoint);
            if (!runningSegmentScript || *runningSegmentScript != script)
            {
                if (!runningSegmentScript)
                {
                    runningSegmentStartIndex = currentPos;
                    runningSegmentScript = script;
                }
                else if (!IsSpecialScript(script))
                {
                    // 对于特殊的 Script，使用后继的 Script 进行合并，以减少分割数量
                    if (IsSpecialScript(*runningSegmentScript))
                    {
                        runningSegmentScript = script;
                    }
                    else
                    {
                        emitRun(currentPos);
                        runningSegmentStartIndex = currentPos + 1;
                        runningSegmentScript.reset();
                    }
                }
            }
        }
        emitRun(run.StartIndex + run.Length);

        if (!newRun.SubSequences.empty())
        {
            // 如果书写方向是 RTL，则翻转 Subsequence
            if (newRun.Direction == TextDirection::RightToLeft)
                std::reverse(newRun.SubSequences.begin(), newRun.SubSequences.end());

            output.emplace_back(std::move(newRun));
        }
    }
}

Result<lstg::Subsystem::Render::Font::detail::HarfBuzzBridge::FontPtr*> HarfBuzzTextShaper::CreateHBFont(FontFacePtr face,
    FontGlyphRasterParam param) noexcept
{
    // 查缓存
    HarfBuzzFontCacheKey cacheKey {};
    cacheKey.Face = face.get();
    cacheKey.Param = param;
    auto ret = m_stHBFontCache.TryGet(cacheKey);
    if (ret)
        return ret;

    // 创建对象
    auto hbFace = detail::HarfBuzzBridge::CreateFace(std::move(face));
    if (!hbFace)
        return hbFace.GetError();
    auto hbFont = detail::HarfBuzzBridge::CreateFont(std::move(*hbFace), param);
    if (!hbFont)
        return hbFace.GetError();

    // 写缓存
    try
    {
        return m_stHBFontCache.Emplace(cacheKey, std::move(*hbFont));
    }
    catch (...)
    {
        return make_error_code(errc::not_enough_memory);
    }
}
