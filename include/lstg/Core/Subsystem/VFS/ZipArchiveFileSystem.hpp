/**
 * @file
 * @date 2022/2/22
 * @author 9chu
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#pragma once
#include <map>
#include <variant>
#include "IFileSystem.hpp"

namespace lstg::Subsystem::VFS
{
    namespace detail
    {
        struct ZipFileEntry;
        class ZipFile;

        struct ZipDirectoryEntry
        {
            std::string Name;
            std::map<std::string, ZipDirectoryEntry*, std::less<>> Directories;
            std::map<std::string, ZipFileEntry, std::less<>> Files;
        };

        using ConstZipEntry = std::variant<const ZipDirectoryEntry*, const ZipFileEntry*>;
    }

    /**
     * ZIP 档案文件系统
     * 仅支持从 ZIP 文档中读取文件。
     */
    class ZipArchiveFileSystem :
        public IFileSystem
    {
    public:
        ZipArchiveFileSystem(StreamPtr underlayStream, std::string password);
        ZipArchiveFileSystem(const ZipArchiveFileSystem&) = delete;
        ~ZipArchiveFileSystem() override;

    public:  // IFileSystem
        Result<void> CreateDirectory(Path path) noexcept override;
        Result<void> Remove(Path path) noexcept override;
        Result<FileAttribute> GetFileAttribute(Path path) noexcept override;
        Result<DirectoryIteratorPtr> VisitDirectory(Path path) noexcept override;
        Result<StreamPtr> OpenFile(Path path, FileAccessMode access, FileOpenFlags flags) noexcept override;
        const std::string& GetUserData() const noexcept override;
        void SetUserData(std::string ud) noexcept override;

    private:
        [[nodiscard]] detail::ConstZipEntry LocatePath(const Path& path) const noexcept;
        void ClearFileTree();
        detail::ZipDirectoryEntry* CreateTree(const Path& path);

    private:
        std::string m_stUserData;
        detail::ZipFile* m_pZipFile = nullptr;
        detail::ZipDirectoryEntry m_stRoot;
        std::string m_stPassword;
    };
}
