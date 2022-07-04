/**
 * @file
 * @date 2022/4/11
 * @author 9chu
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#pragma once
#include <cstdint>
#include <memory>
#include "../../Span.hpp"
#include "../../Result.hpp"
#include "../../Math/Rectangle.hpp"

namespace Diligent
{
    struct ITexture;
}

namespace lstg::Subsystem
{
    class RenderSystem;
}

namespace lstg::Subsystem::Render
{
    class Material;
    class RenderDevice;
}

namespace lstg::Subsystem::Render
{
    /**
     * 纹理
     */
    class Texture
    {
        friend class lstg::Subsystem::Render::Material;
        friend class lstg::Subsystem::RenderSystem;

    public:
        Texture(RenderDevice& device, Diligent::ITexture* handler);
        Texture(const Texture&) = delete;
        Texture(Texture&&) noexcept = delete;
        ~Texture();

    public:
        /**
         * 获取宽度
         */
        uint32_t GetWidth() const noexcept;

        /**
         * 获取高度
         */
        uint32_t GetHeight() const noexcept;

        /**
         * 更新 2D 纹理（针对动态纹理）
         * @param range 纹理范围
         * @param data 数据源
         * @param stride 一行的字节数
         * @param mipmapLevel Mipmap 级别
         * @param arrayIndex 数组下标
         */
        Result<void> Commit(Math::ImageRectangle range, Span<const uint8_t> data, size_t stride, size_t mipmapLevel = 0,
            size_t arrayIndex = 0) noexcept;

    private:
        RenderDevice& m_stDevice;
        Diligent::ITexture* m_pNativeHandler = nullptr;
    };

    using TexturePtr = std::shared_ptr<Texture>;
}
