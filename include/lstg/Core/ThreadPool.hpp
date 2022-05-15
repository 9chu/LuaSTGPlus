/**
 * @file
 * @date 2022/5/14
 * @author 9chu
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#pragma once
#include <cstdint>
#include <cassert>
#include <chrono>
#include <system_error>
#include <variant>
#include <deque>
#include <vector>
#include <functional>
#if !(defined(__EMSCRIPTEN__) && !defined(__EMSCRIPTEN_PTHREADS__))
#include <thread>
#include <mutex>
#include <condition_variable>
#endif

namespace lstg
{
    struct SingleThreadModeTag {};
    struct MultiThreadModeTag {};

#if defined(__EMSCRIPTEN__) && !defined(__EMSCRIPTEN_PTHREADS__)
    template <typename Mode = SingleThreadModeTag>
    class ThreadPool;
#else
    template <typename Mode = MultiThreadModeTag>
    class ThreadPool;
#endif

    namespace detail
    {
        template <typename T>
        struct ThreadJobCompletedCallbackType
        {
            using Callback = std::function<void(std::error_code, T)>;
        };

        template <>
        struct ThreadJobCompletedCallbackType<void>
        {
            using Callback = std::function<void(std::error_code)>;
        };
    }

    template <typename T>
    using ThreadJobCallback = std::function<T()>;

    template <typename T>
    using ThreadJobCompletedCallback = typename detail::ThreadJobCompletedCallbackType<T>::Callback;

    namespace detail
    {
        class IThreadJob
        {
        public:
            IThreadJob() = default;
            virtual ~IThreadJob() noexcept = default;

        public:
            /**
             * 执行任务
             */
            virtual void Execute() noexcept = 0;

            /**
             * 执行回调
             * @note 总是在主线程
             */
            virtual void CallHandler() noexcept = 0;
        };

        using ThreadJobPtr = std::shared_ptr<IThreadJob>;

        template <typename T>
        class ThreadJob :
            public IThreadJob
        {
        public:
            ThreadJob(ThreadJobCallback<T> job, ThreadJobCompletedCallback<T> cb)
                : m_stJob(std::move(job)), m_stCompletedCallback(std::move(cb)) {}

        public:
            void Execute() noexcept override
            {
                try
                {
                    m_stResult.emplace(std::in_place_index_t<1>{}, m_stJob());
                }
                catch (const std::system_error& ex)
                {
                    m_stResult.emplace(std::in_place_index_t<0>{}, ex.code());
                }
                catch (...)
                {
                    // 不明原因异常均按照内存不足处理
                    // FIXME: 考虑增加错误类型
                    m_stResult.emplace(std::in_place_index_t<0>{}, make_error_code(std::errc::not_enough_memory));
                }
            }

            void CallHandler() noexcept override
            {
                if (m_stCompletedCallback)
                {
                    try
                    {
                        if (m_stResult.index() == 0)
                            m_stCompletedCallback(std::get<0>(m_stResult), T{});
                        else
                            m_stCompletedCallback({}, std::move(std::get<1>(m_stResult)));
                    }
                    catch (...)
                    {
                        // FIXME: 考虑增加日志
                    }
                }
            }

        private:
            std::variant<std::error_code, T> m_stResult;
            ThreadJobCallback<T> m_stJob;
            ThreadJobCompletedCallback<T> m_stCompletedCallback;
        };

        template <>
        class ThreadJob<void> :
            public IThreadJob
        {
        public:
            ThreadJob(ThreadJobCallback<void> job, ThreadJobCompletedCallback<void> cb)
                : m_stJob(std::move(job)), m_stCompletedCallback(std::move(cb)) {}

        public:
            void Execute() noexcept override
            {
                try
                {
                    m_stJob();
                }
                catch (const std::system_error& ex)
                {
                    m_stResult = ex.code();
                }
                catch (...)
                {
                    // 不明原因异常均按照内存不足处理
                    // FIXME: 考虑增加错误类型
                    m_stResult = make_error_code(std::errc::not_enough_memory);
                }
            }

            void CallHandler() noexcept override
            {
                if (m_stCompletedCallback)
                {
                    try
                    {
                        m_stCompletedCallback(m_stResult);
                    }
                    catch (...)
                    {
                        // FIXME: 考虑增加日志
                    }
                }
            }

        private:
            std::error_code m_stResult;
            ThreadJobCallback<void> m_stJob;
            ThreadJobCompletedCallback<void> m_stCompletedCallback;
        };
    }

    /**
     * 线程池
     * 单线程特化，用于单核系统环境。
     */
    template <>
    class ThreadPool<SingleThreadModeTag>
    {
    public:
        static inline uint32_t GetSystemThreadCount() noexcept
        {
            return 1u;
        }

    public:
        /**
         * 构造线程池
         * @param maxUpdateIntervalMs 最大更新时间间隔，当任务调度耗时超过该值则 Update 方法会直接跳出
         */
        ThreadPool(uint32_t /* workThreadCount */, uint32_t maxUpdateIntervalMs = 100)
            : m_uMaxUpdateIntervalMs(maxUpdateIntervalMs)
        {}

    public:
        /**
         * 提交任务
         * @tparam T 任务返回值
         * @param job 任务
         * @param completedCallback 任务完成回调，若为空，则不执行回调
         */
        template <typename T>
        void Commit(ThreadJobCallback<T> job, ThreadJobCompletedCallback<T> completedCallback = {})
        {
            assert(job);
            auto wrapper = std::make_shared<detail::ThreadJob<T>>(std::move(job), std::move(completedCallback));
            m_stJobQueue.emplace_back(std::move(wrapper));
        }

        /**
         * 更新状态
         */
        void Update() noexcept
        {
            if (m_stJobQueue.empty())
                return;

            auto start = std::chrono::steady_clock::now();
            while (!m_stJobQueue.empty())
            {
                auto wrapper = m_stJobQueue.front();
                m_stJobQueue.pop_front();

                // 执行任务
                wrapper->Execute();

                // 执行 CompletedHandler
                wrapper->CallHandler();

                // 如果超过时间限制，无论如何都跳出
                auto end = std::chrono::steady_clock::now();
                if (std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() > m_uMaxUpdateIntervalMs)
                    break;
            }
        }

    private:
        const uint32_t m_uMaxUpdateIntervalMs;
        std::deque<detail::ThreadJobPtr> m_stJobQueue;
    };

