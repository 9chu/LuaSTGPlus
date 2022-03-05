/**
 * @file
 * @date 2022/3/5
 * @author 9chu
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#pragma once
#include <ctime>
#include <cassert>
#include <string_view>
#include <lstg/Core/Result.hpp>
#include "WebFileSystemError.hpp"

struct emscripten_fetch_t;

namespace lstg::Subsystem::VFS::detail
{
    /**
     * 大小写不敏感比较
     * @param s1 输入串 A
     * @param s2 输入串 B
     * @return 比较结果，如果相等返回 0
     */
    int CaseInsensitiveCompare(std::string_view s1, std::string_view s2) noexcept;

    /**
     * 解析 HTTP 日期时间
     * @param buf 输入 GMT 时间串
     * @return UNIX 时间戳
     */
    Result<time_t> ParseHttpDateTime(std::string_view buf) noexcept;

    /**
     * 遍历 HTTP 头部
     * @tparam TVisitor 遍历器
     * @param rawHeaders 原始头部数据
     * @param visitor 遍历器
     * @return 遍历器返回失败结果时透传
     */
    template <typename TVisitor>
    Result<void> VisitHeader(std::string_view rawHeaders, TVisitor&& visitor)
    {
        enum {
            STATE_KEY_START,
            STATE_READ_KEY,
            STATE_VALUE_START,
            STATE_READ_VALUE,
            STATE_HEADER_END_LF,
            STATE_ALL_HEADER_END_LF,
        } state = STATE_KEY_START;

        char keyBuffer[32];
        size_t keyBufferLength = 0;
        char valueBuffer[128];
        size_t valueBufferLength = 0;

#define APPEND_TO_BUFFER(WHICH, CH) \
        {                                                                           \
            if (WHICH##BufferLength >= sizeof(WHICH##Buffer))                       \
                return make_error_code(WebFileSystemError::HttpHeaderFieldTooLong); \
            WHICH##Buffer[WHICH##BufferLength ++] = CH;                             \
        } while (false)

        for (size_t i = 0; i < rawHeaders.size() + 1; ++i)
        {
            auto ch = (i >= rawHeaders.size()) ? '\0' : rawHeaders[i];

            switch (state)
            {
                case STATE_KEY_START:
                    if (ch == ' ' || ch == '\t')
                    {
                        continue;
                    }
                    else if (ch == '\r')
                    {
                        state = STATE_ALL_HEADER_END_LF;
                    }
                    else if (ch != '\0')
                    {
                        state = STATE_READ_KEY;
                        APPEND_TO_BUFFER(key, ch);
                    }
                    break;
                case STATE_READ_KEY:
                    if (ch == ':')
                    {
                        if (keyBufferLength == 0)
                            return make_error_code(WebFileSystemError::InvalidHttpHeader);
                        state = STATE_VALUE_START;
                    }
                    else if (ch == '\r' || ch == '\n' || ch == '\0')
                    {
                        return make_error_code(WebFileSystemError::InvalidHttpHeader);
                    }
                    else
                    {
                        APPEND_TO_BUFFER(key, ch);
                    }
                    break;
                case STATE_VALUE_START:
                    if (ch == ' ' || ch == '\t')
                    {
                        continue;
                    }
                    else if (ch == '\r' || ch == '\n' || ch == '\0')
                    {
                        return make_error_code(WebFileSystemError::InvalidHttpHeader);
                    }
                    else
                    {
                        state = STATE_READ_VALUE;
                        APPEND_TO_BUFFER(value, ch);
                    }
                    break;
                case STATE_READ_VALUE:
                    if (ch == '\r')
                        state = STATE_HEADER_END_LF;
                    else if (ch == '\n' || ch == '\0')
                        return make_error_code(WebFileSystemError::InvalidHttpHeader);
                    else
                        APPEND_TO_BUFFER(value, ch);
                    break;
                case STATE_HEADER_END_LF:
                    if (ch == '\n')
                    {
                        auto ret = visitor(std::string_view{keyBuffer, keyBufferLength}, std::string_view{valueBuffer, valueBufferLength});
                        if (!ret)
                            return ret.GetError();

                        state = STATE_KEY_START;
                        keyBufferLength = 0;
                        valueBufferLength = 0;
                    }
                    else
                    {
                        return make_error_code(WebFileSystemError::InvalidHttpHeader);
                    }
                    break;
                case STATE_ALL_HEADER_END_LF:
                    if (ch == '\n')
                        i = rawHeaders.length();
                    else
                        return make_error_code(WebFileSystemError::InvalidHttpHeader);
                    break;
                default:
                    assert(false);
                    break;
            }
        }
#undef APPEND_TO_BUFFER

        return {};
    }

    struct FetchCloser
    {
        void operator()(emscripten_fetch_t* fetch) noexcept;
    };

    using FetchPtr = std::unique_ptr<emscripten_fetch_t, FetchCloser>;
}
