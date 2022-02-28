/**
 * @file
 * @date 2022/2/20
 * @author 9chu
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#include <lstg/Core/Subsystem/VFS/LocalFileSystem.hpp>

#include <lstg/Core/Subsystem/VFS/FileStream.hpp>

using namespace std;
using namespace lstg;
using namespace lstg::Subsystem::VFS;

namespace
{
    class LocalFileSystemDirectoryIterator :
        public IDirectoryIterator
    {
    public:
        LocalFileSystemDirectoryIterator(filesystem::directory_iterator it)
            : m_stIterator(std::move(it))
        {
            if (end(m_stIterator) != m_stIterator)
                m_stCurrentFileName = Path{m_stIterator->path().stem().string()};
        }

    public:
        Path GetName() const noexcept override
        {
            return m_stCurrentFileName;
        }

        Result<void> Next() noexcept override
        {
            try
            {
                if (end(m_stIterator) == m_stIterator)
                    return make_error_code(errc::result_out_of_range);

                ++m_stIterator;

                if (end(m_stIterator) != m_stIterator)
                    m_stCurrentFileName = Path{m_stIterator->path().stem().string()};
                else
                    m_stCurrentFileName = {};
            }
            catch (const system_error& ex)
            {
                return ex.code();
            }
            catch (const bad_alloc& ex)
            {
                return make_error_code(errc::not_enough_memory);
            }
            catch (...)
            {
                return make_error_code(errc::io_error);
            }
            return {};
        }

    private:
        Path m_stCurrentFileName;
        filesystem::directory_iterator m_stIterator;
    };
}

LocalFileSystem::LocalFileSystem(std::filesystem::path root) noexcept
    : m_stRoot(std::move(root))
{
}

Result<void> LocalFileSystem::CreateDirectory(Path path) noexcept
{
    try
    {
        auto target = MakeLocalPath(path);
        filesystem::create_directory(target);
    }
    catch (const system_error& ex)
    {
        return ex.code();
    }
    catch (const bad_alloc& ex)
    {
        return make_error_code(errc::not_enough_memory);
    }
    catch (...)
    {
        return make_error_code(errc::io_error);
    }
    return {};
}

Result<void> LocalFileSystem::Remove(Path path) noexcept
{
    try
    {
        auto target = MakeLocalPath(path);
        filesystem::remove(target);
    }
    catch (const system_error& ex)
    {
        return ex.code();
    }
    catch (const bad_alloc& ex)
    {
        return make_error_code(errc::not_enough_memory);
    }
    catch (...)
    {
        return make_error_code(errc::io_error);
    }
    return {};
}

Result<FileAttribute> LocalFileSystem::GetFileAttribute(Path path) noexcept
{
    try
    {
        FileAttribute ret;

        auto target = MakeLocalPath(path);
        auto stat = filesystem::status(target);
        auto mod = filesystem::last_write_time(target);
        auto size = filesystem::file_size(target);

        if (is_regular_file(stat))
            ret.Type = FileType::RegularFile;
        else if (is_directory(stat))
            ret.Type = FileType::Directory;
        ret.LastModified = filesystem::file_time_type::clock::to_time_t(mod);
        ret.Size = size;
        return ret;
    }
    catch (const system_error& ex)
    {
        return ex.code();
    }
    catch (const bad_alloc& ex)
    {
        return make_error_code(errc::not_enough_memory);
    }
    catch (...)
    {
        return make_error_code(errc::io_error);
    }
}

Result<DirectoryIteratorPtr> LocalFileSystem::VisitDirectory(Path path) noexcept
{
    try
    {
        auto target = MakeLocalPath(path);
        auto it = filesystem::directory_iterator(target);
        auto ret = make_shared<LocalFileSystemDirectoryIterator>(it);
        return ret;
    }
    catch (const system_error& ex)
    {
        return ex.code();
    }
    catch (const bad_alloc& ex)
    {
        return make_error_code(errc::not_enough_memory);
    }
    catch (...)
    {
        return make_error_code(errc::io_error);
    }
}

Result<StreamPtr> LocalFileSystem::OpenFile(Path path, FileAccessMode access, FileOpenFlags flags) noexcept
{
    try
    {
        auto target = MakeLocalPath(path);
        return std::make_shared<FileStream>(target, access, flags);
    }
    catch (const system_error& ex)
    {
        return ex.code();
    }
    catch (const bad_alloc& ex)
    {
        return make_error_code(errc::not_enough_memory);
    }
    catch (...)
    {
        return make_error_code(errc::io_error);
    }
}

std::filesystem::path LocalFileSystem::MakeLocalPath(const Path& path) const
{
    return m_stRoot / filesystem::u8path(path.ToStringView());
}
