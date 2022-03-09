/**
 * @file
 * @date 2022/2/20
 * @author 9chu
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#include <lstg/Core/Subsystem/VFS/WebFileSystem.hpp>

// 仅在 EMSCRIPTEN 下编译
#ifdef LSTG_PLATFORM_EMSCRIPTEN

#include <cstdlib>
#include <charconv>
#include <optional>
#include <string_view>
#include <lstg/Core/Logging.hpp>
#include "detail/WebFileSystemError.hpp"
#include "detail/HttpHelper.hpp"

#include <emscripten/fetch.h>

using namespace std;
using namespace lstg;
using namespace lstg::Subsystem::VFS;

using lstg::Subsystem::VFS::detail::WebFileSystemError;

LSTG_DEF_LOG_CATEGORY(WebFileSystem);

namespace
{
    Result<string> MakeFullUrl(const std::string& base, const Path& path) noexcept
    {
        try
        {
            if (base.empty())
                return path.ToString();
            return fmt::format("{}{}{}", base, (base[base.size() - 1] == '/') ? "" : "/", path.ToStringView());
        }
        catch (...)  // bad_alloc
        {
            return make_error_code(errc::not_enough_memory);
        }
    }

    Result<string> GetResponseHeaders(emscripten_fetch_t* fetch) noexcept
    {
        string ret;
        auto len = ::emscripten_fetch_get_response_headers_length(fetch);
        try
        {
            ret.resize(len + 1);
            auto cnt = emscripten_fetch_get_response_headers(fetch, ret.data(), ret.length() + 1);
            assert(cnt == len);
            assert(ret.empty() || ret[ret.size() - 1] == '\0');
            if (!ret.empty())
                ret.pop_back();
            return ret;
        }
        catch (...)  // bad_alloc
        {
            return make_error_code(errc::not_enough_memory);
        }
    }
}

WebFileSystem::WebFileSystem(std::string_view baseUrl)
    : m_stBaseUrl(baseUrl)
{
}

Result<void> WebFileSystem::CreateDirectory(Path path) noexcept
{
    return make_error_code(errc::not_supported);
}

Result<void> WebFileSystem::Remove(Path path) noexcept
{
    return make_error_code(errc::not_supported);
}

Result<FileAttribute> WebFileSystem::GetFileAttribute(Path path) noexcept
{
    auto fullUrl = MakeFullUrl(m_stBaseUrl, path);
    if (!fullUrl)
        return fullUrl.GetError();

    LSTG_LOG_TRACE_CAT(WebFileSystem, "GetFileAttribute of \"{}\"", *fullUrl);

    ::emscripten_fetch_attr_t attr;
    ::emscripten_fetch_attr_init(&attr);
    strcpy(attr.requestMethod, "HEAD");
    attr.attributes = EMSCRIPTEN_FETCH_LOAD_TO_MEMORY | EMSCRIPTEN_FETCH_SYNCHRONOUS;
    attr.timeoutMSecs = m_uHeadRequestTimeout;
    if (!m_stConfig.UserName.empty() || !m_stConfig.Password.empty())
    {
        attr.userName = m_stConfig.UserName.c_str();
        attr.password = m_stConfig.Password.c_str();
    }

    // 发起请求
#if !(defined(NDEBUG) || defined(LSTG_SHIPPING))
    auto requestStart = std::chrono::steady_clock::now();
#endif
    detail::FetchPtr fetch {::emscripten_fetch(&attr, fullUrl->c_str())};
    if (!fetch)
    {
        LSTG_LOG_ERROR_CAT(WebFileSystem, "emscripten_fetch returns NULL, url: {}", *fullUrl);
        return make_error_code(WebFileSystemError::ApiError);
    }
#if !(defined(NDEBUG) || defined(LSTG_SHIPPING))
    auto requestEnd = std::chrono::steady_clock::now();
    auto requestTime = std::chrono::duration_cast<std::chrono::microseconds>(requestEnd - requestStart).count();
#else
    auto requestTime = -1;
#endif
    LSTG_LOG_TRACE_CAT(WebFileSystem, "Server response {}, url: {}, cost: {}ms", fetch->status, *fullUrl, requestTime);

    if (fetch->status != 200)
    {
        LSTG_LOG_ERROR_CAT(WebFileSystem, "Server response {}, url: {}", fetch->status, *fullUrl);
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
        // 成功，获取请求数据
        auto responseHeaders = GetResponseHeaders(fetch.get());
        if (!responseHeaders)
            return responseHeaders.GetError();

        // 遍历头部获取信息
        optional<uint64_t> contentLength;
        optional<time_t> lastModified;
        bool acceptRange = false;
        detail::VisitHeader(*responseHeaders, [&](string_view key, string_view value) -> Result<void> {
            if (detail::CaseInsensitiveCompare(key, "Content-Length") == 0)
            {
                uint64_t v = 0;
                auto [ptr, ec] = std::from_chars(value.begin(), value.end(), v);
                if (ec == std::errc())
                    contentLength = v;
            }
            else if (detail::CaseInsensitiveCompare(key, "Accept-Ranges") == 0)
            {
                if (value.find("bytes") != string_view::npos)
                    acceptRange = true;
            }
            else if (detail::CaseInsensitiveCompare(key, "Last-Modified") == 0)
            {
                auto ret = detail::ParseHttpDateTime(value);
                if (!ret)
                    LSTG_LOG_WARN_CAT(WebFileSystem, "Parse http date \"{}\" fail", value);
                else
                    lastModified = *ret;
            }
            return {};
        });
        if (!acceptRange)
        {
            LSTG_LOG_ERROR_CAT(WebFileSystem, "Server not accept range request, url: {}", *fullUrl);
            return make_error_code(errc::no_protocol_option);
        }
        if (!contentLength)
        {
            LSTG_LOG_ERROR_CAT(WebFileSystem, "Server not send Content-Length, url: {}", *fullUrl);
            return make_error_code(errc::protocol_not_supported);
        }

        // 返回
        FileAttribute fileAttr;
        fileAttr.Type = FileType::RegularFile;
        fileAttr.LastModified = lastModified ? *lastModified : 0;
        fileAttr.Size = *contentLength;
        return fileAttr;
    }
}

Result<DirectoryIteratorPtr> WebFileSystem::VisitDirectory(Path path) noexcept
{
    return make_error_code(errc::not_supported);
}

Result<StreamPtr> WebFileSystem::OpenFile(Path path, FileAccessMode access, FileOpenFlags flags) noexcept
{
    // 先获取文件属性
    auto attr = GetFileAttribute(path);
    if (!attr)
        return attr.GetError();

    // 如果能获取属性，说明：
    //   1. 可以以 Range 模式访问文件
    //   2. 文件存在
    //   3. 知道文件大小

    try
    {
        auto fullUrl = MakeFullUrl(m_stBaseUrl, path);
        auto stream = make_shared<WebFileStream>(std::move(*fullUrl), m_stConfig, attr->Size);
        return stream;
    }
    catch (...)  // bad_alloc
    {
        return make_error_code(errc::not_enough_memory);
    }
}

#endif
