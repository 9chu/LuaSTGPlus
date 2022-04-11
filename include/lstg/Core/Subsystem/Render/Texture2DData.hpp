/**
 * @file
 * @date 2022/4/11
 * @author 9chu
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#pragma once
#include <cstdint>
#include <memory>
#include "../VFS/IStream.hpp"
#include "../../Span.hpp"

namespace lstg::Subsystem
{
    class RenderSystem;
}

namespace lstg::Subsystem::Render
{
    namespace detail
    {
        class Texture2DDataImpl;
    }

    /**
     * 2D 纹理格式
     * 默认都是 UNORM 形式
     */
    enum class Texture2DFormats
    {
        R8,
        R8G8,
        R8G8B8A8,
        R8G8B8A8_SRGB,
        R16,
        R16G16,
        R16G16B16A16,
    };

    /**
     * 纹理数据
     */
    class Texture2DData
    {
        friend class lstg::Subsystem::RenderSystem;

    public:
        /**
         * 从文件加载纹理
         * 支持 stb_image 所兼容格式
         * @param stream 数据流
         */
        Texture2DData(VFS::StreamPtr stream);

        /**
         * 创建空白纹理
         * @param width 宽度
         * @param height 高度
         * @param format 像素格式
         */
        Texture2DData(uint32_t width, uint32_t height, Texture2DFormats format);

        Texture2DData(const Texture2DData&) = delete;
        Texture2DData(Texture2DData&&) noexcept = delete;

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
         * 行对齐字节数
         */
        size_t GetStride() const noexcept;

        /**
         * 获取格式
         */
        Texture2DFormats GetFormat() const noexcept;

        /**
         * 获取数据块
         */
        Span<const uint8_t> GetBuffer() const noexcept;
        Span<uint8_t> GetBuffer() noexcept;

        /**
         * 生成 Mipmap
         * @param count 数量，设置为 0 表示自动
         * @return 是否成功
         */
        Result<void> GenerateMipmap(size_t count = 0) noexcept;

    private:
        std::shared_ptr<detail::Texture2DDataImpl> m_pImpl;
    };
}
