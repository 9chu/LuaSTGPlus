/**
 * @file
 * @date 2022/2/19
 * @author 9chu
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#pragma once
#include "VFS/RootFileSystem.hpp"

namespace lstg::Subsystem
{
    /**
     * 虚拟文件系统
     */
    class VirtualFileSystem
    {
    public:
        VirtualFileSystem() = default;
        VirtualFileSystem(const VirtualFileSystem&) = delete;
        VirtualFileSystem(VirtualFileSystem&&)noexcept = delete;
        ~VirtualFileSystem() = default;

    public:
        /**
         * 创建文件夹
         * @param path 路径
         * @return 是否成功
         */
        Result<void> CreateDirectory(std::string_view path) noexcept;

        /**
         * 删除文件或文件夹
         * @param path 路径
         * @return 是否成功
         */
        Result<void> Remove(std::string_view path) noexcept;

        /**
         * 获取文件属性
         * @param path 路径
         * @return 是否成功
         */
        Result<VFS::FileAttribute> GetFileAttribute(std::string_view path) noexcept;

        /**
         * 遍历文件夹
         * @param path 路径
         * @return 文件夹遍历器
         */
        Result<VFS::DirectoryIteratorPtr> VisitDirectory(std::string_view path) noexcept;

        /**
         * 打开文件
         * @param path 路径
         * @param access 存取模式
         * @param flags 标记位
         * @return 流对象
         */
        Result<VFS::StreamPtr> OpenFile(std::string_view path, VFS::FileAccessMode access,
            VFS::FileOpenFlags flags = VFS::FileOpenFlags::None) noexcept;

        /**
         * 读取整个文件
         * @param out 输出
         * @param path 路径
         * @return 读取的大小
         */
        Result<size_t> ReadFile(std::vector<uint8_t>& out, std::string_view path);

        /**
         * 挂载文件系统
         * @param path 路径
         * @param fs 文件系统
         */
        Result<void> Mount(std::string_view path, VFS::FileSystemPtr fs) noexcept;

        /**
         * 卸载文件系统
         * @param path 路径
         * @return 是否成功
         */
        Result<bool> Unmount(std::string_view path) noexcept;

    private:
        VFS::RootFileSystem m_stRootFileSystem;
    };
}
