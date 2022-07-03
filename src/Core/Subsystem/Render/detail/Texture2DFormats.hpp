/**
 * @file
 * @date 2022/7/1
 * @author 9chu
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#pragma once
#include <Texture.h>
#include <lstg/Core/Subsystem/Render/Texture2DData.hpp>

namespace lstg::Subsystem::Render::detail
{
    inline Subsystem::Render::Texture2DFormats FromDiligent(Diligent::TEXTURE_FORMAT format) noexcept
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

    inline Diligent::TEXTURE_FORMAT ToDiligent(Subsystem::Render::Texture2DFormats format) noexcept
    {
        switch (format)
        {
            case Subsystem::Render::Texture2DFormats::R8:
                return Diligent::TEX_FORMAT_R8_UNORM;
            case Subsystem::Render::Texture2DFormats::R8G8:
                return Diligent::TEX_FORMAT_RG8_UNORM;
            case Subsystem::Render::Texture2DFormats::R8G8B8A8:
                return Diligent::TEX_FORMAT_RGBA8_UNORM;
            case Subsystem::Render::Texture2DFormats::R8G8B8A8_SRGB:
                return Diligent::TEX_FORMAT_RGBA8_UNORM_SRGB;
            case Subsystem::Render::Texture2DFormats::R16:
                return Diligent::TEX_FORMAT_R16_UNORM;
            case Subsystem::Render::Texture2DFormats::R16G16:
                return Diligent::TEX_FORMAT_RG16_UNORM;
            case Subsystem::Render::Texture2DFormats::R16G16B16A16:
                return Diligent::TEX_FORMAT_RGBA16_UNORM;
            default:
                assert(false);
                return Diligent::TEX_FORMAT_RGBA8_UNORM;
        }
    }

    inline uint32_t GetPixelComponentSize(Subsystem::Render::Texture2DFormats format) noexcept
    {
        switch (format)
        {
            case Texture2DFormats::R8:
                return 1;
            case Texture2DFormats::R8G8:
                return 2;
            case Texture2DFormats::R8G8B8A8:
                return 4;
            case Texture2DFormats::R8G8B8A8_SRGB:
                return 4;
            case Texture2DFormats::R16:
                return 2;
            case Texture2DFormats::R16G16:
                return 4;
            case Texture2DFormats::R16G16B16A16:
                return 8;
            default:
                assert(false);
                return 4;
        }
    }

    inline uint32_t AlignedScanLineSize(uint32_t widthSize) noexcept
    {
        static const unsigned kAlignment = 4;
        return (widthSize + kAlignment - 1) & ~(kAlignment - 1);
    }
}
