/**
 * @file
 * @date 2022/2/22
 * @author 9chu
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#include <lstg/Core/Subsystem/VFS/ZipArchiveFileSystem.hpp>

#include <queue>
#include "detail/ZipFile.hpp"
#include "detail/ZipFileReadError.hpp"

using namespace std;
using namespace lstg;
using namespace lstg::Subsystem::VFS;
using namespace lstg::Subsystem::VFS::detail;

namespace
{
    class ZipArchiveFileSystemDirectoryIterator :
        public IDirectoryIterator
    {
    public:
        ZipArchiveFileSystemDirectoryIterator(const ZipDirectoryEntry* entry)
            : m_pEntry(entry), m_stDirectoryIterator(entry->Directories.begin()), m_stFileIterator(entry->Files.begin())
        {
            assert(m_pEntry);
            MoveNext();
        }

    public:
        Path GetName() const noexcept override
        {
            return m_stCurrentName;
        }

        Result<void> Next() noexcept override
        {
            try
            {
                if (!MoveNext())
                    return make_error_code(errc::result_out_of_range);
                return {};
            }
            catch (...)  // bad_alloc
            {
                return make_error_code(errc::not_enough_memory);
            }
        }

    private:
        bool MoveNext()
        {
            if (m_stDirectoryIterator != m_pEntry->Directories.end())
            {
                m_stCurrentName = Path{m_stDirectoryIterator->first};
                ++m_stDirectoryIterator;
                return true;
            }
            else if (m_stFileIterator != m_pEntry->Files.end())
            {
                m_stCurrentName = Path{m_stFileIterator->first};
                ++m_stFileIterator;
                return true;
            }
            return false;
        }

    private:
        const ZipDirectoryEntry* m_pEntry = nullptr;
        Path m_stCurrentName;
        map<std::string, ZipDirectoryEntry*>::const_iterator m_stDirectoryIterator;
        map<std::string, ZipFileEntry>::const_iterator m_stFileIterator;
    };
}

ZipArchiveFileSystem::ZipArchiveFileSystem(StreamPtr underlayStream, std::string password)
    : m_stPassword(std::move(password))
{
    // 构造 ZipFile 对象
    m_pZipFile = new ZipFile(std::move(underlayStream));

    // 读取所有条目
    detail::ZipFileEntryContainer entries;
    auto ret = m_pZipFile->ReadFileEntries(entries);
    ret.ThrowIfError();

    // 构造文件树
    try
    {
        for (auto& entry : entries)
        {
            Path p {entry.FileName};
            auto dir = CreateTree(p);
            assert(dir);

            // 检查文件是否存在
            auto filename = p.GetFileName().ToStringView();
            auto it = dir->Files.find(filename);
            if (it != dir->Files.end())
                throw system_error(detail::ZipFileReadError::DuplicatedFile);
            dir->Files.emplace(filename, std::move(entry));
        }
    }
    catch (...)
    {
        // 手工回收资源
        ClearFileTree();
        delete m_pZipFile;
        throw;
    }
}

ZipArchiveFileSystem::~ZipArchiveFileSystem()
{
    // 释放所有 Entry
    ClearFileTree();

    // 析构 ZipFile
    delete m_pZipFile;
    m_pZipFile = nullptr;
}

Result<void> ZipArchiveFileSystem::CreateDirectory(Path path) noexcept
{
    return make_error_code(errc::not_supported);
}

Result<void> ZipArchiveFileSystem::Remove(Path path) noexcept
{
    return make_error_code(errc::not_supported);
}

Result<FileAttribute> ZipArchiveFileSystem::GetFileAttribute(Path path) noexcept
{
    auto entry = LocatePath(path);

    if (entry.index() == 0)
    {
        auto dir = get<0>(entry);
        if (!dir)
            return make_error_code(errc::no_such_file_or_directory);

        FileAttribute ret;
        ret.Type = FileType::Directory;
        ret.LastModified = 0;
        ret.Size = 0;
        return ret;
    }
    else
    {
        assert(entry.index() == 1);
        auto file = get<1>(entry);
        if (!file)
            return make_error_code(errc::no_such_file_or_directory);

        FileAttribute ret;
        ret.Type = FileType::RegularFile;
        ret.LastModified = file->LastModified;
        ret.Size = file->UncompressedSize;
        return ret;
    }
}

Result<DirectoryIteratorPtr> ZipArchiveFileSystem::VisitDirectory(Path path) noexcept
{
    try
    {
        auto entry = LocatePath(path);
        if (entry.index() == 0)
        {
            auto dir = get<0>(entry);
            if (!dir)
                return make_error_code(errc::no_such_file_or_directory);
            return make_shared<ZipArchiveFileSystemDirectoryIterator>(dir);
        }
        else
        {
            assert(entry.index() == 1);
            return make_error_code(errc::not_a_directory);
        }
    }
    catch (...)  // std::bad_alloc
    {
        return make_error_code(errc::not_enough_memory);
    }
}

Result<StreamPtr> ZipArchiveFileSystem::OpenFile(Path path, FileAccessMode access, FileOpenFlags flags) noexcept
{
    if (access == FileAccessMode::ReadWrite || access == FileAccessMode::Write)
        return make_error_code(errc::permission_denied);
    if (flags & FileOpenFlags::Truncate)
        return make_error_code(errc::invalid_argument);

    auto entry = LocatePath(path);
    if (entry.index() == 0)
    {
        auto dir = get<0>(entry);
        if (!dir)
            return make_error_code(errc::no_such_file_or_directory);
        return make_error_code(errc::invalid_argument);
    }
    else
    {
        assert(entry.index() == 1);
        auto file = get<1>(entry);

        // 构造流
        // FIXME: 这种接口会造成只能使用单一密码
        return m_pZipFile->OpenEntry(*file, m_stPassword);
    }
}

ConstZipEntry ZipArchiveFileSystem::LocatePath(const Path& path) const noexcept
{
    // 空路径直接返回
    if (path.IsEmpty())
        return static_cast<ZipDirectoryEntry*>(nullptr);

    // 访问所有的文件夹节点
    const ZipDirectoryEntry* entry = &m_stRoot;
    for (size_t i = 0; i + 1 < path.GetSegmentCount(); ++i)
    {
        // 处理 '.' 的情况
        auto dir = path.GetSegment(i);
        assert(dir != "..");
        if (dir == ".")
            continue;

        auto it = entry->Directories.find(path.GetSegment(i));
        if (it == entry->Directories.end())
            return static_cast<ZipDirectoryEntry*>(nullptr);
        entry = it->second;
    }

    // 获取文件名
    auto filename = path.GetSegment(path.GetSegmentCount() - 1);

    // 如果文件名是'.'，直接返回 entry
    if (filename == ".")
        return entry;

    // 查找下是否是文件夹
    {
        auto it = entry->Directories.find(filename);
        if (it != entry->Directories.end())
            return it->second;
    }

    // 查找下是否是文件
    {
        auto it = entry->Files.find(filename);
        if (it != entry->Files.end())
            return &(it->second);
    }
    return static_cast<ZipDirectoryEntry*>(nullptr);
}

void ZipArchiveFileSystem::ClearFileTree()
{
    queue<ZipDirectoryEntry*> clearQueue;
    clearQueue.push(&m_stRoot);

    while (!clearQueue.empty())
    {
        ZipDirectoryEntry* p = clearQueue.front();
        assert(p);
        clearQueue.pop();

        for (auto& it : p->Directories)
            clearQueue.push(it.second);

        if (p != &m_stRoot)
            delete p;
    }
}

ZipDirectoryEntry* ZipArchiveFileSystem::CreateTree(const Path& path)
{
    ZipDirectoryEntry* current = &m_stRoot;

    // 扫描父节点
    for (size_t i = 0; i + 1 < path.GetSegmentCount(); ++i)
    {
        auto seg = path[i];
        auto it = current->Directories.find(seg);
        if (it == current->Directories.end())
        {
            auto ret = current->Directories.emplace(seg, new ZipDirectoryEntry());
            it = ret.first;

            it->second->Name = seg;
        }

        current = it->second;
    }
    return current;
}
