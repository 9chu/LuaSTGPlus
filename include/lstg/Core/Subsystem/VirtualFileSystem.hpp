/**
 * @file
 * @date 2022/2/19
 * @author 9chu
 * 此文件为 LuaSTGPlus 项目的一部分，版权与许可声明详见 COPYRIGHT.txt。
 */
#pragma once
#include "ISubsystem.hpp"
#include "VFS/RootFileSystem.hpp"

namespace lstg::Subsystem
{
    /**
     * 虚拟文件系统
     */
    class VirtualFileSystem :
        public ISubsystem
    {
    public:
        VirtualFileSystem(SubsystemContainer& container);
        VirtualFileSystem(const VirtualFileSystem&) = delete;
        VirtualFileSystem(VirtualFileSystem&&)noexcept = delete;
        ~VirtualFileSystem() = default;

    public:
        /**
         * 获取资源基准目录
         * @note VFS 接口总是基于 '/' 进行访问的，此接口供组件使用来定位素材路径
         */
        [[nodiscard]] const std::string& GetAssetBaseDirectory() const noexcept { return m_stAssetBaseDirectory; }

        /**
         * 设置资源基准目录
         * @note VFS 接口总是基于 '/' 进行访问的，此接口供组件使用来定位素材路径
         * @param path 路径
         */
        void SetAssetBaseDirectory(std::string_view path) { m_stAssetBaseDirectory = path; }

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
         * 重命名文件或文件夹
         * @param from 原始文件名
         * @param to 目的文件名
         * @return 是否成功
         */
        Result<void> Rename(std::string_view from, std::string_view to) noexcept;

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
        std::string m_stAssetBaseDirectory;
    };
}
