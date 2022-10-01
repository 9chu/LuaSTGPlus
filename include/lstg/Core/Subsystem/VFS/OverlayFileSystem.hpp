/**
 * @file
 * @date 2022/2/20
 * @author 9chu
 * 此文件为 LuaSTGPlus 项目的一部分，版权与许可声明详见 COPYRIGHT.txt。
 */
#pragma once
#include <vector>
#include "IFileSystem.hpp"

namespace lstg::Subsystem::VFS
{
    /**
     * 多层文件系统
     * 实现在多个文件系统中对文件进行操作，最先加入的文件系统会在最后参与查找（栈）。
     */
    class OverlayFileSystem :
        public IFileSystem
    {
    public:
        /**
         * 推入文件系统
         * @param fs 文件系统指针
         */
        void PushFileSystem(FileSystemPtr fs);

        /**
         * 弹出文件系统
         */
        bool PopFileSystem() noexcept;

        /**
         * 删除文件系统
         * @param fs 文件系统指针
         * @return 找到并删除文件系统返回 true，否则返回 false
         */
        bool RemoveFileSystem(const FileSystemPtr& fs) noexcept;

        /**
         * 删除文件系统
         * @param index 文件系统索引
         */
        void RemoveFileSystemAt(size_t index) noexcept;

        /**
         * 获取总数
         */
        size_t GetFileSystemCount() const noexcept;

        /**
         * 获取文件系统
         * @param index 文件系统索引
         */
        FileSystemPtr GetFileSystem(size_t index) const noexcept;

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
        std::string m_stUserData;
        std::vector<FileSystemPtr> m_stFileSystems;
    };
}
