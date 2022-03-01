/**
 * @file
 * @date 2022/2/28
 * @author 9chu
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#include <lstg/Core/Subsystem/VFS/DeflateStream.hpp>

#include "detail/ZStream.hpp"
#include "detail/ZLibError.hpp"

using namespace std;
using namespace lstg;
using namespace lstg::Subsystem::VFS;

DeflateStream::DeflateStream(StreamPtr underlayStream)
    : m_pZStream(std::make_shared<detail::ZStream>(detail::DeflateInitTag{})), m_pUnderlayStream(std::move(underlayStream))
{
    assert(m_pUnderlayStream);
}

DeflateStream::DeflateStream(StreamPtr underlayStream, int compressionLevel)
    : m_pZStream(std::make_shared<detail::ZStream>(detail::DeflateInitTag{}, compressionLevel)),
    m_pUnderlayStream(std::move(underlayStream))
{
    assert(m_pUnderlayStream);
}

DeflateStream::~DeflateStream() {
    if (!m_bFinished)
    {
        auto ret = Finish();
        static_cast<void>(ret);
        assert(ret);
    }
}

bool DeflateStream::IsReadable() const noexcept
{
    return false;
}

bool DeflateStream::IsWriteable() const noexcept
{
    return true;
}

bool DeflateStream::IsSeekable() const noexcept
{
    return false;
}

Result<uint64_t> DeflateStream::GetLength() const noexcept
{
    return make_error_code(errc::not_supported);
}

Result<void> DeflateStream::SetLength(uint64_t length) noexcept
{
    return make_error_code(errc::not_supported);
}

Result<uint64_t> DeflateStream::GetPosition() const noexcept
{
    // 位置总是在末尾
    return (**m_pZStream)->total_in;
}

Result<void> DeflateStream::Seek(int64_t offset, StreamSeekOrigins origin) noexcept
{
    return make_error_code(errc::not_supported);
}

Result<bool> DeflateStream::IsEof() const noexcept
{
    return m_bFinished;
}

Result<void> DeflateStream::Flush() noexcept
{
    return m_pUnderlayStream->Flush();
}

Result<size_t> DeflateStream::Read(uint8_t* buffer, size_t length) noexcept
{
    return make_error_code(errc::not_supported);
}

Result<void> DeflateStream::Write(const uint8_t* buffer, size_t length) noexcept
{
    if (m_bFinished)
        return make_error_code(errc::invalid_argument);

    auto z = **m_pZStream;
    z->avail_in = length;
    z->next_in = const_cast<unsigned char*>(buffer);

    do
    {
        z->avail_out = sizeof(m_stChunk);
        z->next_out = m_stChunk;
        auto ret = ::zng_deflate(z, Z_NO_FLUSH);
        assert(ret != Z_STREAM_ERROR);

        // 写出
        auto have = sizeof(m_stChunk) - z->avail_out;
        if (have)
        {
            auto ec = m_pUnderlayStream->Write(m_stChunk, have);
            if (!ec)
                return ec.GetError();
        }
    }
    while (z->avail_in != 0);
    return {};
}

Result<StreamPtr> DeflateStream::Clone() const noexcept
{
    return make_error_code(errc::not_supported);
}

Result<void> DeflateStream::Finish() noexcept
{
    if (m_bFinished)
        return make_error_code(errc::invalid_argument);

    unsigned char buf[1];

    auto z = **m_pZStream;
    z->avail_in = 0;
    z->next_in = buf;

    while (true)
    {
        z->avail_out = sizeof(m_stChunk);
        z->next_out = m_stChunk;
        auto ret = ::zng_deflate(z, Z_FINISH);

        // 写出
        auto have = sizeof(m_stChunk) - z->avail_out;
        if (have)
        {
            auto ec = m_pUnderlayStream->Write(m_stChunk, have);
            if (!ec)
                return ec.GetError();
        }

        if (ret == Z_OK || ret == Z_BUF_ERROR)
            continue;

        assert(ret == Z_STREAM_END);
        break;
    }

    m_bFinished = true;
    return {};
}
