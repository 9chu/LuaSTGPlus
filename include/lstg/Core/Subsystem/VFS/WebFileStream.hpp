/**
 * @file
 * @date 2022/3/5
 * @author 9chu
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#pragma once
#include <lstg/Core/Subsystem/VFS/IStream.hpp>

namespace lstg::Subsystem::VFS
{
    /**
     * WEB 文件流
     * 提供基于 emscripten fetch 阻塞接口的 HTTP 文件访问。
     */
    class WebFileStream :
        public IStream
    {
    public:
        struct FetchConfig
        {
            std::string UserName;
            std::string Password;
            uint32_t ReadRequestTimeout = 30 * 1000;
        };

    public:
        WebFileStream(std::string url, FetchConfig config, uint64_t contentLength);

    public: // IStream
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
        const std::string m_stUrl;
        const FetchConfig m_stConfig;
        uint64_t m_ullContentLength = 0;
        uint64_t m_ullFakeReadPosition = 0;
    };
}
