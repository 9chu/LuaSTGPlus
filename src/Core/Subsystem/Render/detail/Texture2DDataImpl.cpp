/**
 * @file
 * @date 2022/4/11
 * @author 9chu
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#include "Texture2DDataImpl.hpp"

#include <stb_image.h>
#include <GraphicsAccessories.hpp>
#include <GraphicsUtilities.h>
#include <lstg/Core/Logging.hpp>
#include <lstg/Core/Subsystem/VFS/ContainerStream.hpp>

using namespace std;
using namespace lstg;
using namespace lstg::Subsystem::Render::detail;

LSTG_DEF_LOG_CATEGORY(Texture2DDataImpl);

namespace
{
    Result<Subsystem::VFS::StreamPtr> ConvertToSeekableStream(Subsystem::VFS::StreamPtr stream) noexcept
    {
        // 如果本身就是可以 Seek 的，就忽略
        if (stream->IsSeekable())
            return stream;

        // 预分配
        Subsystem::VFS::MemoryStreamPtr ret;
        try
        {
            ret = make_shared<Subsystem::VFS::MemoryStream>();
            auto length = stream->GetLength();
            if (length)
                ret->Reserve(*length);
        }
        catch (...)
        {
            return make_error_code(errc::not_enough_memory);
        }

        // 从当前位置开始读取所有的内容
        const size_t kChunkSize = 16 * 1024;
        size_t readLength = 0;
        auto& container = ret->GetContainer();
        while (true)
        {
            // 分配 kChunkSize 个数据
            try
            {
                container.resize(readLength + kChunkSize);
            }
            catch (...)
            {
                return make_error_code(errc::not_enough_memory);
            }

            // 读取数据
            auto cnt = stream->Read(container.data() + readLength, kChunkSize);
            if (!cnt)
                return cnt.GetError();

            // 缩小实际大小
            if (*cnt)
            {
                readLength += *cnt;
                container.resize(readLength);
            }
            if (*cnt < kChunkSize)
                break;
        }
        return ret;
    }

    int StbImageReadStreamBridge(void* user, char* data, int size) noexcept
    {
        auto stream = static_cast<Subsystem::VFS::IStream*>(user);
        auto ret = stream->Read(reinterpret_cast<uint8_t*>(data), static_cast<size_t>(size));
        if (!ret)
        {
            LSTG_LOG_ERROR_CAT(Texture2DDataImpl, "Read stream fail: {}", ret.GetError());
            return -1;
        }
        return static_cast<int>(*ret);
    }

    void StbImageSkipStreamBridge(void* user, int n) noexcept
    {
        auto stream = static_cast<Subsystem::VFS::IStream*>(user);
        auto ret = stream->Seek(n, Subsystem::VFS::StreamSeekOrigins::Current);
        if (!ret)
            LSTG_LOG_ERROR_CAT(Texture2DDataImpl, "Seek stream fail: {}", ret.GetError());
    }

    int StbImageIsEofStreamBridge(void* user) noexcept
    {
        auto stream = static_cast<Subsystem::VFS::IStream*>(user);
        auto ret = stream->IsEof();
        if (!ret)
        {
            LSTG_LOG_ERROR_CAT(Texture2DDataImpl, "Test eof on stream fail: {}", ret.GetError());
            return 1;
        }
        return *ret;
    }

    struct StbImageMemoryDeleter
    {
        template <typename T>
        void operator()(T* p) const
        {
            if (p)
                ::stbi_image_free(p);
        }
    };

    using StbImageMemoryPtr = std::unique_ptr<uint8_t, StbImageMemoryDeleter>;

    uint32_t AlignedScanLineSize(uint32_t widthSize) noexcept
    {
        static const unsigned kAlignment = 4;
        return (widthSize + kAlignment - 1) & ~(kAlignment - 1);
    }

    Subsystem::Render::Texture2DFormats FromDiligent(Diligent::TEXTURE_FORMAT format) noexcept
    {
        switch (format)
        {
            case Diligent::TEX_FORMAT_R8_UNORM:
                return Subsystem::Render::Texture2DFormats::R8;
            case Diligent::TEX_FORMAT_RG8_UNORM:
                return Subsystem::Render::Texture2DFormats::R8G8;
            case Diligent::TEX_FORMAT_RGBA8_UNORM:
                return Subsystem::Render::Texture2DFormats::R8G8B8A8;
            case Diligent::TEX_FORMAT_RGBA8_UNORM_SRGB:
                return Subsystem::Render::Texture2DFormats::R8G8B8A8_SRGB;
            case Diligent::TEX_FORMAT_R16_UNORM:
                return Subsystem::Render::Texture2DFormats::R16;
            case Diligent::TEX_FORMAT_RG16_UNORM:
                return Subsystem::Render::Texture2DFormats::R16G16;
            case Diligent::TEX_FORMAT_RGBA16_UNORM:
                return Subsystem::Render::Texture2DFormats::R16G16B16A16;
            default:
                assert(false);
                return Subsystem::Render::Texture2DFormats::R8G8B8A8;
        }
    }

//    Diligent::TEXTURE_FORMAT ToDiligent(Subsystem::Render::Texture2DFormats format) noexcept
//    {
//        switch (format)
//        {
//            case Subsystem::Render::Texture2DFormats::R8:
//                return Diligent::TEX_FORMAT_R8_UNORM;
//            case Subsystem::Render::Texture2DFormats::R8G8:
//                return Diligent::TEX_FORMAT_RG8_UNORM;
//            case Subsystem::Render::Texture2DFormats::R8G8B8A8:
//                return Diligent::TEX_FORMAT_RGBA8_UNORM;
//            case Subsystem::Render::Texture2DFormats::R8G8B8A8_SRGB:
//                return Diligent::TEX_FORMAT_RGBA8_UNORM_SRGB;
//            case Subsystem::Render::Texture2DFormats::R16:
//                return Diligent::TEX_FORMAT_R16_UNORM;
//            case Subsystem::Render::Texture2DFormats::R16G16:
//                return Diligent::TEX_FORMAT_RG16_UNORM;
//            case Subsystem::Render::Texture2DFormats::R16G16B16A16:
//                return Diligent::TEX_FORMAT_RGBA16_UNORM;
//            default:
//                assert(false);
//                return Diligent::TEX_FORMAT_RGBA8_UNORM;
//        }
//    }
}

Result<void> Texture2DDataImpl::ReadImageInfoFromStream(uint32_t& width, uint32_t& height, VFS::StreamPtr stream) noexcept
{
    auto seekableStream = ConvertToSeekableStream(std::move(stream));
    if (!seekableStream)
        return seekableStream.GetError();

    ::stbi_io_callbacks callbacks {
        StbImageReadStreamBridge,
        StbImageSkipStreamBridge,
        StbImageIsEofStreamBridge,
    };
    int x, y, channels;
    auto ret = ::stbi_info_from_callbacks(&callbacks, seekableStream->get(), &x, &y, &channels);
    if (ret != 1)
    {
        LSTG_LOG_ERROR_CAT(Texture2DDataImpl, "stbi_load_from_callbacks fail: {}", ::stbi_failure_reason());
        return make_error_code(errc::io_error);
    }
    return {};
}

Texture2DDataImpl::Texture2DDataImpl(VFS::StreamPtr stream)
{
    assert(stream);
    auto seekableStream = ConvertToSeekableStream(std::move(stream));
    seekableStream.ThrowIfError();

    // 解码图像
    int x, y, channels;
    StbImageMemoryPtr data;
    ::stbi_io_callbacks callbacks {
        StbImageReadStreamBridge,
        StbImageSkipStreamBridge,
        StbImageIsEofStreamBridge,
    };
    data.reset(::stbi_load_from_callbacks(&callbacks, seekableStream->get(), &x, &y, &channels, 0));
    if (!data)
    {
        LSTG_LOG_ERROR_CAT(Texture2DDataImpl, "stbi_load_from_callbacks fail: {}", ::stbi_failure_reason());
        throw system_error(make_error_code(errc::io_error));
    }

    // 填充图像大小和位深
    int componentSize = 1;
    m_stDesc.Type = Diligent::RESOURCE_DIM_TEX_2D;
    m_stDesc.Width = static_cast<uint32_t>(x);
    m_stDesc.Height = static_cast<uint32_t>(y);
    switch (channels)
    {
        case 1:
            m_stDesc.Format = Diligent::TEX_FORMAT_R8_UNORM;
            componentSize = 1;
            break;
        case 2:
            m_stDesc.Format = Diligent::TEX_FORMAT_RG8_UNORM;
            componentSize = 2;
            break;
        case 3:
        case 4:
            m_stDesc.Format = Diligent::TEX_FORMAT_RGBA8_UNORM;
            componentSize = 4;
            break;
        default:
            assert(false);
            throw system_error(make_error_code(errc::invalid_argument));
    }

    // 发起数据拷贝并进行对齐
    uint32_t stride = AlignedScanLineSize(m_stDesc.Width * componentSize);
    m_stSubResources.resize(1);
    m_stMipMaps.resize(1);
    m_stMipMaps[0].resize(m_stDesc.Height * stride);
    m_stSubResources[0].Stride = stride;
    m_stSubResources[0].pData = m_stMipMaps[0].data();
    auto srcLineStride = x * channels;
    auto srcLineStart = data.get();
    auto destLineStart = m_stMipMaps[0].data();
    for (int h = 0; h < y; ++h)
    {
        if (componentSize == channels)
        {
            ::memcpy(destLineStart, srcLineStart, srcLineStride);
        }
        else
        {
            assert(channels == 3 && componentSize == 4);
            auto dest = destLineStart;
            auto src = srcLineStart;
            for (int w = 0; w < x; ++w)
            {
                dest[0] = src[0];  // r
                dest[1] = src[1];  // g
                dest[2] = src[2];  // b
                dest[3] = 0xFF;  // a
                dest += componentSize;
                src += channels;
            }
        }
        srcLineStart += srcLineStride;
        destLineStart += stride;
    }
}

Texture2DDataImpl::Texture2DDataImpl(uint32_t width, uint32_t height, Texture2DFormats format)
{
    // 填充图像大小和位深
    int componentSize = 1;
    m_stDesc.Type = Diligent::RESOURCE_DIM_TEX_2D;
    m_stDesc.Width = width;
    m_stDesc.Height = height;
    switch (format)
    {
        case Texture2DFormats::R8:
            m_stDesc.Format = Diligent::TEX_FORMAT_R8_UNORM;
            componentSize = 1;
            break;
        case Texture2DFormats::R8G8:
            m_stDesc.Format = Diligent::TEX_FORMAT_RG8_UNORM;
            componentSize = 2;
            break;
        case Texture2DFormats::R8G8B8A8:
        case Texture2DFormats::R8G8B8A8_SRGB:
            m_stDesc.Format = Diligent::TEX_FORMAT_RGBA8_UNORM;
            componentSize = 4;
            break;
        case Texture2DFormats::R16:
            m_stDesc.Format = Diligent::TEX_FORMAT_R16_UNORM;
            componentSize = 2;
            break;
        case Texture2DFormats::R16G16:
            m_stDesc.Format = Diligent::TEX_FORMAT_RG16_UNORM;
            componentSize = 4;
            break;
        case Texture2DFormats::R16G16B16A16:
            m_stDesc.Format = Diligent::TEX_FORMAT_RGBA16_UNORM;
            componentSize = 8;
            break;
        default:
            assert(false);
            throw system_error(make_error_code(errc::invalid_argument));
    }

    // 初始化数据
    uint32_t stride = AlignedScanLineSize(m_stDesc.Width * componentSize);
    m_stSubResources.resize(1);
    m_stMipMaps.resize(1);
    m_stMipMaps[0].resize(m_stDesc.Height * stride);
    m_stSubResources[0].Stride = stride;
    m_stSubResources[0].pData = m_stMipMaps[0].data();
}

size_t Texture2DDataImpl::GetStride() const noexcept
{
    // Mipmap 0 总是保存原始图像
    assert(!m_stSubResources.empty());
    return m_stSubResources[0].Stride;
}

Subsystem::Render::Texture2DFormats Texture2DDataImpl::GetFormat() const noexcept
{
    return FromDiligent(m_stDesc.Format);
}

Span<const uint8_t> Texture2DDataImpl::GetBuffer() const noexcept
{
    assert(!m_stMipMaps.empty());
    return { m_stMipMaps[0].data(), m_stMipMaps[0].size() };
}

Span<uint8_t> Texture2DDataImpl::GetBuffer() noexcept
{
    assert(!m_stMipMaps.empty());
    return { m_stMipMaps[0].data(), m_stMipMaps[0].size() };
}

Result<void> Texture2DDataImpl::GenerateMipmap(size_t count) noexcept
{
    // 计算 mipmap 层次
    uint32_t mipLevels = Diligent::ComputeMipLevelsCount(m_stDesc.Width, m_stDesc.Height);
    if (count > 0)
        mipLevels = std::min<uint32_t>(mipLevels, count);

    // 如果只有 1 层，就忽略 mipmap 创建
    if (mipLevels <= 1)
    {
        m_stSubResources.resize(1);
        m_stMipMaps.resize(1);
        return {};  // ignore
    }

    // 生成 mipmap
    try
    {
        vector<Diligent::TextureSubResData> newSubResources;
        vector<vector<uint8_t>> newMipMaps;
        newSubResources.resize(mipLevels);
        newMipMaps.resize(mipLevels);

        // 复制第一层
        newSubResources[0] = m_stSubResources[0];
        newMipMaps[0] = m_stMipMaps[0];

        // 生成其他层
        for (uint32_t m = 1; m < mipLevels; ++m)
        {
            auto mipLevelProps = Diligent::GetMipLevelProperties(m_stDesc, m);
            newMipMaps[m].resize(static_cast<size_t>(mipLevelProps.MipSize));
            newSubResources[m].pData  = newMipMaps[m].data();
            newSubResources[m].Stride = mipLevelProps.RowSize;

            auto finerMipProps = GetMipLevelProperties(m_stDesc, m - 1);
            Diligent::ComputeMipLevel(finerMipProps.LogicalWidth, finerMipProps.LogicalHeight, m_stDesc.Format,
                newSubResources[m - 1].pData, static_cast<size_t>(newSubResources[m - 1].Stride), newMipMaps[m].data(),
                static_cast<size_t>(newSubResources[m].Stride));
        }
        return {};
    }
    catch (...)
    {
        return make_error_code(errc::not_enough_memory);
    }
}
