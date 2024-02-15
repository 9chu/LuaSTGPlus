/**
 * @file
 * @date 2022/2/28
 * @author 9chu
 * 此文件为 LuaSTGPlus 项目的一部分，版权与许可声明详见 COPYRIGHT.txt。
 */
#include <lstg/Core/Subsystem/VFS/InflateStream.hpp>

#include <lstg/Core/Logging.hpp>
#include "detail/ZStream.hpp"
#include "detail/ZLibError.hpp"

LSTG_DEF_LOG_CATEGORY(InflateStream);

using namespace std;
using namespace lstg;
using namespace lstg::Subsystem::VFS;

InflateStream::InflateStream(StreamPtr underlayStream, std::optional<uint64_t> uncompressedSize)
    : m_pZStream(std::make_shared<detail::ZStream>(detail::InflateInitTag{}, true)), m_pUnderlayStream(std::move(underlayStream)),
    m_stUncompressedSize(uncompressedSize)
{
    assert(m_pUnderlayStream);
    (**m_pZStream)->next_in = m_stChunk;
    (**m_pZStream)->avail_in = 0;
}

InflateStream::InflateStream(const InflateStream& org)
    : m_pZStream(std::make_shared<detail::ZStream>(*org.m_pZStream)), m_bFinished(org.m_bFinished),
    m_stUncompressedSize(org.m_stUncompressedSize)
{
    auto clone = org.m_pUnderlayStream->Clone();
    m_pUnderlayStream = clone.ThrowIfError();

    // 拷贝后，校准输入缓冲区
    auto zstream = (*m_pZStream);
    if (zstream->next_in)
    {
        auto offset = zstream->next_in - org.m_stChunk;
        assert(offset <= static_cast<long>(sizeof(m_stChunk)));
        ::memcpy(m_stChunk, org.m_stChunk, sizeof(m_stChunk));
        zstream->next_in = m_stChunk + offset;
    }
    else
    {
        zstream->next_in = m_stChunk;
    }
    zstream->next_out = nullptr;
    zstream->avail_out = 0;
}

bool InflateStream::IsReadable() const noexcept
{
    return true;
}

bool InflateStream::IsWriteable() const noexcept
{
    return false;
}

bool InflateStream::IsSeekable() const noexcept
{
    return false;
}

Result<uint64_t> InflateStream::GetLength() const noexcept
{
    if (m_stUncompressedSize)
        return *m_stUncompressedSize;
    return make_error_code(errc::not_supported);
}

Result<void> InflateStream::SetLength(uint64_t length) noexcept
{
    return make_error_code(errc::not_supported);
}

Result<uint64_t> InflateStream::GetPosition() const noexcept
{
    return (**m_pZStream)->total_out;
}

Result<void> InflateStream::Seek(int64_t offset, StreamSeekOrigins origin) noexcept
{
    return make_error_code(errc::not_supported);
}

Result<bool> InflateStream::IsEof() const noexcept
{
    return m_bFinished;
}

Result<void> InflateStream::Flush() noexcept
{
    return m_pUnderlayStream->Flush();
}

Result<size_t> InflateStream::Read(uint8_t* buffer, size_t length) noexcept
{
    if (length == 0 || m_bFinished)
        return static_cast<size_t>(0u);

    auto z = (**m_pZStream);
    z->avail_out = length;
    z->next_out = buffer;

    bool eof = false;
    do
    {
        // 移动上一次没读完的数据
        if (z->avail_in > 0)
            ::memmove(m_stChunk, z->next_in, z->avail_in);
        z->next_in = m_stChunk;

        // 如果还可以读数据，则填满输入缓冲区
        if (!eof)
        {
            auto fill = sizeof(m_stChunk) - z->avail_in;
            auto count = m_pUnderlayStream->Read(m_stChunk + z->avail_in, fill);
            if (!count)
                return count.GetError();
            eof = (*count < fill);
            z->avail_in += *count;
        }

        // 进行解压操作
        auto ret = ::inflate(z, Z_NO_FLUSH);
        switch (ret)
        {
            case Z_NEED_DICT:
            case Z_DATA_ERROR:
                LSTG_LOG_ERROR_CAT(InflateStream, "zng_inflate error {}, {}", ret, z->msg);
                return make_error_code(static_cast<detail::ZLibError>(ret));
            case Z_MEM_ERROR:
                return make_error_code(errc::not_enough_memory);  // 转换到 errc
            case Z_BUF_ERROR:
                assert(z->avail_out == 0);
                return length - z->avail_out;
            default:
                assert(ret != Z_STREAM_ERROR);
                break;
        }

        if (ret == Z_STREAM_END)
        {
            m_bFinished = true;
            break;
        }
    }
    while (z->avail_out != 0);

    return length - z->avail_out;
}

Result<void> InflateStream::Write(const uint8_t* buffer, size_t length) noexcept
{
    return make_error_code(errc::not_supported);
}

Result<StreamPtr> InflateStream::Clone() const noexcept
{
    try
    {
        return make_shared<InflateStream>(*this);
    }
    catch (const system_error& ex)
    {
        return ex.code();
    }
    catch (...)  // bad_alloc
    {
        return make_error_code(errc::not_enough_memory);
    }
}

Result<void> InflateStream::Reset() noexcept
{
    detail::ZStreamPtr reset;
    try
    {
        reset = std::make_shared<detail::ZStream>(detail::InflateInitTag{});
    }
    catch (...)  // bad_alloc
    {
        return make_error_code(errc::not_enough_memory);
    }

    auto ret = m_pUnderlayStream->Seek(0, StreamSeekOrigins::Begin);
    if (!ret)
        return ret.GetError();

    m_pZStream = reset;
    m_bFinished = false;
    return {};
}
