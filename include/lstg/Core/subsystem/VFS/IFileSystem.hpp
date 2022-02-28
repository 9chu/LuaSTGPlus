/**
 * @file
 * @date 2022/2/19
 * @author 9chu
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#pragma once
#include "Path.hpp"
#include "IStream.hpp"
#include "../../Flag.hpp"
#include "../../Result.hpp"

namespace lstg::Subsystem::VFS
{
    /**
     * 文件类型
     */
    enum class FileType
    {
        Unknown = 0,
        RegularFile = 1,
        Directory = 2,
    };

    /**
     * 文件属性信息
     */
    struct FileAttribute
    {
        FileType Type = FileType::Unknown;
        ::time_t LastModified = 0;
        size_t Size = 0;
    };

    /**
     * 文件夹迭代访问器
     */
    class IDirectoryIterator
    {
    public:
        IDirectoryIterator() = default;
        virtual ~IDirectoryIterator() = default;

    public:
        /**
         * 获取当前名称
         * 当迭代完成时，返回空文件名。
         */
        virtual Path GetName() const noexcept = 0;

        /**
         * 遍历下一个
         */
        virtual Result<void> Next() noexcept = 0;
    };

    using DirectoryIteratorPtr = std::shared_ptr<IDirectoryIterator>;

    /**
     * 文件访问模式
     */
    enum class FileAccessMode
    {
        Read,
        Write,
        ReadWrite,
    };

    /**
     * 文件打开标志位
     */
    LSTG_FLAG_BEGIN(FileOpenFlags)
        None = 0,
        Truncate = 1,
    LSTG_FLAG_END(FileOpenFlags)

    /**
     * 文件系统接口
     *
     * 文件系统接口用于隐藏对本地文件系统、ZIP资源包、网络文件数据等的实现。
     * 文件系统本身的接口不保证线程安全，但是产生的流对象应当保证可以在各自线程存取。
     * 出于简化实现目的，所有接口均采取同步接口实现。
     *
     * 文件系统需要提供基本的增删改查接口，包括：
     *  - 创建文件/文件夹
     *  - 删除文件/文件夹
     *  - 打开文件
     *  - 获取文件/文件夹状态信息
     *  - 遍历目录
     */
    class IFileSystem
    {
    public:
        IFileSystem() = default;
        virtual ~IFileSystem() = default;

    public:
        /**
         * 创建文件夹
         * @param path 传入路径，调用方保证一定是相对路径
         * @return 错误码
         */
        virtual Result<void> CreateDirectory(Path path) noexcept = 0;

        /**
         * 删除文件或文件夹
         * 删除文件夹的操作通常要求保证文件夹内没有其他文件
         * @param path 传入路径，调用方保证一定是相对路径
         * @return 错误码
         */
        virtual Result<void> Remove(Path path) noexcept = 0;

        /**
         * 获取文件属性
         * @param path 传入路径，调用方保证一定是相对路径
         * @return 成功返回文件属性，失败返回错误码
         */
        virtual Result<FileAttribute> GetFileAttribute(Path path) noexcept = 0;

        /**
         * 遍历文件夹
         * @param path 传入路径，调用方保证一定是相对路径
         * @return 成功返回迭代器，失败返回错误码
         */
        virtual Result<DirectoryIteratorPtr> VisitDirectory(Path path) noexcept = 0;

        /**
         * 打开文件
         * @param path 传入路径，调用方保证一定是相对路径
         * @return 成功返回文件流，失败返回错误码
         */
        virtual Result<StreamPtr> OpenFile(Path path, FileAccessMode access, FileOpenFlags flags) noexcept = 0;
    };

    using FileSystemPtr = std::shared_ptr<IFileSystem>;
}
