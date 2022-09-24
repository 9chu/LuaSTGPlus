/**
 * @file
 * @date 2022/2/28
 * @author 9chu
 * 此文件为 LuaSTGPlus 项目的一部分，版权与许可声明详见 COPYRIGHT.txt。
 */
#pragma once
#include <lstg/Core/Subsystem/VFS/IStream.hpp>

namespace lstg::Subsystem::VFS::detail
{
    class ZipPkDecryptStream :
        public IStream
    {
    public:
        ZipPkDecryptStream(StreamPtr underlayStream, std::string_view password, uint16_t verify);
        ZipPkDecryptStream(const ZipPkDecryptStream& org);

    public:  // IStream
        bool IsReadable() const noexcept override;
        bool IsWriteable() const noexcept override;
        bool IsSeekable() const noexcept override;
        Result<uint64_t> GetLength() const noexcept override;
        Result<void> SetLength(uint64_t length) noexcept override;
        Result<uint64_t> GetPosition() const noexcept override;
        Result<void> Seek(int64_t offset, StreamSeekOrigins origin) noexcept override;
        Result<bool> IsEof() const noexcept override;
        Result<void> Flush() noexcept override;
        Result<size_t> Read(uint8_t* buffer, size_t length) noexcept override;
        Result<void> Write(const uint8_t* buffer, size_t length) noexcept override;
        Result<StreamPtr> Clone() const noexcept override;

    private:
        void PkCryptUpdateKeys(uint8_t c) noexcept;
        uint8_t DecodeByte(uint8_t c) noexcept;

    private:
        StreamPtr m_pUnderlayStream;
        uint32_t m_uKeys[3] = {0, 0, 0};
        uint8_t m_uVerify1 = 0;
        uint8_t m_uVerify2 = 0;
        uint64_t m_uReadCount = 0;
    };
}
