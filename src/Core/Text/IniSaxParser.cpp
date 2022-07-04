/**
 * @file
 * @date 2022/7/3
 * @author 9chu
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#include <lstg/Core/Text/IniSaxParser.hpp>

#include <cassert>

using namespace std;
using namespace lstg;
using namespace lstg::Text;

namespace
{
    constexpr bool IsSpace(char ch) noexcept
    {
        return ch == ' ' || ch == '\t';
    }
}

void IniSaxParser::Parse(std::string_view content, IIniSaxListener* listener, IniParsingFlags flags)
{
    assert(listener);

    enum {
        STATE_GUESS_LINE_CONTENT,
        STATE_TAILING_COMMENT,
        STATE_READING_SECTION,
        STATE_READING_SECTION_LEFT,
        STATE_READING_KEY,
        STATE_READING_VALUE,
    } state = STATE_GUESS_LINE_CONTENT;

    size_t bufferStartIndex = 0;
    size_t bufferLength = 0;
    std::string_view pendingKey;
    bool firstSection = true;

    auto isCommentCharacter = [&](char ch) {
        return ch == ';' || ((flags & IniParsingFlags::UnixStyleComment) && ch == '#');
    };
    auto removeLeadingSpaces = [&]() {
        while (bufferLength > 0 && IsSpace(content[bufferStartIndex]))
        {
            ++bufferStartIndex;
            --bufferLength;
        }
    };
    auto removeTailingSpaces = [&]() {
        while (bufferLength > 0 && IsSpace(content[bufferStartIndex + bufferLength - 1]))
            --bufferLength;
    };
    auto commitSection = [&]() {
        if (flags & IniParsingFlags::IgnoreSectionLeadingSpaces)
            removeLeadingSpaces();
        if (flags & IniParsingFlags::IgnoreSectionTailingSpaces)
            removeTailingSpaces();
        string_view value = { content.data() + bufferStartIndex, bufferLength };
        listener->OnSectionBegin(value);
        firstSection = false;
    };
    auto commitKey = [&]() {
        if (flags & IniParsingFlags::IgnoreKeyLeadingSpaces)
            removeLeadingSpaces();
        if (flags & IniParsingFlags::IgnoreKeyTailingSpaces)
            removeTailingSpaces();
        pendingKey = { content.data() + bufferStartIndex, bufferLength };
    };
    auto commitValue = [&]() {
        if (flags & IniParsingFlags::IgnoreValueLeadingSpaces)
            removeLeadingSpaces();
        if (flags & IniParsingFlags::IgnoreValueTailingSpaces)
            removeTailingSpaces();
        string_view value = { content.data() + bufferStartIndex, bufferLength };
        listener->OnValue(pendingKey, value);
        pendingKey = {};
    };

    for (size_t i = 0; i < content.size() + 1; )
    {
        char ch = (i >= content.size() ? '\0' : content[i]);
        switch (state)
        {
            case STATE_GUESS_LINE_CONTENT:
                if (isCommentCharacter(ch))
                {
                    state = STATE_TAILING_COMMENT;
                    break;
                }
                else if (ch == '[')
                {
                    state = STATE_READING_SECTION;
                    bufferStartIndex = i + 1;
                    bufferLength = 0;
                    if (!firstSection)
                        listener->OnSectionEnd();
                    break;
                }
                else if (IsSpace(ch))
                {
                    ++bufferLength;
                    break;
                }
                else if (ch == '\r' || ch == '\n' || ch == '\0')
                {
                    bufferStartIndex = i + 1;
                    bufferLength = 0;
                    break;
                }
                else
                {
                    // ReadingKey 状态需要继承 bufferStartIndex 和 bufferLength
                    state = STATE_READING_KEY;
                    continue;
                }
            case STATE_TAILING_COMMENT:
                if (ch == '\r' || ch == '\n' || ch == '\0')
                {
                    state = STATE_GUESS_LINE_CONTENT;
                    bufferStartIndex = i + 1;
                    bufferLength = 0;
                }
                break;
            case STATE_READING_SECTION:
                if (ch == ']')
                {
                    state = STATE_READING_SECTION_LEFT;
                    commitSection();
                    bufferStartIndex = bufferLength = 0;
                    break;
                }
                else if (ch == '\r' || ch == '\n' || ch == '\0')
                {
                    // 针对 Section 声明没有正确结束，容错，按照 Section 结束处理
                    state = STATE_GUESS_LINE_CONTENT;
                    commitSection();
                    bufferStartIndex = i + 1;
                    bufferLength = 0;
                    break;
                }
                ++bufferLength;
                break;
            case STATE_READING_SECTION_LEFT:
                // 简单处理，直接忽略行所有剩余内容
                if (ch == '\r' || ch == '\n' || ch == '\0')
                {
                    state = STATE_GUESS_LINE_CONTENT;
                    bufferStartIndex = i + 1;
                    bufferLength = 0;
                }
                break;
            case STATE_READING_KEY:
                if (ch == '=')
                {
                    state = STATE_READING_VALUE;
                    commitKey();
                    bufferStartIndex = i + 1;
                    bufferLength = 0;
                    break;
                }
                else if (isCommentCharacter(ch) || (ch == '\r' || ch == '\n' || ch == '\0'))
                {
                    // 此时直接当 Value 处理
                    assert(pendingKey.empty());
                    state = STATE_READING_VALUE;
                    continue;
                }
                ++bufferLength;
                break;
            case STATE_READING_VALUE:
                if (isCommentCharacter(ch) && (flags & IniParsingFlags::RemoveCommentInValue))
                {
                    state = STATE_TAILING_COMMENT;
                    commitValue();
                    bufferStartIndex = bufferLength = 0;
                    break;
                }
                else if (ch == '\r' || ch == '\n' || ch == '\0')
                {
                    state = STATE_GUESS_LINE_CONTENT;
                    commitValue();
                    bufferStartIndex = i + 1;
                    bufferLength = 0;
                    break;
                }
                ++bufferLength;
                break;
            default:
                assert(false);
                break;
        }

        ++i;
    }

    if (!firstSection)
        listener->OnSectionEnd();
}
