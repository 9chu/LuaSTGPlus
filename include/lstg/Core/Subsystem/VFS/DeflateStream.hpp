/**
 * @file
 * @date 2022/2/28
 * @author 9chu
 * 此文件为 LuaSTGPlus 项目的一部分，版权与许可声明详见 COPYRIGHT.txt。
 */
#pragma once
#include "InflateStream.hpp"

namespace lstg::Subsystem::VFS
{
    /**
     * 压缩流
     */
    class DeflateStream :
        public IStream
    {
    public:
        DeflateStream(StreamPtr underlayStream);
        DeflateStream(StreamPtr underlayStream, int compressionLevel);
        DeflateStream(const DeflateStream& org);
        ~DeflateStream() override;

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

    public:
        /**
         * 获取底层数据流
         */
        [[nodiscard]] const StreamPtr& GetUnderlayStream() const noexcept { return m_pUnderlayStream; }

        /**
         * 通知压缩结束
         * 用于指示写入结束。
         * 若用户不主动调用则在析构时调用。
         */
        Result<void> Finish() noexcept;

    private:
        detail::ZStreamPtr m_pZStream;
        StreamPtr m_pUnderlayStream;
        bool m_bFinished = false;
        uint8_t m_stChunk[4 * 1024];  // 4k
    };
}
