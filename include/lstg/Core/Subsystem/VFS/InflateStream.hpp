/**
 * @file
 * @date 2022/2/28
 * @author 9chu
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#pragma once
#include <memory>
#include <optional>
#include "IStream.hpp"

namespace lstg::Subsystem::VFS
{
    namespace detail
    {
        class ZStream;
        using ZStreamPtr = std::shared_ptr<ZStream>;
    }

    /**
     * 解压流
     */
    class InflateStream :
        public IStream
    {
    public:
        InflateStream(StreamPtr underlayStream, std::optional<uint64_t> uncompressedSize = {});
        InflateStream(const InflateStream&) = delete;

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
         * 重置状态
         * 方法会清空内部状态，同时 Seek 到底层流开始处。
         */
        Result<void> Reset() noexcept;

    private:
        detail::ZStreamPtr m_pZStream;
        StreamPtr m_pUnderlayStream;
        bool m_bFinished = false;
        std::optional<uint64_t> m_stUncompressedSize;
        uint8_t m_stChunk[16 * 1024];  // 16k
    };
}
