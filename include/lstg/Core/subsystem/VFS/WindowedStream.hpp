/**
 * @file
 * @date 2022/2/28
 * @author 9chu
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#pragma once
#include "IStream.hpp"

namespace lstg::Subsystem::VFS
{
    /**
     * Stream 窗口
     * 在父 Stream 限定一个区域进行 I/O 操作。
     * 需要注意，操作在父 Stream 进行时会影响读写位置。
     */
    class WindowedStream :
        public IStream
    {
    public:
        /**
         * 从当前位置开始，建立长度不超过 maxLength 的子数据流。
         * @param underlay 底层数据流
         * @param maxLength 最大长度，需要保证 GetPosition() + maxLength <= GetLength()
         */
        WindowedStream(StreamPtr underlay, uint64_t maxLength) noexcept;
        virtual ~WindowedStream() noexcept = default;

    public:  // IStream
        bool IsReadable() const noexcept override;
        bool IsWriteable() const noexcept override;
        bool IsSeekable() const noexcept override;
        Result<uint64_t> GetLength() const noexcept override;
        Result<void> SetLength(uint64_t length) noexcept override;
        Result<uint64_t> GetPosition() const noexcept override;
        Result<uint64_t> Seek(int64_t offset, StreamSeekOrigins origin) noexcept override;
        Result<bool> IsEof() const noexcept override;
        Result<void> Flush() noexcept override;
        Result<size_t> Read(uint8_t* buffer, size_t length) noexcept override;
        Result<void> Write(const uint8_t* buffer, size_t length) noexcept override;
        Result<StreamPtr> Clone() const noexcept override;

    private:
        StreamPtr m_pUnderlay;
        uint64_t m_ullMaxLength = 0;
        uint64_t m_ullLength = 0;
        uint64_t m_ullPosition = 0;
    };
}
