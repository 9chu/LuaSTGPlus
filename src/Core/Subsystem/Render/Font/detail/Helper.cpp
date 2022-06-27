/**
 * @file
 * @date 2022/6/27
 * @author 9chu
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#include "Helper.hpp"

#include <unicode/uchar.h>

using namespace std;
using namespace lstg;
using namespace lstg::Subsystem::Render::Font::detail;

constexpr const uint16_t kHighSurrogateStart = 0xD800u;
constexpr const uint16_t kHighSurrogateEnd = 0xDBFFu;
constexpr const uint16_t kLowSurrogateStart = 0xDC00u;
constexpr const uint16_t kLowSurrogateEnd = 0xDFFFu;

bool Helper::IsHighSurrogate(uint32_t codePoint) noexcept
{
    return codePoint >= kHighSurrogateStart && codePoint <= kHighSurrogateEnd;
}

bool Helper::IsLowSurrogate(uint32_t codePoint) noexcept
{
    return codePoint >= kLowSurrogateStart && codePoint <= kLowSurrogateEnd;
}

uint32_t Helper::EncodeSurrogate(uint16_t highSurrogate, uint16_t lowSurrogate) noexcept
{
    return ((highSurrogate - kHighSurrogateStart) << 10) + (lowSurrogate - kLowSurrogateStart) + 0x10000;
}

bool Helper::IsRenderAsWhitespace(char32_t codePoint) noexcept
{
    return u_isWhitespace(codePoint) ||
        codePoint == U'\u200B' ||  // Zero Width Space
        codePoint == U'\u2009' ||  // Thin Space
        codePoint == U'\u202F';  // Narrow No-break Space
}

bool Helper::IsControlCharacter(char32_t codePoint) noexcept
{
    return u_isISOControl(codePoint);
}
