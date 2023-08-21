/**
 * @file
 * @date 2023/8/19
 * @author 9chu
 * 此文件为 LuaSTGPlus 项目的一部分，版权与许可声明详见 COPYRIGHT.txt。
 */
#include <lstg/Core/Subsystem/VFS/AndroidAssetStream.hpp>

#ifdef LSTG_PLATFORM_ANDROID

#include <android/asset_manager.h>

using namespace std;
using namespace lstg;
using namespace lstg::Subsystem::VFS;

AndroidAssetStream::AndroidAssetStream(AAssetManager* assetManager, const char* path)
    : m_pManager(assetManager), m_stPath(path)
{
    assert(assetManager);
    m_pAsset = ::AAssetManager_open(assetManager, path, AASSET_MODE_RANDOM);
    if (!m_pAsset)
        throw system_error(make_error_code(errc::no_such_file_or_directory));
}

AndroidAssetStream::AndroidAssetStream(AAssetManager* assetManager, std::string&& path, AAsset* asset) noexcept
    : m_pManager(assetManager), m_stPath(std::move(path)), m_pAsset(asset)
{
    assert(assetManager && asset);
}

AndroidAssetStream::~AndroidAssetStream()
{
    if (m_pAsset)
        ::AAsset_close(m_pAsset);
}

AndroidAssetStream::AndroidAssetStream(AndroidAssetStream&& rhs) noexcept
    : m_pManager(rhs.m_pManager), m_stPath(std::move(rhs.m_stPath)), m_pAsset(rhs.m_pAsset)
{
    rhs.m_pManager = nullptr;
    rhs.m_pAsset = nullptr;
}

AndroidAssetStream& AndroidAssetStream::operator=(AndroidAssetStream&& rhs) noexcept
{
    if (this == &rhs)
        return *this;

    if (m_pAsset)
        ::AAsset_close(m_pAsset);

    m_pManager = rhs.m_pManager;
    m_stPath = std::move(rhs.m_stPath);
    m_pAsset = rhs.m_pAsset;

    rhs.m_pManager = nullptr;
    rhs.m_pAsset = nullptr;
    return *this;
}

bool AndroidAssetStream::IsReadable() const noexcept
{
    return true;
}

bool AndroidAssetStream::IsWriteable() const noexcept
{
    return false;
}

bool AndroidAssetStream::IsSeekable() const noexcept
{
    return true;
}

Result<uint64_t> AndroidAssetStream::GetLength() const noexcept
{
    assert(m_pAsset);
    auto len = ::AAsset_getLength64(m_pAsset);
    if (len == static_cast<off64_t>(-1))
        return make_error_code(errc::io_error);
    return static_cast<uint64_t>(len);
}

Result<void> AndroidAssetStream::SetLength(uint64_t length) noexcept
{
    return make_error_code(errc::not_supported);
}

Result<uint64_t> AndroidAssetStream::GetPosition() const noexcept
{
    assert(m_pAsset);
    auto pos = ::AAsset_seek64(m_pAsset, 0, SEEK_CUR);
    if (pos == static_cast<off64_t>(-1))
        return make_error_code(errc::io_error);
    return static_cast<uint64_t>(pos);
}

Result<void> AndroidAssetStream::Seek(int64_t offset, StreamSeekOrigins origin) noexcept
{
    assert(m_pAsset);

    off64_t off = 0;
    switch (origin)
    {
        case StreamSeekOrigins::Begin:
            off = ::AAsset_seek64(m_pAsset, offset, SEEK_SET);
            break;
        case StreamSeekOrigins::Current:
            off = ::AAsset_seek64(m_pAsset, offset, SEEK_CUR);
            break;
        case StreamSeekOrigins::End:
            off = ::AAsset_seek64(m_pAsset, offset, SEEK_END);
            break;
        default:
            assert(false);
            break;
    }
    if (off == static_cast<off64_t>(-1))
        return make_error_code(errc::io_error);
    return {};
}

Result<bool> AndroidAssetStream::IsEof() const noexcept
{
    assert(m_pAsset);
    auto rest = ::AAsset_getRemainingLength64(m_pAsset);
    if (rest == static_cast<off64_t>(-1))
        return make_error_code(errc::io_error);
    return rest == 0;
}

Result<void> AndroidAssetStream::Flush() noexcept
{
    return make_error_code(errc::not_supported);
}

Result<size_t> AndroidAssetStream::Read(uint8_t* buffer, size_t length) noexcept
{
    assert(m_pAsset);

    if (length == 0)
        return static_cast<size_t>(0u);

    assert(length <= numeric_limits<uint32_t>::max());
    auto ret = ::AAsset_read(m_pAsset, buffer, length);
    if (ret < 0)
        return make_error_code(errc::io_error);
    return static_cast<size_t>(ret);
}

Result<void> AndroidAssetStream::Write(const uint8_t* buffer, size_t length) noexcept
{
    return make_error_code(errc::not_supported);
}

Result<StreamPtr> AndroidAssetStream::Clone() const noexcept
{
    assert(m_pManager && m_pAsset);

    // 获取当前的读写位置
    auto pos = GetPosition();
    if (!pos)
        return pos.GetError();

    try
    {
        auto stream = std::make_shared<AndroidAssetStream>(m_pManager, m_stPath.c_str());

        // 设置读写位置
        auto err = stream->Seek(*pos, StreamSeekOrigins::Begin);
        if (!err)
            return pos.GetError();

        return stream;
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

#endif
