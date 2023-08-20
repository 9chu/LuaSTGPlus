/**
 * @file
 * @date 2023/8/20
 * @author 9chu
 * 此文件为 LuaSTGPlus 项目的一部分，版权与许可声明详见 COPYRIGHT.txt。
 */
#include <lstg/Core/Subsystem/VFS/AndroidAssetFileSystem.hpp>

#ifdef LSTG_PLATFORM_ANDROID

#include <cassert>
#include <android/asset_manager.h>

#include <lstg/Core/Logging.hpp>
#include <lstg/Core/Subsystem/VFS/AndroidAssetStream.hpp>

using namespace std;
using namespace lstg;
using namespace lstg::Subsystem::VFS;

namespace
{
    class AndroidAssetDirectoryIterator :
        public IDirectoryIterator
    {
    public:
        AndroidAssetDirectoryIterator(AAssetDir* dir)
            : m_pDirectory(dir)
        {
            assert(m_pDirectory);
            MoveNext();
        }
        ~AndroidAssetDirectoryIterator() override
        {
            ::AAssetDir_close(m_pDirectory);
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
            auto name = ::AAssetDir_getNextFileName(m_pDirectory);
            if (name == nullptr)
            {
                m_stCurrentName = {};
                return false;
            }
            m_stCurrentName = Path::Normalize(name);
            return true;
        }

    private:
        ::AAssetDir* m_pDirectory = nullptr;
        Path m_stCurrentName;
    };
}

AndroidAssetFileSystem::AndroidAssetFileSystem(AAssetManager* assetManager, const char* prefix) noexcept
    : m_pAssetManager(assetManager), m_stPathPrefix(prefix)
{
    assert(m_pAssetManager);

    // 保证前缀是空或者以 '/' 结尾
    while (!m_stPathPrefix.empty() && m_stPathPrefix[0] == '/')
        m_stPathPrefix.erase(0);
    if (!m_stPathPrefix.empty() && m_stPathPrefix[m_stPathPrefix.length() - 1] != '/')
        m_stPathPrefix.push_back('/');
}

Result<void> AndroidAssetFileSystem::CreateDirectory(Path path) noexcept
{
    return make_error_code(errc::not_supported);
}

Result<void> AndroidAssetFileSystem::Remove(Path path) noexcept
{
    return make_error_code(errc::not_supported);
}

Result<void> AndroidAssetFileSystem::Rename(Path from, Path to) noexcept
{
    return make_error_code(errc::not_supported);
}

Result<FileAttribute> AndroidAssetFileSystem::GetFileAttribute(Path path) noexcept
{
    string assetPath;
    try
    {
        assetPath = MakeAssetPath(path);
    }
    catch (...)
    {
        return make_error_code(errc::not_enough_memory);
    }

    auto dir = ::AAssetManager_openDir(m_pAssetManager, assetPath.c_str());
    if (!dir)
    {
        auto file = ::AAssetManager_open(m_pAssetManager, assetPath.c_str(), AASSET_MODE_RANDOM);
        if (!file)
            return make_error_code(errc::no_such_file_or_directory);

        auto length = ::AAsset_getLength64(file);
        if (length == static_cast<off64_t>(-1))
        {
            ::AAsset_close(file);
            return make_error_code(errc::io_error);
        }

        FileAttribute ret;
        ret.Type = FileType::RegularFile;
        ret.LastModified = 0;
        ret.Size = length;

        ::AAsset_close(file);
        return ret;
    }
    else
    {
        ::AAssetDir_close(dir);

        FileAttribute ret;
        ret.Type = FileType::Directory;
        ret.LastModified = 0;
        ret.Size = 0;
        return ret;
    }
}

Result<DirectoryIteratorPtr> AndroidAssetFileSystem::VisitDirectory(Path path) noexcept
{
    string assetPath;
    try
    {
        assetPath = MakeAssetPath(path);
    }
    catch (...)
    {
        return make_error_code(errc::not_enough_memory);
    }

    // FIXME: 无法返回文件夹
    // see https://github.com/android/ndk-samples/issues/603
    auto dir = ::AAssetManager_openDir(m_pAssetManager, assetPath.c_str());
    if (!dir)
        return make_error_code(errc::no_such_file_or_directory);

    try
    {
        return make_shared<AndroidAssetDirectoryIterator>(dir);
    }
    catch (...)
    {
        return make_error_code(errc::not_enough_memory);
    }
}

Result<StreamPtr> AndroidAssetFileSystem::OpenFile(Path path, FileAccessMode access, FileOpenFlags flags) noexcept
{
    if (access == FileAccessMode::ReadWrite || access == FileAccessMode::Write)
        return make_error_code(errc::permission_denied);
    if (flags & FileOpenFlags::Truncate)
        return make_error_code(errc::invalid_argument);

    string assetPath;
    try
    {
        assetPath = MakeAssetPath(path);
    }
    catch (...)
    {
        return make_error_code(errc::not_enough_memory);
    }

    auto file = ::AAssetManager_open(m_pAssetManager, assetPath.c_str(), AASSET_MODE_RANDOM);
    if (!file)
        return make_error_code(errc::no_such_file_or_directory);

    return make_shared<AndroidAssetStream>(m_pAssetManager, std::move(assetPath), file);
}

const std::string& AndroidAssetFileSystem::GetUserData() const noexcept
{
    return m_stUserData;
}

void AndroidAssetFileSystem::SetUserData(std::string ud) noexcept
{
    m_stUserData = std::move(ud);
}

std::string AndroidAssetFileSystem::MakeAssetPath(const Path& path) const
{
    auto ret = path.ToString();
    while (!ret.empty() && ret[0] == '/')
        ret.erase(0);
    ret.insert(0, m_stPathPrefix);
    return ret;
}

#endif
