/**
 * @file
 * @date 2022/2/28
 * @author 9chu
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#include "ZipPkDecryptStream.hpp"

#include <zlib-ng.h>
#include "ZipFileReadError.hpp"

using namespace std;
using namespace lstg;
using namespace lstg::Subsystem::VFS::detail;

using StreamPtr = lstg::Subsystem::VFS::StreamPtr;

static const unsigned kPkCryptHeaderSize = 12;

ZipPkDecryptStream::ZipPkDecryptStream(StreamPtr underlayStream, std::string_view password, uint16_t verify)
    : m_pUnderlayStream(std::move(underlayStream))
{
    assert(m_pUnderlayStream);

    uint8_t verify1 = static_cast<uint8_t>((verify >> 8u) & 0xFFu);
    uint8_t verify2 = static_cast<uint8_t>(verify & 0xFFu);

    // 初始化密钥
    m_uKeys[0] = 305419896u;
    m_uKeys[1] = 591751049u;
    m_uKeys[2] = 878082192u;
    for (char ch : password)
        PkCryptUpdateKeys(static_cast<uint8_t>(ch));

    // 校验头
    uint8_t header[kPkCryptHeaderSize];
    auto ret = m_pUnderlayStream->Read(header, sizeof(header));
    ret.ThrowIfError();
    if (*ret != sizeof(header))
        throw system_error(ZipFileReadError::UnexpectedEndOfStream);

    size_t i = 0;
    for (; i < kPkCryptHeaderSize - 2; ++i)
        header[i] = DecodeByte(header[i]);
    auto calcVerify1 = DecodeByte(header[i++]);
    auto calcVerify2 = DecodeByte(header[i++]);

    // 新版本只用一个字节进行校验
    static_cast<void>(verify1);
    static_cast<void>(calcVerify1);
    if ((calcVerify2 != 0) && (calcVerify2 != verify2))
        throw system_error(ZipFileReadError::BadPassword);

    m_uReadCount += kPkCryptHeaderSize;
}

ZipPkDecryptStream::ZipPkDecryptStream(const ZipPkDecryptStream& org)
{
    auto stream = org.m_pUnderlayStream->Clone();
    stream.ThrowIfError();

    m_pUnderlayStream = std::move(*stream);
    ::memcpy(m_uKeys, org.m_uKeys, sizeof(m_uKeys));
    m_uVerify1 = org.m_uVerify1;
    m_uVerify2 = org.m_uVerify2;
    m_uReadCount = org.m_uReadCount;
}

bool ZipPkDecryptStream::IsReadable() const noexcept
{
    return true;
}

bool ZipPkDecryptStream::IsWriteable() const noexcept
{
    return false;
}

bool ZipPkDecryptStream::IsSeekable() const noexcept
{
    return false;
}

Result<uint64_t> ZipPkDecryptStream::GetLength() const noexcept
{
    auto ret = m_pUnderlayStream->GetLength();
    if (!ret)
        return ret;
    auto len = *ret;
    if (len < kPkCryptHeaderSize)
        return static_cast<uint64_t>(0);
    return len - kPkCryptHeaderSize;
}

Result<void> ZipPkDecryptStream::SetLength(uint64_t length) noexcept
{
    return make_error_code(errc::not_supported);
}

Result<uint64_t> ZipPkDecryptStream::GetPosition() const noexcept
{
    return m_uReadCount;
}

Result<void> ZipPkDecryptStream::Seek(int64_t offset, StreamSeekOrigins origin) noexcept
{
    return make_error_code(errc::not_supported);
}

Result<bool> ZipPkDecryptStream::IsEof() const noexcept
{
    return m_pUnderlayStream->IsEof();
}

Result<void> ZipPkDecryptStream::Flush() noexcept
{
    return m_pUnderlayStream->Flush();
}

Result<size_t> ZipPkDecryptStream::Read(uint8_t* buffer, size_t length) noexcept
{
    auto ret = m_pUnderlayStream->Read(buffer, length);
    if (ret)
    {
        length = *ret;
        for (size_t i = 0; i < length; ++i)
            buffer[i] = DecodeByte(buffer[i]);
    }
    return ret;
}

Result<void> ZipPkDecryptStream::Write(const uint8_t* buffer, size_t length) noexcept
{
    return make_error_code(errc::not_supported);
}

Result<StreamPtr> ZipPkDecryptStream::Clone() const noexcept
{
    try
    {
        return make_shared<ZipPkDecryptStream>(*this);
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

void ZipPkDecryptStream::PkCryptUpdateKeys(uint8_t c) noexcept
{
    uint8_t buf = c;
    m_uKeys[0] = static_cast<uint32_t>(~::zng_crc32(static_cast<uint32_t>(~m_uKeys[0]), &buf, 1));

    m_uKeys[1] += m_uKeys[0] & 0xFF;
    m_uKeys[1] *= 134775813u;
    m_uKeys[1] += 1;

    buf = static_cast<uint8_t>(m_uKeys[1] >> 24u);
    m_uKeys[2] = static_cast<uint32_t>(~::zng_crc32(static_cast<uint32_t>(~m_uKeys[2]), &buf, 1));
}

uint8_t ZipPkDecryptStream::DecodeByte(uint8_t c) noexcept
{
    auto t = m_uKeys[2] | 2;
    auto d = static_cast<uint8_t>(((t * (t ^ 1)) >> 8) & 0xFF);
    c ^= d;
    PkCryptUpdateKeys(c);
    return c;
}
