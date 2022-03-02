/**
 * @file
 * @date 2022/3/1
 * @author 9chu
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#include <lstg/Core/Subsystem/VirtualFileSystem.hpp>

using namespace std;
using namespace lstg;
using namespace lstg::Subsystem;

namespace
{
    /**
     * 规格化路径
     * 去掉 '.' 和 '..' 项目。
     * @param path 路径
     * @return 返回规格化后的结果
     */
    Result<VFS::Path> NormalizePath(std::string_view path) noexcept
    {
        try
        {
            VFS::Path temp(path);

            // 过滤 '.' 和 '..'
            vector<string> normalized;
            normalized.reserve(temp.GetSegmentCount());
            for (size_t i = 0; i < temp.GetSegmentCount(); ++i)
            {
                auto segment = temp[i];
                assert(!segment.empty());

                if (segment == ".")
                    continue;
                if (segment == "..")
                {
                    if (!normalized.empty())
                        normalized.pop_back();
                    continue;
                }
                normalized.emplace_back(segment);
            }

            // 合并路径
            string join;
            join.reserve(path.length());
            for (size_t i = 0; i < normalized.size(); ++i)
            {
                if (!join.empty())
                    join.append("/");
                join.append(normalized[i]);
            }

            return VFS::Path {join};
        }
        catch (...)  // bad_alloc
        {
            return make_error_code(errc::not_enough_memory);
        }
    }
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
