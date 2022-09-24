/**
 * @file
 * @date 2022/4/11
 * @author 9chu
 * 此文件为 LuaSTGPlus 项目的一部分，版权与许可声明详见 COPYRIGHT.txt。
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
    width = static_cast<uint32_t>(x);
    height = static_cast<uint32_t>(y);
    return {};
}

Texture2DDataImpl::Texture2DDataImpl(VFS::StreamPtr stream, bool convertToRGBA32)
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
    if (convertToRGBA32)
    {
        m_stDesc.Format = Diligent::TEX_FORMAT_RGBA8_UNORM_SRGB;
        componentSize = 4;
    }
    else
    {
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
                m_stDesc.Format = Diligent::TEX_FORMAT_RGBA8_UNORM_SRGB;
                componentSize = 4;
                break;
            default:
                assert(false);
                throw system_error(make_error_code(errc::invalid_argument));
        }
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
            assert(componentSize == 4);
            auto dest = destLineStart;
            auto src = srcLineStart;
            for (int w = 0; w < x; ++w)
            {
                if (channels == 3)
                {
                    dest[0] = src[0];  // r
                    dest[1] = src[1];  // g
                    dest[2] = src[2];  // b
                    dest[3] = 0xFF;  // a
                }
                else if (channels == 2)
                {
                    dest[0] = src[0];  // r
                    dest[1] = src[0];  // g
                    dest[2] = src[0];  // b
                    dest[3] = src[1];  // a
                }
                else if (channels == 1)
                {
                    dest[0] = src[0];  // r
                    dest[1] = src[0];  // g
                    dest[2] = src[0];  // b
                    dest[3] = 0xFF;  // a
                }
                else
                {
                    assert(false);
                }
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
    auto componentSize = GetPixelComponentSize(format);
    m_stDesc.Type = Diligent::RESOURCE_DIM_TEX_2D;
    m_stDesc.Width = width;
    m_stDesc.Height = height;
    m_stDesc.Format = ToDiligent(format);

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

            auto attr = Diligent::ComputeMipLevelAttribs {
                m_stDesc.Format,
                finerMipProps.LogicalWidth,
                finerMipProps.LogicalHeight,
                newSubResources[m - 1].pData,
                static_cast<size_t>(newSubResources[m - 1].Stride),
                newMipMaps[m].data(),
                static_cast<size_t>(newSubResources[m].Stride),
            };
            Diligent::ComputeMipLevel(attr);
        }
        return {};
    }
    catch (...)
    {
        return make_error_code(errc::not_enough_memory);
    }
}
