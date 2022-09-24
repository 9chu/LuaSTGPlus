/**
 * @file
 * @date 2022/3/1
 * @author 9chu
 * 此文件为 LuaSTGPlus 项目的一部分，版权与许可声明详见 COPYRIGHT.txt。
 */
#include <lstg/Core/Subsystem/VirtualFileSystem.hpp>

using namespace std;
using namespace lstg;
using namespace lstg::Subsystem;

namespace
{
    Result<VFS::Path> NormalizePath(std::string_view path) noexcept
    {
        try
        {
            return VFS::Path::Normalize(path);
        }
        catch (...)  // bad_alloc
        {
            return make_error_code(errc::not_enough_memory);
        }
    }
}

VirtualFileSystem::VirtualFileSystem(SubsystemContainer& container)
{
    static_cast<void>(container);
}

Result<void> VirtualFileSystem::CreateDirectory(std::string_view path) noexcept
{
    auto npath = NormalizePath(path);
    if (!npath)
        return npath.GetError();
    return m_stRootFileSystem.CreateDirectory(*npath);
}

Result<void> VirtualFileSystem::Remove(std::string_view path) noexcept
{
    auto npath = NormalizePath(path);
    if (!npath)
        return npath.GetError();
    return m_stRootFileSystem.Remove(*npath);
}

Result<VFS::FileAttribute> VirtualFileSystem::GetFileAttribute(std::string_view path) noexcept
{
    auto npath = NormalizePath(path);
    if (!npath)
        return npath.GetError();
    return m_stRootFileSystem.GetFileAttribute(*npath);
}

Result<VFS::DirectoryIteratorPtr> VirtualFileSystem::VisitDirectory(std::string_view path) noexcept
{
    auto npath = NormalizePath(path);
    if (!npath)
        return npath.GetError();
    return m_stRootFileSystem.VisitDirectory(*npath);
}

Result<VFS::StreamPtr> VirtualFileSystem::OpenFile(std::string_view path, VFS::FileAccessMode access, VFS::FileOpenFlags flags) noexcept
{
    auto npath = NormalizePath(path);
    if (!npath)
        return npath.GetError();
    return m_stRootFileSystem.OpenFile(*npath, access, flags);
}

Result<size_t> VirtualFileSystem::ReadFile(std::vector<uint8_t>& out, std::string_view path)
{
    out.clear();

    // 打开文件
    auto stream = OpenFile(path, VFS::FileAccessMode::Read, VFS::FileOpenFlags::None);
    if (!stream)
        return stream.GetError();

    // 发起读操作
    static const unsigned kExpandSize = 16 * 1024;  // 16k

    size_t readSize = 0;
    try
    {
        while (true)
        {
            out.resize(readSize + kExpandSize);

            auto input = (*stream)->Read(out.data() + readSize, kExpandSize);
            if (!input)
            {
                out.clear();
                return input.GetError();
            }
            assert(*input <= kExpandSize);
            readSize += *input;

            if (*input < kExpandSize)
            {
                out.resize(readSize);
                break;
            }
        }
    }
    catch (...)  // bad_alloc
    {
        out.clear();
        return make_error_code(errc::not_enough_memory);
    }
    return readSize;
}

Result<void> VirtualFileSystem::Mount(std::string_view path, VFS::FileSystemPtr fs) noexcept
{
    auto npath = NormalizePath(path);
    if (!npath)
        return npath.GetError();
    try
    {
        m_stRootFileSystem.Mount(*npath, std::move(fs));
    }
    catch (...)  // bad_alloc
    {
        return make_error_code(errc::not_enough_memory);
    }
    return {};
}

Result<bool> VirtualFileSystem::Unmount(std::string_view path) noexcept
{
    auto npath = NormalizePath(path);
    if (!npath)
        return npath.GetError();
    return m_stRootFileSystem.Unmount(*npath);
}
