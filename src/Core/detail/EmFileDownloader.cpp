/**
 * @file
 * @date 2022/8/16
 * @author 9chu
 * 此文件为 LuaSTGPlus 项目的一部分，版权与许可声明详见 COPYRIGHT.txt。
 */
#include "EmFileDownloader.hpp"

// 仅在 EMSCRIPTEN 下编译
#ifdef LSTG_PLATFORM_EMSCRIPTEN

#include <functional>
#include <charconv>
#include <emscripten/fetch.h>
#include <lstg/Core/Logging.hpp>
#include <lstg/Core/Span.hpp>
#include "HttpHelper.hpp"

using namespace std;
using namespace lstg;
using namespace lstg::detail;

namespace
{
    using HttpRequestCallback = std::function<void(std::error_code, emscripten_fetch_t*) /* noexcept */>;

    /**
     * 发起异步 HTTP 请求
     * @param method 方法
     * @param url URL
     * @param headers 头部
     * @param content 请求正文
     * @param timeoutMs 超时（毫秒）
     * @param callback 回调
     * @return 是否成功发起
     */
    template <typename TCallback>
    Result<void> AsyncHttpRequest(const char* method, const char* url, Span<const char*> headers, std::string_view content,
        uint32_t timeoutMs, TCallback&& callback) noexcept
    {
        struct Wrapper
        {
            ::emscripten_fetch_attr_t Attr;
            HttpRequestCallback Callback;
            std::vector<std::string> RequestHeaders;
            std::vector<const char*> RequestHeaderPointers;
            std::string RequestContent;
        };

        // 构造 wrapper
        Wrapper* wrapper = nullptr;
        try
        {
            wrapper = new Wrapper();
            ::emscripten_fetch_attr_init(&wrapper->Attr);
            wrapper->Callback = std::forward<TCallback>(callback);
            assert(wrapper->Callback);
            for (size_t i = 0; i < headers.size(); ++i)
                wrapper->RequestHeaders.emplace_back(headers[i]);
            for (size_t i = 0; i < wrapper->RequestHeaders.size(); ++i)
                wrapper->RequestHeaderPointers.push_back(wrapper->RequestHeaders[i].data());
            wrapper->RequestHeaderPointers.push_back(nullptr);
            wrapper->RequestContent = content;
        }
        catch (...)  // bad_alloc
        {
            return make_error_code(errc::not_enough_memory);
        }

        // 初始化 Request 参数
        auto& attr = wrapper->Attr;
        ::strcpy(attr.requestMethod, method);
        attr.attributes = EMSCRIPTEN_FETCH_LOAD_TO_MEMORY;
        attr.timeoutMSecs = timeoutMs;
        attr.userData = wrapper;
        attr.requestHeaders = wrapper->RequestHeaderPointers.data();
        attr.requestData = wrapper->RequestContent.data();
        attr.requestDataSize = wrapper->RequestContent.size();
        attr.onsuccess = [](emscripten_fetch_t* fetch) noexcept {
            assert(fetch->userData);
            auto wrapper = reinterpret_cast<Wrapper*>(fetch->userData);
            wrapper->Callback({}, fetch);
            delete wrapper;
            fetch->userData = nullptr;
            ::emscripten_fetch_close(fetch);
        };
        attr.onerror = [](emscripten_fetch_t* fetch) noexcept {
            assert(fetch->userData);
            auto wrapper = reinterpret_cast<Wrapper*>(fetch->userData);
            wrapper->Callback(make_error_code(errc::connection_aborted), fetch);  // FIXME: 错误码从哪里取？
            delete wrapper;
            fetch->userData = nullptr;
            ::emscripten_fetch_close(fetch);
        };

        auto fetch = ::emscripten_fetch(&wrapper->Attr, url);
        if (!fetch)
        {
            delete wrapper;
            return make_error_code(errc::io_error);
        }
        return {};
    }

