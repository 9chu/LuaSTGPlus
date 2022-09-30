/**
 * @file
 * @date 2022/2/20
 * @author 9chu
 * 此文件为 LuaSTGPlus 项目的一部分，版权与许可声明详见 COPYRIGHT.txt。
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
        Result<void> Rename(Path from, Path to) noexcept override;
        Result<FileAttribute> GetFileAttribute(Path path) noexcept override;
        Result<DirectoryIteratorPtr> VisitDirectory(Path path) noexcept override;
        Result<StreamPtr> OpenFile(Path path, FileAccessMode access, FileOpenFlags flags) noexcept override;
        const std::string& GetUserData() const noexcept override;
        void SetUserData(std::string ud) noexcept override;

    private:
        std::filesystem::path MakeLocalPath(const Path& path) const;

    private:
        std::string m_stUserData;
        std::filesystem::path m_stRoot;
    };
}
