/**
 * @file
 * @date 2022/2/20
 * @author 9chu
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#pragma once
#include <filesystem>
#include "IFileSystem.hpp"

namespace lstg::Subsystem::VFS
{
    /**
     * 本地文件系统
     */
    class LocalFileSystem :
        public IFileSystem
    {
    public:
        /**
         * 构造文件系统
         * @param root 根路径
         */
        LocalFileSystem(std::filesystem::path root) noexcept;

    public:  // IFileSystem
        Result<void> CreateDirectory(Path path) noexcept override;
        Result<void> Remove(Path path) noexcept override;
        Result<FileAttribute> GetFileAttribute(Path path) noexcept override;
        Result<DirectoryIteratorPtr> VisitDirectory(Path path) noexcept override;
        Result<StreamPtr> OpenFile(Path path, FileAccessMode access, FileOpenFlags flags) noexcept override;

    private:
        std::filesystem::path MakeLocalPath(const Path& path) const;

    private:
        std::filesystem::path m_stRoot;
    };
}