/**
 * @file
 * @date 2022/2/28
 * @author 9chu
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#include <lstg/Core/Subsystem/VFS/WindowedStream.hpp>

using namespace std;
using namespace lstg;
using namespace lstg::Subsystem::VFS;

WindowedStream::WindowedStream(StreamPtr underlay, uint64_t maxLength) noexcept
    : m_pUnderlay(std::move(underlay)), m_ullMaxLength(maxLength), m_ullLength(maxLength)
{
}

bool WindowedStream::IsReadable() const noexcept
{
    return m_pUnderlay->IsReadable();
}

bool WindowedStream::IsWriteable() const noexcept
{
    return m_pUnderlay->IsWriteable();
}

bool WindowedStream::IsSeekable() const noexcept
{
    return m_pUnderlay->IsSeekable();
}

Result<uint64_t> WindowedStream::GetLength() const noexcept
{
    return m_ullLength;
}

Result<void> WindowedStream::SetLength(uint64_t length) noexcept
{
    if (length > m_ullMaxLength)
        return make_error_code(errc::result_out_of_range);

    if (length >= m_ullLength)
    {
        m_ullLength = length;
        return {};
    }
    else
    {
        if (m_ullPosition > length)
        {
            auto shift = m_ullPosition - length;
            auto seek = m_pUnderlay->Seek(-static_cast<int64_t>(shift), StreamSeekOrigins::Current);
            if (!seek)
                return seek.GetError();
            m_ullPosition -= shift;
            assert(m_ullPosition == length);
        }
        m_ullLength = length;
        return {};
    }
}

Result<uint64_t> WindowedStream::GetPosition() const noexcept
{
    return m_ullPosition;
}

Result<void> WindowedStream::Seek(int64_t offset, StreamSeekOrigins origin) noexcept
{
    // 总是把绝对偏移转换到相对偏移
    if (origin == StreamSeekOrigins::End)  // 转换到 Begin 进行处理
    {
        assert(m_ullLength < std::numeric_limits<int64_t>::max());  // 谁没事干搞这么大？
        offset = static_cast<int64_t>(m_ullLength) + offset;
        origin = StreamSeekOrigins::Begin;
    }

    if (origin == StreamSeekOrigins::Begin)  // 转换到 Current 进行处理
    {
        if (static_cast<uint64_t>(offset) > m_ullPosition)
            offset = static_cast<int64_t>(static_cast<uint64_t>(offset) - m_ullPosition);
        else
            offset = -static_cast<int64_t>(m_ullPosition - static_cast<uint64_t>(offset));
        origin = StreamSeekOrigins::Current;
    }

    assert(origin == StreamSeekOrigins::Current);

    // 在这里进行范围限定
    if (offset < 0)
    {
        offset = -static_cast<int64_t>(std::min(m_ullPosition, static_cast<uint64_t>(-offset)));
    }
    else
    {
        auto rest = m_ullLength - m_ullPosition;
        offset = static_cast<int64_t>(std::min(rest, static_cast<uint64_t>(offset)));
    }

    // 底层发起 seek
    auto seek = m_pUnderlay->Seek(offset, StreamSeekOrigins::Current);
    if (!seek)
        return seek.GetError();

    // 成功刷新 Position
    if (offset < 0)
    {
        assert(static_cast<uint64_t>(-offset) <= m_ullPosition);
        m_ullPosition -= static_cast<uint64_t>(-offset);
    }
    else
    {
        m_ullPosition += static_cast<uint64_t>(offset);
        assert(m_ullPosition <= m_ullLength);
    }
    return {};
}

Result<bool> WindowedStream::IsEof() const noexcept
{
    return m_ullPosition >= m_ullLength;
}

Result<void> WindowedStream::Flush() noexcept
{
    return m_pUnderlay->Flush();
}

Result<size_t> WindowedStream::Read(uint8_t* buffer, size_t length) noexcept
{
    auto rest = m_ullLength - m_ullPosition;
    length = static_cast<size_t>(std::min<uint64_t>(length, rest));
    auto ret = m_pUnderlay->Read(buffer, length);
    if (!ret)
        return ret.GetError();
    assert(*ret <= rest);
    m_ullPosition += *ret;
    return ret;
}

Result<void> WindowedStream::Write(const uint8_t* buffer, size_t length) noexcept
{
    auto rest = m_ullLength - m_ullPosition;
    if (rest < length)
        return make_error_code(errc::result_out_of_range);
    auto ret = m_pUnderlay->Write(buffer, length);
    if (!ret)
        return ret.GetError();
    m_ullPosition += length;
    return ret;
}

Result<StreamPtr> WindowedStream::Clone() const noexcept
{
    // 先复制底层
    auto stream = m_pUnderlay->Clone();
    if (!stream)
        return stream.GetError();

    try
    {
        auto clone = make_shared<WindowedStream>(std::move(*stream), m_ullMaxLength);
        clone->m_ullLength = m_ullLength;
        clone->m_ullPosition = m_ullPosition;
        return clone;
    }
    catch (...)  // bad_alloc
    {
        return make_error_code(errc::not_enough_memory);
    }
}
