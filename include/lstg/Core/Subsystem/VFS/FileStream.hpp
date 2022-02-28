/**
 * @file
 * @date 2022/2/20
 * @author 9chu
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#pragma once
#include <filesystem>
#include "IStream.hpp"
#include "IFileSystem.hpp"

namespace lstg::Subsystem::VFS
{
    /**
     * 文件数据流
     */
    class FileStream :
        public IStream
    {
    public:
        /**
         * 打开文件
         * @see fopen
         * @param path 路径
         * @param access 存取模式
         * @param openFlags 指示特殊的打开方式的标志位
         */
        FileStream(std::filesystem::path path, FileAccessMode access, FileOpenFlags openFlags);
        ~FileStream() override;

        FileStream(FileStream&& rhs) noexcept;
        FileStream& operator=(FileStream&& rhs) noexcept;

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
        std::filesystem::path m_stPath;
        FileAccessMode m_iAccess = FileAccessMode::Read;
        FileOpenFlags m_iFlags = FileOpenFlags::None;
        FILE* m_pHandle = nullptr;
    };
}
