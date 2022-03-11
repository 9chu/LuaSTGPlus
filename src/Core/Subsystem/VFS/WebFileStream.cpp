/**
 * @file
 * @date 2022/3/5
 * @author 9chu
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#include <lstg/Core/Subsystem/VFS/WebFileStream.hpp>

// 仅在 EMSCRIPTEN 下编译
#ifdef LSTG_PLATFORM_EMSCRIPTEN

#include <emscripten/fetch.h>
#include <lstg/Core/Logging.hpp>
#include "detail/HttpHelper.hpp"

using namespace std;
using namespace lstg;
using namespace lstg::Subsystem::VFS;

using WebFileSystemError = lstg::Subsystem::VFS::detail::WebFileSystemError;

LSTG_DEF_LOG_CATEGORY(WebFileStream);

WebFileStream::WebFileStream(std::string url, FetchConfig config, uint64_t contentLength)
    : m_stUrl(std::move(url)), m_stConfig(std::move(config)), m_ullContentLength(contentLength)
{
}

bool WebFileStream::IsReadable() const noexcept
{
    return true;
}

bool WebFileStream::IsWriteable() const noexcept
{
    return false;
}

bool WebFileStream::IsSeekable() const noexcept
{
    return true;
}

Result<uint64_t> WebFileStream::GetLength() const noexcept
{
    return m_ullContentLength;
}

Result<void> WebFileStream::SetLength(uint64_t length) noexcept
{
    return make_error_code(errc::not_supported);
}

Result<uint64_t> WebFileStream::GetPosition() const noexcept
{
    return m_ullFakeReadPosition;
}

Result<void> WebFileStream::Seek(int64_t offset, StreamSeekOrigins origin) noexcept
{
    switch (origin)
    {
        case StreamSeekOrigins::Begin:
            m_ullFakeReadPosition = static_cast<uint64_t>(std::max<int64_t>(0, offset));
            m_ullFakeReadPosition = std::min<uint64_t>(m_ullFakeReadPosition, m_ullContentLength);
            break;
        case StreamSeekOrigins::Current:
            if (offset < 0)
            {
                auto positive = static_cast<uint64_t>(-offset);
                if (positive >= m_ullFakeReadPosition)
                    m_ullFakeReadPosition = 0;
                else
                    m_ullFakeReadPosition -= positive;
            }
            else
            {
                m_ullFakeReadPosition += static_cast<uint64_t>(offset);
                m_ullFakeReadPosition = std::min<uint64_t>(m_ullFakeReadPosition, m_ullContentLength);
            }
            break;
        case StreamSeekOrigins::End:
            if (offset >= 0)
            {
                m_ullFakeReadPosition = m_ullContentLength;
            }
            else
            {
                auto positive = static_cast<uint64_t>(-offset);
                if (positive >= m_ullContentLength)
                    m_ullFakeReadPosition = 0;
                else
                    m_ullFakeReadPosition -= positive;
            }
            break;
    }
    return {};
}

Result<bool> WebFileStream::IsEof() const noexcept
{
    return m_ullFakeReadPosition >= m_ullContentLength;
}

Result<void> WebFileStream::Flush() noexcept
{
    return make_error_code(errc::not_supported);
}

Result<size_t> WebFileStream::Read(uint8_t* buffer, size_t length) noexcept
{
    if (length == 0)
        return static_cast<size_t>(0);

    assert(m_ullFakeReadPosition <= m_ullContentLength);
    auto rest = static_cast<size_t>(m_ullContentLength - m_ullFakeReadPosition);
    length = std::min<size_t>(length, rest);

    bool fetchInProgress = true;
    ::emscripten_fetch_attr_t attr;
    ::emscripten_fetch_attr_init(&attr);
    strcpy(attr.requestMethod, "GET");
    attr.attributes = EMSCRIPTEN_FETCH_LOAD_TO_MEMORY;
    attr.timeoutMSecs = m_stConfig.ReadRequestTimeout;
    attr.userData = &fetchInProgress;
    attr.onsuccess = [](emscripten_fetch_t* fetch) {
        LSTG_LOG_TRACE_CAT(WebFileStream, "onsuccess");
        *reinterpret_cast<bool*>(fetch->userData) = false;
    };
    attr.onerror = [](emscripten_fetch_t* fetch) {
        LSTG_LOG_TRACE_CAT(WebFileStream, "onerror");
        *reinterpret_cast<bool*>(fetch->userData) = false;
    };
    if (!m_stConfig.UserName.empty() || !m_stConfig.Password.empty())
    {
        attr.userName = m_stConfig.UserName.c_str();
        attr.password = m_stConfig.Password.c_str();
    }
    string range = fmt::format("bytes={}-{}", m_ullFakeReadPosition, m_ullFakeReadPosition + length - 1);
    const char* headers[] = {"Range", range.c_str(), nullptr};
    attr.requestHeaders = headers;

    // 发起请求
#if !(defined(NDEBUG) || defined(LSTG_SHIPPING))
    auto requestStart = std::chrono::steady_clock::now();
#endif
    detail::FetchPtr fetch {::emscripten_fetch(&attr, m_stUrl.c_str())};
    if (!fetch)
    {
        LSTG_LOG_ERROR_CAT(WebFileStream, "emscripten_fetch returns NULL, url: {}", m_stUrl);
        return make_error_code(WebFileSystemError::ApiError);
    }
    while (fetchInProgress)
    {
        // 依赖 ASYNCIFY 进行忙等待
        ::emscripten_sleep(0);
    }
#if !(defined(NDEBUG) || defined(LSTG_SHIPPING))
    auto requestEnd = std::chrono::steady_clock::now();
    auto requestTime = std::chrono::duration_cast<std::chrono::microseconds>(requestEnd - requestStart).count();
#else
    auto requestTime = -1;
#endif
    LSTG_LOG_TRACE_CAT(WebFileStream, "Server response {}, url: {}, cost: {}ms", fetch->status, m_stUrl, requestTime);

    if (fetch->status != 200 && fetch->status != 206)
    {
        LSTG_LOG_ERROR_CAT(WebFileStream, "Server response {}, url: {}", fetch->status, m_stUrl);
        if (fetch->status == 404)
            return make_error_code(errc::no_such_file_or_directory);
        else if (fetch->status == 503)
            return make_error_code(errc::permission_denied);
        else if (fetch->status == 400)
            return make_error_code(errc::protocol_error);
        else
            return make_error_code(WebFileSystemError::HttpError);
    }
    else
    {
        // 获取数据
        if (fetch->numBytes != length)
        {
            LSTG_LOG_ERROR_CAT(WebFileStream, "Server response {} bytes, but {} bytes expected, url: {}", fetch->numBytes, length, m_stUrl);
            return make_error_code(errc::io_error);
        }

        // 拷贝
        ::memcpy(buffer, fetch->data, length);

        m_ullFakeReadPosition += length;
        return length;
    }
}

Result<void> WebFileStream::Write(const uint8_t* buffer, size_t length) noexcept
{
    return make_error_code(errc::not_supported);
}

Result<StreamPtr> WebFileStream::Clone() const noexcept
{
    try
    {
        return make_shared<WebFileStream>(m_stUrl, m_stConfig, m_ullContentLength);
    }
    catch (...)  // bad_alloc
    {
        return make_error_code(errc::not_enough_memory);
    }
}

#endif
