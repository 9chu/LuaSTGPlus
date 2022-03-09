/**
 * @file
 * @date 2022/3/5
 * @author 9chu
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#include "HttpHelper.hpp"

#include <charconv>
#include <lstg/Core/Logging.hpp>

#ifdef LSTG_PLATFORM_EMSCRIPTEN
#include <emscripten/fetch.h>
#endif

using namespace std;
using namespace lstg;
using namespace lstg::Subsystem::VFS::detail;

LSTG_DEF_LOG_CATEGORY(HttpHelper);

int lstg::Subsystem::VFS::detail::CaseInsensitiveCompare(std::string_view s1, std::string_view s2) noexcept
{
    static unsigned char kCaseInsensitiveMap[] = {
        0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
        0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
        0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
        0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f,
        0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27,
        0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f,
        0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,
        0x38, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x3e, 0x3f,
        0x40, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67,
        0x68, 0x69, 0x6a, 0x6b, 0x6c, 0x6d, 0x6e, 0x6f,
        0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77,
        0x78, 0x79, 0x7a, 0x5b, 0x5c, 0x5d, 0x5e, 0x5f,
        0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67,
        0x68, 0x69, 0x6a, 0x6b, 0x6c, 0x6d, 0x6e, 0x6f,
        0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77,
        0x78, 0x79, 0x7a, 0x7b, 0x7c, 0x7d, 0x7e, 0x7f,
        0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87,
        0x88, 0x89, 0x8a, 0x8b, 0x8c, 0x8d, 0x8e, 0x8f,
        0x90, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97,
        0x98, 0x99, 0x9a, 0x9b, 0x9c, 0x9d, 0x9e, 0x9f,
        0xa0, 0xa1, 0xa2, 0xa3, 0xa4, 0xa5, 0xa6, 0xa7,
        0xa8, 0xa9, 0xaa, 0xab, 0xac, 0xad, 0xae, 0xaf,
        0xb0, 0xb1, 0xb2, 0xb3, 0xb4, 0xb5, 0xb6, 0xb7,
        0xb8, 0xb9, 0xba, 0xbb, 0xbc, 0xbd, 0xbe, 0xbf,
        0xc0, 0xe1, 0xe2, 0xe3, 0xe4, 0xc5, 0xe6, 0xe7,
        0xe8, 0xe9, 0xea, 0xeb, 0xec, 0xed, 0xee, 0xef,
        0xf0, 0xf1, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7,
        0xf8, 0xf9, 0xfa, 0xdb, 0xdc, 0xdd, 0xde, 0xdf,
        0xe0, 0xe1, 0xe2, 0xe3, 0xe4, 0xe5, 0xe6, 0xe7,
        0xe8, 0xe9, 0xea, 0xeb, 0xec, 0xed, 0xee, 0xef,
        0xf0, 0xf1, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7,
        0xf8, 0xf9, 0xfa, 0xfb, 0xfc, 0xfd, 0xfe, 0xff,
    };

    size_t i = 0;
    while (true)
    {
        uint8_t u1 = (i >= s1.length()) ? 0 : static_cast<uint8_t>(s1[i]);
        uint8_t u2 = (i >= s2.length()) ? 0 : static_cast<uint8_t>(s2[i]);
        if (kCaseInsensitiveMap[u1] != kCaseInsensitiveMap[u2])
            return kCaseInsensitiveMap[u1] - kCaseInsensitiveMap[u2];
        if (u1 == 0)
            return 0;
        ++i;
    }
}

Result<time_t> lstg::Subsystem::VFS::detail::ParseHttpDateTime(std::string_view buf) noexcept
{
    static const char* kMonths[12] = {
        "Jan", "Feb", "Mar", "Apr", "May", "Jun",
        "Jul", "Aug", "Sep", "Oct", "Nov", "Dec",
    };

    enum {
        STATE_LOCATE_COMMA,
        STATE_LOOK_FOR_DAY,
        STATE_PARSE_DAY,
        STATE_LOOK_FOR_MONTH,
        STATE_PARSE_MONTH,
        STATE_LOOK_FOR_YEAR,
        STATE_PARSE_YEAR,
        STATE_LOOK_FOR_HOURS,
        STATE_PARSE_HOURS,
        STATE_LOOK_FOR_MINUTES,
        STATE_PARSE_MINUTES,
        STATE_LOOK_FOR_SECONDS,
        STATE_PARSE_SECONDS,
        STATE_LOOK_END,
    } state = STATE_LOCATE_COMMA;

    tm tm;
    time_t ret = 0;
    char buffer[16];
    size_t bufferLength = 0;

    ::memset(&tm, 0, sizeof(tm));

#define APPEND_TO_BUFFER(CH) \
    {                                                                        \
        if (bufferLength >= sizeof(buffer))                                  \
            return make_error_code(WebFileSystemError::InvalidHttpDateTime); \
        buffer[bufferLength ++] = CH;                                        \
    } while (false)


#define GEN_PARSE_NUMBER_STATE(LOOKFOR_STATE, LOOKFOR_EXPECT_COND, PARSE_STATE, EXIT_EXPECT_COND, PARSE_OUTPUT, NEXT_STATE) \
            case LOOKFOR_STATE:                                                                                           \
                if (LOOKFOR_EXPECT_COND)                                                                                  \
                {                                                                                                         \
                    continue;                                                                                             \
                }                                                                                                         \
                else if ('0' <= ch && ch <= '9')                                                                          \
                {                                                                                                         \
                    state = PARSE_STATE;                                                                                  \
                    APPEND_TO_BUFFER(ch);                                                                                 \
                }                                                                                                         \
                else                                                                                                      \
                {                                                                                                         \
                    LSTG_LOG_DEBUG_CAT(HttpHelper, "Unexpected character '{}'", ch);                                      \
                    return make_error_code(WebFileSystemError::InvalidHttpDateTime);                                      \
                }                                                                                                         \
                break;                                                                                                    \
            case PARSE_STATE:                                                                                             \
                if ('0' <= ch && ch <= '9')                                                                               \
                {                                                                                                         \
                    APPEND_TO_BUFFER(ch);                                                                                 \
                }                                                                                                         \
                else if (EXIT_EXPECT_COND)                                                                                \
                {                                                                                                         \
                    auto [ptr, ec] = from_chars(buffer, buffer + bufferLength, PARSE_OUTPUT);                             \
                    if (ec != std::errc())                                                                                \
                    {                                                                                                     \
                        LSTG_LOG_DEBUG_CAT(HttpHelper, "Parse number \"{}\" fail", string_view{buffer, bufferLength});    \
                        return make_error_code(WebFileSystemError::InvalidHttpDateTime);                                  \
                    }                                                                                                     \
                    bufferLength = 0;                                                                                     \
                    state = NEXT_STATE;                                                                                   \
                }                                                                                                         \
                else                                                                                                      \
                {                                                                                                         \
                    LSTG_LOG_DEBUG_CAT(HttpHelper, "Unexpected character '{}'", ch);                                      \
                    return make_error_code(WebFileSystemError::InvalidHttpDateTime);                                      \
                }                                                                                                         \
                break

    // e.g. Sun, 04 Jan 1970 00:00:00 GMT
    for (size_t i = 0; i < buf.length() + 1; ++i)
    {
        auto ch = (i >= buf.length()) ? '\0' : buf[i];

        switch (state)
        {
            case STATE_LOCATE_COMMA:
                if (ch == '\0')
                    return make_error_code(WebFileSystemError::InvalidHttpDateTime);
                else if (ch == ',')
                    state = STATE_LOOK_FOR_DAY;
                break;
            GEN_PARSE_NUMBER_STATE(STATE_LOOK_FOR_DAY, (ch == ' '), STATE_PARSE_DAY, (ch == ' '), tm.tm_mday, STATE_LOOK_FOR_MONTH);
            case STATE_LOOK_FOR_MONTH:
                if (ch == ' ')
                {
                    continue;
                }
                else if (('a' <= ch && ch <= 'z') || ('A' <= ch && ch <= 'Z'))
                {
                    state = STATE_PARSE_MONTH;
                    APPEND_TO_BUFFER(ch);
                }
                else
                {
                    LSTG_LOG_DEBUG_CAT(HttpHelper, "Unexpected character '{}'", ch);
                    return make_error_code(WebFileSystemError::InvalidHttpDateTime);
                }
                break;
            case STATE_PARSE_MONTH:
                if (('a' <= ch && ch <= 'z') || ('A' <= ch && ch <= 'Z'))
                {
                    APPEND_TO_BUFFER(ch);
                }
                else if (ch == ' ')
                {
                    tm.tm_mon = -1;
                    for (size_t m = 0; m < std::extent_v<decltype(kMonths)>; m++)
                    {
                        if (CaseInsensitiveCompare({buffer, bufferLength}, kMonths[m]) == 0)
                        {
                            tm.tm_mon = static_cast<int>(m);
                            break;
                        }
                    }
                    if (tm.tm_mon < 0)
                    {
                        LSTG_LOG_DEBUG_CAT(HttpHelper, "Unexpected month \"{}\"", string_view{buffer, bufferLength});
                        return make_error_code(WebFileSystemError::InvalidHttpDateTime);
                    }
                    bufferLength = 0;
                    state = STATE_LOOK_FOR_YEAR;
                }
                else
                {
                    LSTG_LOG_DEBUG_CAT(HttpHelper, "Unexpected character '{}'", ch);
                    return make_error_code(WebFileSystemError::InvalidHttpDateTime);
                }
                break;
            GEN_PARSE_NUMBER_STATE(STATE_LOOK_FOR_YEAR, (ch == ' '), STATE_PARSE_YEAR, (ch == ' '), tm.tm_year, STATE_LOOK_FOR_HOURS);
            GEN_PARSE_NUMBER_STATE(STATE_LOOK_FOR_HOURS, (ch == ' '), STATE_PARSE_HOURS, (ch == ' ' || ch == ':'), tm.tm_hour,
                STATE_LOOK_FOR_MINUTES);
            GEN_PARSE_NUMBER_STATE(STATE_LOOK_FOR_MINUTES, (ch == ' ' || ch == ':'), STATE_PARSE_MINUTES, (ch == ' ' || ch == ':'),
                tm.tm_min, STATE_LOOK_FOR_SECONDS);
            GEN_PARSE_NUMBER_STATE(STATE_LOOK_FOR_SECONDS, (ch == ' '), STATE_PARSE_SECONDS, (ch == ' '), tm.tm_sec, STATE_LOOK_END);
            case STATE_LOOK_END:
                i = buf.length();
                break;
            default:
                assert(false);
                break;
        }
    }
#undef APPEND_TO_BUFFER

    tm.tm_year -= 1900;
    ret = timegm(&tm);
    return ret;
}

#ifdef LSTG_PLATFORM_EMSCRIPTEN

void FetchCloser::operator()(emscripten_fetch_t* fetch) noexcept
{
    if (fetch)
        ::emscripten_fetch_close(fetch);
}

#endif
