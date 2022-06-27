/**
 * @file
 * @date 2017/5/28
 * @author 9chu
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#include <lstg/Core/Encoding/Unicode.hpp>

using namespace std;
using namespace lstg;
using namespace lstg::Encoding;

// <editor-fold desc="UTF-8">

const char* const Utf8::kName = "Utf8";

EncodingResult Utf8::Decoder::operator()(InputType ch, std::array<OutputType, kMaxOutputCount>& out, uint32_t& count) noexcept
{
    // http://bjoern.hoehrmann.de/utf-8/decoder/dfa/
    static const uint8_t kUtf8Dfa[] = {
        0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
        0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0, // 00..1f
        0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
        0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0, // 20..3f
        0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
        0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0, // 40..5f
        0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
        0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0, // 60..7f
        1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
        9,   9,   9,   9,   9,   9,   9,   9,   9,   9,   9,   9,   9,   9,   9,   9, // 80..9f
        7,   7,   7,   7,   7,   7,   7,   7,   7,   7,   7,   7,   7,   7,   7,   7,
        7,   7,   7,   7,   7,   7,   7,   7,   7,   7,   7,   7,   7,   7,   7,   7, // a0..bf
        8,   8,   2,   2,   2,   2,   2,   2,   2,   2,   2,   2,   2,   2,   2,   2,
        2,   2,   2,   2,   2,   2,   2,   2,   2,   2,   2,   2,   2,   2,   2,   2,   // c0..df
        0xa, 0x3, 0x3, 0x3, 0x3, 0x3, 0x3, 0x3, 0x3, 0x3, 0x3, 0x3, 0x3, 0x4, 0x3, 0x3, // e0..ef
        0xb, 0x6, 0x6, 0x6, 0x5, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, 0x8, // f0..ff
        0x0, 0x1, 0x2, 0x3, 0x5, 0x8, 0x7, 0x1, 0x1, 0x1, 0x4, 0x6, 0x1, 0x1, 0x1, 0x1, // s0..s0
        1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,
        1,   0,   1,   1,   1,   1,   1,   0,   1,   0,   1,   1,   1,   1,   1,   1, // s1..s2
        1,   2,   1,   1,   1,   1,   1,   2,   1,   2,   1,   1,   1,   1,   1,   1,
        1,   1,   1,   1,   1,   1,   1,   2,   1,   1,   1,   1,   1,   1,   1,   1, // s3..s4
        1,   2,   1,   1,   1,   1,   1,   1,   1,   2,   1,   1,   1,   1,   1,   1,
        1,   1,   1,   1,   1,   1,   1,   3,   1,   3,   1,   1,   1,   1,   1,   1, // s5..s6
        1,   3,   1,   1,   1,   1,   1,   3,   1,   3,   1,   1,   1,   1,   1,   1,
        1,   3,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1,   1, // s7..s8
    };

    auto b = static_cast<uint8_t>(ch);
    uint32_t type = kUtf8Dfa[b];
    m_iTmp = (m_iState != 0) ? (b & 0x3Fu) | (m_iTmp << 6u) : (0xFFu >> type) & b;
    m_iState = kUtf8Dfa[256 + m_iState * 16 + type];

    count = 0;
    switch (m_iState)
    {
        case 0:
            out[0] = m_iTmp;
            m_iTmp = 0;
            count = 1;
            return EncodingResult::Accept;
        case 1:
            m_iState = 0;
            m_iTmp = 0;
            return EncodingResult::Reject;
        default:
            return EncodingResult::Incomplete;
    }
}

EncodingResult Utf8::Encoder::operator()(InputType ch, std::array<OutputType, kMaxOutputCount>& out, uint32_t& count) noexcept
{
    auto cp = static_cast<uint32_t>(ch);

    count = 0;
    if (cp <= 0x7Fu)
    {
        out[0] = static_cast<char>(cp & 0xFFu);
        count = 1;
    }
    else if (cp <= 0x7FFu)
    {
        out[0] = static_cast<char>(0xC0u | ((cp >> 6u) & 0xFFu));
        out[1] = static_cast<char>(0x80u | (cp & 0x3Fu));
        count = 2;
    }
    else if (cp <= 0xFFFFu)
    {
        out[0] = static_cast<char>(0xE0u | ((cp >> 12u) & 0xFFu));
        out[1] = static_cast<char>(0x80u | ((cp >> 6u) & 0x3Fu));
        out[2] = static_cast<char>(0x80u | (cp & 0x3Fu));
        count = 3;
    }
    else if (cp <= 0x10FFFFu)
    {
        out[0] = static_cast<char>(0xF0u | ((cp >> 18u) & 0xFFu));
        out[1] = static_cast<char>(0x80u | ((cp >> 12u) & 0x3Fu));
        out[2] = static_cast<char>(0x80u | ((cp >> 6u) & 0x3Fu));
        out[3] = static_cast<char>(0x80u | (cp & 0x3Fu));
        count = 4;
    }
    else
    {
        return EncodingResult::Reject;
    }
    return EncodingResult::Accept;
}

// </editor-fold>

// <editor-fold desc="UTF-16">

const char* const Utf16::kName = "Utf16";

EncodingResult Utf16::Decoder::operator()(InputType ch, std::array<OutputType, kMaxOutputCount>& out, uint32_t& count) noexcept
{
    auto word = static_cast<uint16_t>(ch);

    count = 0;
    switch (m_iState)
    {
        case 0:
            if (word < 0xD800u || word > 0xDFFFu)
            {
                out[0] = word;
                count = 1;
                return EncodingResult::Accept;
            }
            else if (word <= 0xDBFFu)
            {
                m_iLastWord = word;
                m_iState = 1;
                return EncodingResult::Incomplete;
            }
            else [[unlikely]]
            {
                return EncodingResult::Reject;
            }
        case 1:
            if (!(word >= 0xDC00u && word <= 0xDFFFu)) [[unlikely]]
            {
                m_iState = 0;
                return EncodingResult::Reject;
            }
            out[0] = (m_iLastWord & 0x3FFu) << 10u;
            out[0] |= word & 0x3FFu;
            out[0] += 0x10000u;
            count = 1;
            return EncodingResult::Accept;
        default:
            assert(false);
            return EncodingResult::Reject;
    }
}

EncodingResult Utf16::Encoder::operator()(InputType ch, std::array<OutputType, kMaxOutputCount>& out, uint32_t& count) noexcept
{
    auto cp = static_cast<uint32_t>(ch);

    count = 0;
    if (cp <= 0xFFFFu)
    {
        out[0] = static_cast<char16_t>(cp);
        count = 1;
    }
    else if (cp <= 0x10FFFFu)
    {
        cp -= 0x10000u;
        out[0] = static_cast<char16_t>(0xD800u | (cp >> 10u));
        out[1] = static_cast<char16_t>(0xDC00u | (cp & 0x3FFu));
        count = 2;
    }
    else [[unlikely]]
    {
        return EncodingResult::Reject;
    }
    return EncodingResult::Accept;
}

// </editor-fold>

// <editor-fold desc="UTF-32">

const char* const Utf32::kName = "Utf32_Dummy";

// </editor-fold>
