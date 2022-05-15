/**
 * @file
 * @date 2022/4/11
 * @author 9chu
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#pragma once
#include <vector>
#include <lstg/Core/Subsystem/Render/Texture2DData.hpp>

#include <Texture.h>

namespace lstg::Subsystem
{
    class RenderSystem;
}

namespace lstg::Subsystem::Render::detail
{
    class Texture2DDataImpl
    {
        friend class lstg::Subsystem::RenderSystem;

    public:
        /**
         * 从流获取图像信息
         * @param[out] width 宽度
         * @param[out] height 高度
         * @param stream 流
         * @return 是否成功
         */
        static Result<void> ReadImageInfoFromStream(uint32_t& width, uint32_t& height, VFS::StreamPtr stream) noexcept;

    public:
        /**
         * 从文件加载纹理
         * 支持 stb_image 所兼容格式
         * @param stream 数据流
         */
        Texture2DDataImpl(VFS::StreamPtr stream);

        /**
         * 创建空白纹理
         * @param width 宽度
         * @param height 高度
         * @param format 像素格式
         */
        Texture2DDataImpl(uint32_t width, uint32_t height, Texture2DFormats format);

    public:
        /**
         * 获取宽度
         */
        [[nodiscard]] uint32_t GetWidth() const noexcept { return m_stDesc.Width; }

        /**
         * 获取高度
         */
        [[nodiscard]] uint32_t GetHeight() const noexcept { return m_stDesc.Height; }

        /**
         * 行对齐字节数
         */
        [[nodiscard]] size_t GetStride() const noexcept;

        /**
         * 获取格式
         */
        [[nodiscard]] Texture2DFormats GetFormat() const noexcept;

        /**
         * 获取数据块
         */
        [[nodiscard]] Span<const uint8_t> GetBuffer() const noexcept;
        [[nodiscard]] Span<uint8_t> GetBuffer() noexcept;

        /**
         * 生成 Mipmap
         * @param count 数量，设置为 0 表示自动
         * @return 是否成功
         */
        Result<void> GenerateMipmap(size_t count = 0) noexcept;

    private:
        Diligent::TextureDesc m_stDesc;
        std::vector<Diligent::TextureSubResData> m_stSubResources;
        std::vector<std::vector<uint8_t>> m_stMipMaps;
    };
}
