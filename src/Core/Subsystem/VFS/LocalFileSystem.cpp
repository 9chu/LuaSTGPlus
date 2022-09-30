/**
 * @file
 * @date 2022/2/20
 * @author 9chu
 * 此文件为 LuaSTGPlus 项目的一部分，版权与许可声明详见 COPYRIGHT.txt。
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
                    m_stCurrentFileName = Path{m_stIterator->path().filename().string()};
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

    template <class TClock, class = void>
    struct FileTimeConvertToTimeT
    {
        time_t operator()(typename TClock::time_point tp) noexcept
        {
            // http://stackoverflow.com/questions/61030383/how-to-convert-stdfilesystemfile-time-type-to-time-t
            auto sctp = chrono::time_point_cast<chrono::system_clock::duration>(tp - TClock::now() + chrono::system_clock::now());
            return chrono::system_clock::to_time_t(sctp);
        }
    };

    template <class TClock>
    struct FileTimeConvertToTimeT<TClock, std::void_t<decltype(TClock::to_time_t(std::declval<TClock::time_point>()))>>
    {
        time_t operator()(typename TClock::time_point tp) noexcept
        {
            // to_time_t is not portable until C++20
            // see: http://en.cppreference.com/w/cpp/filesystem/file_time_type
            return TClock::to_time_t(tp);
        }
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

Result<void> LocalFileSystem::Rename(Path from, Path to) noexcept
{
    try
    {
        auto fromPath = MakeLocalPath(from);
        auto toPath = MakeLocalPath(to);
        filesystem::rename(fromPath, toPath);
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
        uintmax_t size = 0;

        if (is_regular_file(stat))
        {
            ret.Type = FileType::RegularFile;
            size = filesystem::file_size(target);
        }
        else if (is_directory(stat))
        {
            ret.Type = FileType::Directory;
        }
        ret.LastModified = FileTimeConvertToTimeT<filesystem::file_time_type::clock>{}(mod);
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

const std::string& LocalFileSystem::GetUserData() const noexcept
{
    return m_stUserData;
}

void LocalFileSystem::SetUserData(std::string ud) noexcept
{
    m_stUserData = std::move(ud);
}

std::filesystem::path LocalFileSystem::MakeLocalPath(const Path& path) const
{
    return m_stRoot / filesystem::u8path(path.ToStringView());
}
