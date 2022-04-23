/**
 * @file
 * @date 2022/3/5
 * @author 9chu
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#pragma once
#include "IFileSystem.hpp"
#include "WebFileStream.hpp"

namespace lstg::Subsystem::VFS
{
    /**
     * 网络文件系统
     * 提供基于 emscripten fetch 阻塞接口的 HTTP 文件访问。
     */
    class WebFileSystem :
        public IFileSystem
    {
    public:
        /**
         * 构造文件系统
         * @param baseUrl 基准 URL
         */
        WebFileSystem(std::string_view baseUrl);

    public:
        /**
         * 获取用户名
         */
        const std::string& GetUserName() const noexcept { return m_stConfig.UserName; }

        /**
         * 设置用户名
         */
        void SetUserName(std::string value) noexcept { m_stConfig.UserName = std::move(value); }

        /**
         * 获取密码
         */
        const std::string& GetPassword() const noexcept { return m_stConfig.Password; }

        /**
         * 设置密码
         */
        void SetPassword(std::string value) noexcept { m_stConfig.Password = std::move(value); }

        /**
         * 获取读请求的超时时间（秒）
         */
        uint32_t GetReadRequestTimeout() const noexcept { return m_stConfig.ReadRequestTimeout; }

        /**
         * 设置读请求的超时时间（秒）
         */
        void SetReadRequestTimeout(uint32_t value) noexcept { m_stConfig.ReadRequestTimeout = value; }

        /**
         * 获取 HEAD 请求的超时时间（秒）
         */
        uint32_t GetHeadRequestTimeout() const noexcept { return m_uHeadRequestTimeout; }

        /**
         * 设置 HEAD 请求的超时时间（秒）
         */
        void SetHeadRequestTimeout(uint32_t value) noexcept { m_uHeadRequestTimeout = value; }

    public:  // IFileSystem
        Result<void> CreateDirectory(Path path) noexcept override;
        Result<void> Remove(Path path) noexcept override;
        Result<FileAttribute> GetFileAttribute(Path path) noexcept override;
        Result<DirectoryIteratorPtr> VisitDirectory(Path path) noexcept override;
        Result<StreamPtr> OpenFile(Path path, FileAccessMode access, FileOpenFlags flags) noexcept override;
        const std::string& GetUserData() const noexcept override;
        void SetUserData(std::string ud) noexcept override;

    private:
        std::string m_stUserData;
        std::string m_stBaseUrl;
        WebFileStream::FetchConfig m_stConfig;
        uint32_t m_uHeadRequestTimeout = 5 * 1000;
    };
}
