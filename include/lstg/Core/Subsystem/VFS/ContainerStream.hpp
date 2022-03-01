/**
 * @file
 * @date 2022/2/27
 * @author 9chu
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#pragma once
#include <cassert>
#include "IStream.hpp"
#include "../../Span.hpp"

namespace lstg::Subsystem::VFS
{
    namespace detail
    {
        template <class, class = void>
        struct IsResizableContainer : std::false_type {};

        template <class T>
        struct IsResizableContainer<T, std::void_t<decltype(std::declval<T&>.resize(0u))>> : std::true_type {};

        template <class, class = void>
        struct IsReservableContainer : std::false_type {};

        template <class T>
        struct IsReservableContainer<T, std::void_t<decltype(std::declval<T&>.reserve(0u))>> : std::true_type {};
    }  // namespace detail

    /**
     * 到容器的数据流
     * 容器必须满足连续的要求。
     */
    template <typename T>
    class ContainerStream : public IStream
    {
        static_assert(lstg::detail::IsContiguousContainer<T, uint8_t>::value);

        using SharedContainer = std::shared_ptr<T>;

    public:
        /**
         * 构造容器存储的流
         */
        ContainerStream()
        {
            m_pContainer = std::make_shared<T>();
        }

        /**
         * 转移一个外部的容器
         * @param container 容器对象
         */
        ContainerStream(T&& container)
        {
            m_pContainer = std::make_shared<T>(std::move(container));
        }

        /**
         * 构造容器存储的流
         * @param sz 初始化大小
         */
        template <typename P = T>
        explicit ContainerStream(size_t sz, typename std::enable_if<detail::IsResizableContainer<P>::value>::type = {})
        {
            m_pContainer = std::make_shared<P>();
            m_pContainer->resize(sz);
        }

        /**
         * 构造容器存储的流
         * @param sz 初始化大小（填0）
         * @param preAllocate 预分配大小
         */
        template <typename P = T>
        explicit ContainerStream(size_t sz, size_t preAllocate,
            typename std::enable_if<detail::IsResizableContainer<P>::value && detail::IsReservableContainer<P>::value>::type = {})
        {
            m_pContainer = std::make_shared<T>();
            m_pContainer->reserve(preAllocate);
            m_pContainer->resize(sz);
        }

        ContainerStream(const ContainerStream&) = default;
        ContainerStream(ContainerStream&&) noexcept = default;

        ContainerStream& operator=(const ContainerStream&) = default;
        ContainerStream& operator=(ContainerStream&&) noexcept = default;

    public:  // for IStream
        bool IsReadable() const noexcept override { return true; }

        bool IsWriteable() const noexcept override { return true; }

        bool IsSeekable() const noexcept override { return true; }

        Result<uint64_t> GetLength() const noexcept override { return m_pContainer->size(); }

        Result<void> SetLength(uint64_t length) noexcept override
        {
            if constexpr (detail::IsResizableContainer<T>::value)
            {
                try
                {
                    CopyIfRequired();
                    assert(m_pContainer->use_count() == 1);
                    m_pContainer->resize(length);
                }
                catch (...)  // bad_alloc
                {
                    return std::make_error_code(std::errc::not_enough_memory);
                }
                m_uPosition = m_pContainer->size();
                return {};
            }
            else
            {
                return std::make_error_code(std::errc::not_supported);
            }
        }

        Result<uint64_t> GetPosition() const noexcept override { return m_uPosition; }

        Result<void> Seek(int64_t offset, StreamSeekOrigins origin) noexcept override
        {
            switch (origin)
            {
                case StreamSeekOrigins::Begin:
                    m_uPosition = static_cast<uint64_t>(std::max<int64_t>(0, offset));
                    m_uPosition = std::min<uint64_t>(m_uPosition, m_pContainer->size());
                    break;
                case StreamSeekOrigins::Current:
                    if (offset < 0)
                    {
                        auto positive = static_cast<uint64_t>(-offset);
                        if (positive >= m_uPosition)
                            m_uPosition = 0;
                        else
                            m_uPosition -= positive;
                    }
                    else
                    {
                        m_uPosition += static_cast<uint64_t>(offset);
                        m_uPosition = std::min<uint64_t>(m_uPosition, m_pContainer->size());
                    }
                    break;
                case StreamSeekOrigins::End:
                    if (offset >= 0)
                    {
                        m_uPosition = m_pContainer->size();
                    }
                    else
                    {
                        auto positive = static_cast<uint64_t>(-offset);
                        if (positive >= m_pContainer->size())
                            m_uPosition = 0;
                        else
                            m_uPosition -= positive;
                    }
                    break;
            }
            return {};
        }

        Result<bool> IsEof() const noexcept override
        {
            return m_uPosition >= m_pContainer->size();
        }

        Result<void> Flush() noexcept override { return make_error_code(std::errc::not_supported); }

        Result<size_t> Read(uint8_t* buffer, size_t length) noexcept override
        {
            if (length == 0)
                return 0u;
            assert(buffer);
            assert(m_pContainer->size() >= m_uPosition);
            if (m_uPosition == m_pContainer->size())
                return 0u;
            length = std::min<uint64_t>(length, m_pContainer->size() - m_uPosition);
            ::memcpy(buffer, m_pContainer->data() + m_uPosition, length);
            m_uPosition += length;
            return length;
        }

        Result<void> Write(const uint8_t* buffer, size_t length) noexcept override
        {
            if (length == 0)
                return {};

            try
            {
                CopyIfRequired();
                assert(m_pContainer.use_count() == 1);
            }
            catch (...)  // bad_alloc
            {
                return std::make_error_code(std::errc::not_enough_memory);
            }

            assert(buffer);
            if (m_uPosition + length > m_pContainer->size())
            {
                if constexpr (detail::IsResizableContainer<T>::value)
                {
                    try
                    {
                        m_pContainer->resize(m_uPosition + length);
                    }
                    catch (...)  // bad_alloc
                    {
                        return std::make_error_code(std::errc::not_enough_memory);
                    }
                }
                else
                {
                    return std::make_error_code(std::errc::result_out_of_range);
                }
            }

            assert(m_uPosition + length <= m_pContainer->size());
            ::memcpy(m_pContainer->data() + m_uPosition, buffer, length);
            m_uPosition += length;
            return {};
        }

        /**
         * 克隆操作
         * 通过 COW 机制保证 Clone 操作是轻量的。
         * 需要注意对同一个流的操作需要上锁或局限在某一线程中。
         * @return
         */
        Result<StreamPtr> Clone() const noexcept override
        {
            try
            {
                return std::make_shared<ContainerStream<T>>(*this);
            }
            catch (...)  // bad_alloc
            {
                return std::make_error_code(std::errc::not_enough_memory);
            }
        }

    public:
        /**
         * @brief 获取底层容器
         */
        T& GetContainer()
        {
            CopyIfRequired();
            assert(m_pContainer.use_count() == 1);
            return *m_pContainer;
        }

        /**
         * @brief 获取底层容器
         */
        const T& GetContainer() const noexcept
        {
            return *m_pContainer;
        }

        /**
         * @brief 预分配内存
         * @param sz 大小
         */
        template <typename P = T>
        void Reserve(size_t sz, typename std::enable_if<detail::IsReservableContainer<P>::value>::type = {})
        {
            CopyIfRequired();
            assert(m_pContainer.use_count() == 1);
            m_pContainer->reserve(sz);
        }

    private:
        void CopyIfRequired()
        {
            if (m_pContainer.use_count() > 1)
            {
                auto copy = std::make_shared<T>(*m_pContainer);
                m_pContainer = copy;
            }
        }

    private:
        uint64_t m_uPosition = 0;
        SharedContainer m_pContainer;
    };

    template <typename T>
    using ContainerStreamPtr = std::shared_ptr<ContainerStream<T>>;

    /**
     * 内存数据流
     */
    using MemoryStream = ContainerStream<std::vector<uint8_t>>;

    using MemoryStreamPtr = std::shared_ptr<MemoryStream>;

    /**
     * 缓冲区视图流
     */
    using BufferViewStream = ContainerStream<Span<uint8_t>>;

    using BufferViewStreamPtr = ContainerStream<Span<uint8_t>>;
}
