/**
 * @file
 * @date 2023/8/25
 * @author 9chu
 * 此文件为 LuaSTGPlus 项目的一部分，版权与许可声明详见 COPYRIGHT.txt。
 */
#pragma once
#include <cassert>
#include <type_traits>
#include "IStream.hpp"
#include "../../Span.hpp"

namespace lstg::Subsystem::VFS
{
    /**
     * 指向定长缓冲区的 Stream
     * 不持有 Buffer 对象
     */
    template <typename T>
    class BufferViewStream : public IStream
    {
    public:
        BufferViewStream(Span<T> buffer) noexcept
            : m_stBuffer(buffer)
        {
        }

        BufferViewStream(const BufferViewStream& rhs) noexcept
            : m_stBuffer(rhs.m_stBuffer), m_uPosition(rhs.m_uPosition)
        {
        }

        BufferViewStream(BufferViewStream&& rhs) noexcept
            : m_stBuffer(rhs.m_stBuffer), m_uPosition(rhs.m_uPosition)
        {
            rhs.m_stBuffer = {};
            rhs.m_uPosition = 0;
        }

        BufferViewStream& operator=(const BufferViewStream& rhs) noexcept
        {
            if (this == &rhs)
                return *this;

            m_stBuffer = rhs.m_stBuffer;
            m_uPosition = rhs.m_uPosition;
            return *this;
        }

        BufferViewStream& operator=(BufferViewStream&& rhs) noexcept
        {
            if (this == &rhs)
                return *this;

            m_stBuffer = rhs.m_stBuffer;
            m_uPosition = rhs.m_uPosition;
            rhs.m_stBuffer = {};
            rhs.m_uPosition = 0u;
            return *this;
        }

    public:  // IStream
        bool IsReadable() const noexcept override { return true; }

        bool IsWriteable() const noexcept override { return !std::is_const_v<T>; }

        bool IsSeekable() const noexcept override { return true; }

        Result<uint64_t> GetLength() const noexcept override { return m_stBuffer.GetSize(); }

        Result<void> SetLength(uint64_t length) noexcept override { return std::make_error_code(std::errc::not_supported); }

        Result<uint64_t> GetPosition() const noexcept override { return m_uPosition; }

        Result<void> Seek(int64_t offset, StreamSeekOrigins origin) noexcept override
        {
            switch (origin)
            {
                case StreamSeekOrigins::Begin:
                    m_uPosition = static_cast<uint64_t>(std::max<int64_t>(0, offset));
                    m_uPosition = std::min<uint64_t>(m_uPosition, m_stBuffer.GetSize());
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
                        m_uPosition = std::min<uint64_t>(m_uPosition, m_stBuffer.GetSize());
                    }
                    break;
                case StreamSeekOrigins::End:
                    if (offset >= 0)
                    {
                        m_uPosition = m_stBuffer.GetSize();
                    }
                    else
                    {
                        auto positive = static_cast<uint64_t>(-offset);
                        if (positive >= m_stBuffer.GetSize())
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
            return m_uPosition >= m_stBuffer.GetSize();
        }

        Result<void> Flush() noexcept override
        {
            return make_error_code(std::errc::not_supported);
        }

        Result<size_t> Read(uint8_t* buffer, size_t length) noexcept override
        {
            if (length == 0)
                return 0u;
            assert(buffer);
            assert(m_stBuffer.GetSize() >= m_uPosition);
            if (m_uPosition == m_stBuffer.GetSize())
                return 0u;
            length = std::min<uint64_t>(length, m_stBuffer.GetSize() - m_uPosition);
            ::memcpy(buffer, m_stBuffer.GetData() + m_uPosition, length);
            m_uPosition += length;
            return length;
        }

        Result<void> Write(const uint8_t* buffer, size_t length) noexcept override
        {
            if constexpr (std::is_const_v<T>)
            {
                return make_error_code(std::errc::not_supported);
            }
            else
            {
                if (length == 0)
                    return {};

                assert(buffer);
                if (m_uPosition + length > m_stBuffer.GetSize())
                    return std::make_error_code(std::errc::result_out_of_range);

                assert(m_uPosition + length <= m_stBuffer.GetSize());
                ::memcpy(m_stBuffer.GetData() + m_uPosition, buffer, length);
                m_uPosition += length;
                return {};
            }
        }

        Result<StreamPtr> Clone() const noexcept override
        {
            try
            {
                return std::make_shared<BufferViewStream<T>>(*this);
            }
            catch (...)  // bad_alloc
            {
                return std::make_error_code(std::errc::not_enough_memory);
            }
        }

    private:
        Span<T> m_stBuffer;
        size_t m_uPosition = 0u;
    };
}  // namespace lstg::Subsystem::VFS
