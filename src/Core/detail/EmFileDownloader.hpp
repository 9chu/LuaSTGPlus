/**
 * @file
 * @date 2022/8/16
 * @author 9chu
 * 此文件为 LuaSTGPlus 项目的一部分，版权与许可声明详见 COPYRIGHT.txt。
 */
#pragma once
#include <string>
#include <vector>
#include <optional>
#include <functional>
#include <lstg/Core/Result.hpp>
#include <lstg/Core/Subsystem/VFS/FileStream.hpp>

struct emscripten_fetch_t;

namespace lstg::detail
{
    using DownloadCallback = std::function<void(std::error_code) /* noexcept */>;
    using DownloadProgressCallback = std::function<void(std::string_view /* url */, std::string_view /* saveAs */,
        float /* percent */) /* noexcept */>;

    /**
     * 文件下载器
     */
    class EmFileDownloader
    {
    public:
        EmFileDownloader();
        ~EmFileDownloader();

    public:
        /**
         * 获取请求超时时间
         */
        uint32_t GetRequestTimeoutMs() const noexcept { return m_uRequestTimeout; }

        /**
         * 设置请求超时时间
         * @param ts 时间
         */
        void SetRequestTimeoutMs(uint32_t ts) noexcept { m_uRequestTimeout = ts; }

        /**
         * 获取任务数
         */
        size_t GetTaskCount() const noexcept;

        /**
         * 增加任务
         * @param url URL
         * @param saveAs 存储到
         * @param callback 回调
         * @param progressCallback 进度回调
         * @return 是否成功
         */
        Result<void> AddTask(std::string_view url, std::string_view saveAs, DownloadCallback callback,
            DownloadProgressCallback progressCallback) noexcept;

        /**
         * 取消所有等待的任务
         * @note 不触发 callback
         */
        void CancelPendingTasks() noexcept;

        /**
         * 更新状态
         */
        void Update() noexcept;

    private:
        void FetchNextChunk() noexcept;
        void OnHandleGetFileInfo(std::error_code ec, emscripten_fetch_t* fetch) noexcept;
        void OnHandleReadChunk(std::error_code ec, emscripten_fetch_t* fetch) noexcept;

    private:
        enum TaskStates
        {
            TASK_READY,
            TASK_GET_FILE_INFO,
            TASK_READ_CHUNK,
            TASK_ERROR,
            TASK_FINISHED,
        };

        struct Task
        {
            std::string Url;
            std::string SaveAs;
            DownloadCallback Callback;
            DownloadProgressCallback ProgressCallback;

            TaskStates State = TASK_READY;
            std::error_code ErrorCode;
            size_t ContentLength = 0;
            size_t ReadSize = 0;  // 已读取
            size_t CurrentReadSize = 0;  // 当前操作正在读取的数量
            Subsystem::VFS::StreamPtr FileStream;
        };

        uint32_t m_uRequestTimeout = 30 * 1000;  // 30s
        uint32_t m_uMaxReadChunkSize = 1 * 1024 * 1024;  // 1M
        std::vector<Task> m_stPendingTasks;
        std::optional<Task> m_stCurrentTask;
    };
}