#if !(defined(__EMSCRIPTEN__) && !defined(__EMSCRIPTEN_PTHREADS__))
    /**
     * 线程池
     */
    template <>
    class ThreadPool<MultiThreadModeTag>
    {
    public:
        static inline uint32_t GetSystemThreadCount() noexcept
        {
            return std::thread::hardware_concurrency();
        }

    public:
        /**
         * 构造线程池
         * @param workThreadCount 工作线程数量
         * @param maxUpdateIntervalMs 最大更新时间间隔，当任务调度耗时超过该值则 Update 方法会直接跳出
         */
        ThreadPool(uint32_t workThreadCount, uint32_t maxUpdateIntervalMs = 100)
            : m_uMaxUpdateIntervalMs(maxUpdateIntervalMs)
        {
            m_bThreadStopped.store(false, std::memory_order_release);
            InitWorkThreads(std::max(1u, workThreadCount));
        }

        ThreadPool(const ThreadPool&) = delete;
        ThreadPool(ThreadPool&&) = delete;

        ~ThreadPool() noexcept
        {
            DestroyWorkThreads();
        }

    public:
        /**
         * 提交任务
         * @tparam T 任务返回值
         * @param job 任务
         * @param completedCallback 任务完成回调，若为空，则不执行回调
         */
        template <typename T>
        void Commit(ThreadJobCallback<T> job, ThreadJobCompletedCallback<T> completedCallback = {})
        {
            assert(job);
            auto wrapper = std::make_shared<detail::ThreadJob<T>>(std::move(job), std::move(completedCallback));

            // 放入队列
            {
                std::unique_lock<std::mutex> lockGuard(m_stMutex);
                m_stJobQueue.emplace_back(std::move(wrapper));

                // 通知工作线程
                lockGuard.unlock();
                m_stCondVar.notify_one();
            }
        }

        /**
         * 更新状态
         */
        void Update() noexcept
        {
            std::unique_lock<std::mutex> lockGuard(m_stPendingJobMutex);

            if (m_stPendingCompletedJobQueue.empty())
                return;

            auto start = std::chrono::steady_clock::now();
            while (!m_stPendingCompletedJobQueue.empty())
            {
                auto wrapper = m_stPendingCompletedJobQueue.front();
                m_stPendingCompletedJobQueue.pop_front();

                // 执行 CompletedHandler
                wrapper->CallHandler();

                // 如果超过时间限制，无论如何都跳出
                auto end = std::chrono::steady_clock::now();
                if (std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() > m_uMaxUpdateIntervalMs)
                    break;
            }
        }

    private:
        void ThreadJob() noexcept
        {
            while (!m_bThreadStopped.load(std::memory_order_acquire))
            {
                std::unique_lock<std::mutex> lockGuard(m_stMutex);

                if (m_stJobQueue.empty())
                {
                    m_stCondVar.wait(lockGuard);

                    // 用于解决伪唤醒，或接收到终止信号
                    if (m_stJobQueue.empty())
                        continue;
                }

                // 获取一个任务
                assert(!m_stJobQueue.empty());
                auto wrapper = m_stJobQueue.front();
                m_stJobQueue.pop_front();

                // 此时可以解锁 lockGuard
                lockGuard.unlock();

                // 调度任务
                wrapper->Execute();

                // 任务完成后放入完成队列
                {
                    std::unique_lock<std::mutex> completedLockGuard(m_stPendingJobMutex);
                    m_stPendingCompletedJobQueue.emplace_back(std::move(wrapper));
                }
            }
        }

        void InitWorkThreads(uint32_t cnt)
        {
            for (uint32_t i = 0; i < cnt; ++i)
            {
                m_stThreads.emplace_back(std::thread([this]() {
                    ThreadJob();
                }));
            }
        }

        void DestroyWorkThreads() noexcept
        {
            m_bThreadStopped.store(true, std::memory_order_release);
            m_stCondVar.notify_all();
            for (auto& thread : m_stThreads)
                thread.join();
        }

    private:
        const uint32_t m_uMaxUpdateIntervalMs;
        std::atomic<bool> m_bThreadStopped;
        std::vector<std::thread> m_stThreads;

        std::mutex m_stMutex;
        std::condition_variable m_stCondVar;
        std::deque<detail::ThreadJobPtr> m_stJobQueue;

        std::mutex m_stPendingJobMutex;
        std::deque<detail::ThreadJobPtr> m_stPendingCompletedJobQueue;
    };
#endif
}