    /**
     * 获取请求头
     * @param fetch fetch 对象
     * @return 请求头
     */
    Result<string> GetResponseHeaders(emscripten_fetch_t* fetch) noexcept
    {
        string ret;
        auto len = ::emscripten_fetch_get_response_headers_length(fetch);
        try
        {
            ret.resize(len + 1);
            auto cnt = emscripten_fetch_get_response_headers(fetch, ret.data(), ret.length() + 1);
            static_cast<void>(cnt);
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

LSTG_DEF_LOG_CATEGORY(EmFileDownloader);

EmFileDownloader::EmFileDownloader()
{
}

EmFileDownloader::~EmFileDownloader()
{
    // FIXME: 未处理析构时任务正在执行的情况
    assert(GetTaskCount() == 0);
}

size_t EmFileDownloader::GetTaskCount() const noexcept
{
    return m_stPendingTasks.size() + (m_stCurrentTask ? 1 : 0);
}

Result<void> EmFileDownloader::AddTask(std::string_view url, std::string_view saveAs, DownloadCallback callback,
    DownloadProgressCallback progressCallback) noexcept
{
    try
    {
        Task t;
        t.Url = url;
        t.SaveAs = saveAs;
        t.Callback = std::move(callback);
        t.ProgressCallback = std::move(progressCallback);
        m_stPendingTasks.emplace_back(std::move(t));
        return {};
    }
    catch (...)  // bad_alloc
    {
        return make_error_code(errc::not_enough_memory);
    }
}

void EmFileDownloader::CancelPendingTasks() noexcept
{
    m_stPendingTasks.clear();
}

void EmFileDownloader::Update() noexcept
{
    // 检查是否发起任务
    if (!m_stCurrentTask)
    {
        if (!m_stPendingTasks.empty())
        {
            // 发起任务
            m_stCurrentTask = std::move(m_stPendingTasks.front());
            m_stPendingTasks.erase(m_stPendingTasks.begin());
        }
        else
        {
            return;
        }
    }

    // 处理当前任务，转换状态机
    Result<void> ret;
    assert(m_stCurrentTask);
    switch (m_stCurrentTask->State)
    {
        case TASK_READY:
            ret = AsyncHttpRequest("HEAD", m_stCurrentTask->Url.c_str(), {}, {}, m_uRequestTimeout, [this](auto ec, auto fetch) noexcept {
                this->OnHandleGetFileInfo(ec, fetch);
            });
            if (!ret)
            {
                LSTG_LOG_ERROR_CAT(EmFileDownloader, "Start HTTP request error, url={}", m_stCurrentTask->Url);
                m_stCurrentTask->State = TASK_ERROR;
                m_stCurrentTask->ErrorCode = ret.GetError();
            }
            else
            {
                m_stCurrentTask->State = TASK_GET_FILE_INFO;
            }
            break;
        case TASK_GET_FILE_INFO:
        case TASK_READ_CHUNK:
            break;
        case TASK_ERROR:
            assert(m_stCurrentTask->Callback);
            assert(m_stCurrentTask->ErrorCode);
            {
                auto task = std::move(*m_stCurrentTask);
                m_stCurrentTask.reset();
                task.Callback(task.ErrorCode);
            }
            break;
        case TASK_FINISHED:
            assert(m_stCurrentTask->Callback);
            assert(!m_stCurrentTask->ErrorCode);
            {
                auto task = std::move(*m_stCurrentTask);
                m_stCurrentTask.reset();
                task.Callback({});
            }
            break;
        default:
            assert(false);
            break;
    }
}

void EmFileDownloader::FetchNextChunk() noexcept
{
    using namespace Subsystem::VFS;

    assert(m_stCurrentTask);
    if (m_stCurrentTask->State == TASK_GET_FILE_INFO)
    {
        try
        {
            m_stCurrentTask->FileStream = make_shared<FileStream>(m_stCurrentTask->SaveAs.c_str(), FileAccessMode::Write,
                FileOpenFlags::Truncate);
        }
        catch (const std::system_error& ex)
        {
            LSTG_LOG_ERROR_CAT(EmFileDownloader, "Open output stream fail, path={}, ec={}", m_stCurrentTask->SaveAs, ex.code());
            m_stCurrentTask->State = TASK_ERROR;
            m_stCurrentTask->ErrorCode = ex.code();
            return;
        }
        catch (...)  // bad_alloc
        {
            m_stCurrentTask->State = TASK_ERROR;
            m_stCurrentTask->ErrorCode = make_error_code(errc::not_enough_memory);
            return;
        }
        m_stCurrentTask->State = TASK_READ_CHUNK;
        if (m_stCurrentTask->ProgressCallback)
            m_stCurrentTask->ProgressCallback(m_stCurrentTask->Url, m_stCurrentTask->SaveAs, 0.f);
    }

    assert(m_stCurrentTask->State == TASK_READ_CHUNK);
    if (m_stCurrentTask->ReadSize >= m_stCurrentTask->ContentLength)
    {
        LSTG_LOG_TRACE_CAT(EmFileDownloader, "File downloaded, url={}", m_stCurrentTask->Url);
        m_stCurrentTask->State = TASK_FINISHED;
        m_stCurrentTask->FileStream.reset();  // Close file
        return;
    }

    // 发起下一个区块的读
    m_stCurrentTask->CurrentReadSize = std::min<size_t>(m_stCurrentTask->ContentLength - m_stCurrentTask->ReadSize, m_uMaxReadChunkSize);
    assert(m_stCurrentTask->CurrentReadSize != 0);
    auto readStart = m_stCurrentTask->ReadSize;
    auto readEnd = readStart + m_stCurrentTask->CurrentReadSize - 1;
    char readRange[64] = { 0 };
    assert(readStart <= static_cast<size_t>(std::numeric_limits<int>::max()));
    assert(readEnd <= static_cast<size_t>(std::numeric_limits<int>::max()));
    ::snprintf(readRange, sizeof(readRange) - 1, "bytes=%d-%d", static_cast<int>(readStart), static_cast<int>(readEnd));
    const char* headers[] = {"Range", readRange};
    auto ret = AsyncHttpRequest("GET", m_stCurrentTask->Url.c_str(), { headers, std::extent_v<decltype(headers)> }, {}, m_uRequestTimeout,
        [this](auto ec, auto fetch) noexcept {
            this->OnHandleReadChunk(ec, fetch);
        });
    if (!ret)
    {
        LSTG_LOG_ERROR_CAT(EmFileDownloader, "Start HTTP request error, url={}, range={}", m_stCurrentTask->Url, readRange);
        m_stCurrentTask->State = TASK_ERROR;
        m_stCurrentTask->ErrorCode = ret.GetError();
    }
}

void EmFileDownloader::OnHandleGetFileInfo(std::error_code ec, emscripten_fetch_t* fetch) noexcept
{
    if (fetch->status > 0 && fetch->status != 200)
    {
        LSTG_LOG_ERROR_CAT(EmFileDownloader, "Request HTTP error, url={}, status={}", fetch->url, fetch->status);
        m_stCurrentTask->State = TASK_ERROR;
        if (fetch->status == 404)
            m_stCurrentTask->ErrorCode = make_error_code(errc::no_such_file_or_directory);
        else if (fetch->status == 403)
            m_stCurrentTask->ErrorCode = make_error_code(errc::permission_denied);
        else if (fetch->status == 400)
            m_stCurrentTask->ErrorCode = make_error_code(errc::protocol_error);
        else
            m_stCurrentTask->ErrorCode = make_error_code(errc::device_or_resource_busy);
        return;
    }

    if (ec)
    {
        LSTG_LOG_ERROR_CAT(EmFileDownloader, "Request network error, url={}", fetch->url);
        m_stCurrentTask->State = TASK_ERROR;
        m_stCurrentTask->ErrorCode = ec;
        return;
    }

    // 成功，获取请求数据
    auto responseHeaders = GetResponseHeaders(fetch);
    if (!responseHeaders)
    {
        LSTG_LOG_ERROR_CAT(EmFileDownloader, "Get response header error, url={}, error={}", fetch->url, responseHeaders.GetError());
        m_stCurrentTask->State = TASK_ERROR;
        m_stCurrentTask->ErrorCode = responseHeaders.GetError();
        return;
    }

    // 遍历头部获取信息
    optional<uint64_t> contentLength;
    bool acceptRange = false;
    detail::VisitHeader(*responseHeaders, [&](string_view key, string_view value) -> Result<void> {
        if (detail::CaseInsensitiveCompare(key, "Content-Length") == 0)
        {
            uint64_t v = 0;
            auto [ptr, ec2] = std::from_chars(value.begin(), value.end(), v);
            if (ec2 == std::errc())
                contentLength = v;
        }
        else if (detail::CaseInsensitiveCompare(key, "Accept-Ranges") == 0)
        {
            if (value.find("bytes") != string_view::npos)
                acceptRange = true;
        }
        return {};
    });
    if (!acceptRange)
    {
        LSTG_LOG_ERROR_CAT(EmFileDownloader, "Server not accept range request, url={}", fetch->url);
        m_stCurrentTask->State = TASK_ERROR;
        m_stCurrentTask->ErrorCode = make_error_code(errc::no_protocol_option);
        return;
    }
    if (!contentLength)
    {
        LSTG_LOG_ERROR_CAT(EmFileDownloader, "Server not send content length, url={}", fetch->url);
        m_stCurrentTask->State = TASK_ERROR;
        m_stCurrentTask->ErrorCode = make_error_code(errc::protocol_not_supported);
        return;
    }

    // 刷新 ContentLength
    m_stCurrentTask->ContentLength = *contentLength;

    // 发起读 Chunk 操作
    FetchNextChunk();
}

void EmFileDownloader::OnHandleReadChunk(std::error_code ec, emscripten_fetch_t* fetch) noexcept
{
    if (fetch->status > 0 && fetch->status != 200 && fetch->status != 206)
    {
        LSTG_LOG_ERROR_CAT(EmFileDownloader, "Request HTTP error, url={}, status={}", fetch->url, fetch->status);
        m_stCurrentTask->State = TASK_ERROR;
        if (fetch->status == 404)
            m_stCurrentTask->ErrorCode = make_error_code(errc::no_such_file_or_directory);
        else if (fetch->status == 403)
            m_stCurrentTask->ErrorCode = make_error_code(errc::permission_denied);
        else if (fetch->status == 400)
            m_stCurrentTask->ErrorCode = make_error_code(errc::protocol_error);
        else
            m_stCurrentTask->ErrorCode = make_error_code(errc::device_or_resource_busy);
        return;
    }

    if (ec)
    {
        LSTG_LOG_ERROR_CAT(EmFileDownloader, "Request network error, url={}", fetch->url);
        m_stCurrentTask->State = TASK_ERROR;
        m_stCurrentTask->ErrorCode = ec;
        return;
    }

    // 获取数据
    if (fetch->numBytes != m_stCurrentTask->CurrentReadSize)
    {
        LSTG_LOG_ERROR_CAT(EmFileDownloader, "Server response {} bytes, but {} bytes expected, url={}", fetch->numBytes,
            m_stCurrentTask->CurrentReadSize, fetch->url);
        m_stCurrentTask->State = TASK_ERROR;
        m_stCurrentTask->ErrorCode = make_error_code(errc::protocol_error);
        return;
    }

    // 写出数据
    assert(m_stCurrentTask->FileStream);
    auto ret = m_stCurrentTask->FileStream->Write(reinterpret_cast<const uint8_t*>(fetch->data), fetch->numBytes);
    if (!ret)
    {
        LSTG_LOG_ERROR_CAT(EmFileDownloader, "Write to output file fail, path={}, ec={}", m_stCurrentTask->SaveAs, ret.GetError());
        m_stCurrentTask->State = TASK_ERROR;
        m_stCurrentTask->ErrorCode = ret.GetError();
        return;
    }
    m_stCurrentTask->ReadSize += fetch->numBytes;

    if (m_stCurrentTask->ProgressCallback)
    {
        m_stCurrentTask->ProgressCallback(m_stCurrentTask->Url, m_stCurrentTask->SaveAs,
            m_stCurrentTask->ReadSize / static_cast<float>(m_stCurrentTask->ContentLength));
    }

    // 发起下一个读
    FetchNextChunk();
}

#endif
