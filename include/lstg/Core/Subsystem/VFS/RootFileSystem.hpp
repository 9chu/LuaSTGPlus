/**
 * @file
 * @date 2022/2/20
 * @author 9chu
 * 此文件为 LuaSTGPlus 项目的一部分，版权与许可声明详见 COPYRIGHT.txt。
 */
#pragma once
#include <tuple>
#include <map>
#include <string>
#include "IFileSystem.hpp"

namespace lstg::Subsystem::VFS
{
    /**
     * 根文件系统
     * 支持将各种子文件系统挂载在根文件系统上
     */
    class RootFileSystem :
        public IFileSystem
    {
    public:
        /**
         * 挂载文件系统
         * @param path 路径
         * @param fs 文件系统
         */
        void Mount(Path path, FileSystemPtr fs);

        /**
         * 卸载文件系统
         * @param path 路径
         * @return 是否成功
         */
        bool Unmount(Path path) noexcept;

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
        struct MountingPoint
        {
            std::string Name;
            FileSystemPtr FileSystem;
            std::map<std::string, MountingPoint, std::less<>> SubNodes;
        };

        [[nodiscard]] std::tuple<FileSystemPtr, Path> FindMountPoint(const Path& path) const noexcept;

    private:
        std::string m_stUserData;
        MountingPoint m_stRoot;
    };
}
