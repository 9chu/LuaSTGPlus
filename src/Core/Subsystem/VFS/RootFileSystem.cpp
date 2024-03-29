/**
 * @file
 * @date 2022/2/20
 * @author 9chu
 * 此文件为 LuaSTGPlus 项目的一部分，版权与许可声明详见 COPYRIGHT.txt。
 */
#include <lstg/Core/Subsystem/VFS/RootFileSystem.hpp>

using namespace std;
using namespace lstg;
using namespace lstg::Subsystem::VFS;

void RootFileSystem::Mount(Path path, FileSystemPtr fs)
{
    auto* mp = &m_stRoot;

    // 从根文件系统开始找 path，匹配最长前缀
    for (size_t i = 0; i < path.GetSegmentCount(); ++i)
    {
        // 我们允许 path 中存在 '.' 但是不允许存在 '..'
        assert(path[i] != "..");
        if (path[i] == ".")
            continue;

        auto it = mp->SubNodes.find(path[i]);
        if (it == mp->SubNodes.end())
        {
            MountingPoint nmp;
            nmp.Name = path[i];
            mp->SubNodes[nmp.Name] = std::move(nmp);

            it = mp->SubNodes.find(path[i]);
            assert(it != mp->SubNodes.end());
        }
        mp = &(it->second);
    }

    // 插入末端
    mp->FileSystem = std::move(fs);
}

bool RootFileSystem::Unmount(Path path) noexcept
{
    auto* mp = &m_stRoot;

    // 从根文件系统开始找 path，匹配最长前缀
    for (size_t i = 0; i < path.GetSegmentCount(); ++i)
    {
        // 我们允许 path 中存在 '.' 但是不允许存在 '..'
        assert(path[i] != "..");
        if (path[i] == ".")
            continue;

        auto it = mp->SubNodes.find(path[i]);
        if (it == mp->SubNodes.end())
            return false;
        mp = &(it->second);
    }

    // FIXME: 中间路径没有删除
    mp->FileSystem.reset();
    return true;
}

Result<void> RootFileSystem::CreateDirectory(Path path) noexcept
{
    auto [fs, postfix] = FindMountPoint(path);

    if (!fs)
        return make_error_code(errc::no_such_device);
    return fs->CreateDirectory(postfix);
}

Result<void> RootFileSystem::Remove(Path path) noexcept
{
    auto [fs, postfix] = FindMountPoint(path);

    if (!fs)
        return make_error_code(errc::no_such_device);
    return fs->Remove(postfix);
}

Result<void> RootFileSystem::Rename(Path from, Path to) noexcept
{
    auto [fs, postfix] = FindMountPoint(from);
    auto [fs2, postfix2] = FindMountPoint(to);

    // FIXME: 跨文件系统移动，需要额外支持
    if (fs != fs2)
        return make_error_code(errc::not_supported);

    return fs->Rename(postfix, postfix2);
}

Result<FileAttribute> RootFileSystem::GetFileAttribute(Path path) noexcept
{
    auto [fs, postfix] = FindMountPoint(path);

    if (!fs)
        return make_error_code(errc::no_such_device);
    return fs->GetFileAttribute(postfix);
}

Result<DirectoryIteratorPtr> RootFileSystem::VisitDirectory(Path path) noexcept
{
    auto [fs, postfix] = FindMountPoint(path);

    if (!fs)
        return make_error_code(errc::no_such_device);
    return fs->VisitDirectory(postfix);
}

Result<StreamPtr> RootFileSystem::OpenFile(Path path, FileAccessMode access, FileOpenFlags flags) noexcept
{
    auto [fs, postfix] = FindMountPoint(path);

    if (!fs)
        return make_error_code(errc::no_such_device);
    return fs->OpenFile(postfix, access, flags);
}

std::tuple<FileSystemPtr, Path> RootFileSystem::FindMountPoint(const Path& path) const noexcept
{
    auto* mp = &m_stRoot;
    auto* longest = mp;
    size_t longestIndex = 0;

    // 从根文件系统开始找 path，匹配最长前缀
    for (size_t i = 0; i < path.GetSegmentCount(); ++i)
    {
        // 我们允许 path 中存在 '.' 但是不允许存在 '..'
        assert(path[i] != "..");
        if (path[i] == ".")
            continue;

        auto it = mp->SubNodes.find(path[i]);
        if (it == mp->SubNodes.end())
            break;

        mp = &(it->second);

        if (mp->FileSystem)
        {
            longest = mp;
            longestIndex = i;
        }
    }

    auto postfix = path.Slice(longestIndex + 1);
    if (postfix.IsEmpty())
        postfix = Path(".");
    return make_tuple(longest->FileSystem, postfix);
}

const std::string& RootFileSystem::GetUserData() const noexcept
{
    return m_stUserData;
}

void RootFileSystem::SetUserData(std::string ud) noexcept
{
    m_stUserData = std::move(ud);
}
