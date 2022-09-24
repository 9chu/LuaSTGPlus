/**
 * @file
 * @date 2022/7/13
 * @author 9chu
 * 此文件为 LuaSTGPlus 项目的一部分，版权与许可声明详见 COPYRIGHT.txt。
 */
#pragma once
#include <lstg/Core/Subsystem/Asset/Asset.hpp>
#include <lstg/Core/Subsystem/Asset/BasicTexture2DAsset.hpp>
#include <lstg/Core/Subsystem/Render/Drawing2D/Sprite.hpp>
#include <lstg/Core/Subsystem/Render/Font/IFontFace.hpp>
#include <lstg/Core/Subsystem/Render/Font/FontCollection.hpp>
#include "../BlendMode.hpp"

namespace lstg::v2::Asset
{
    /**
     * HGE 字体资源
     */
    class HgeFontAsset :
        public Subsystem::Asset::Asset
    {
        friend class HgeFontAssetLoader;

    public:
        [[nodiscard]] static Subsystem::Asset::AssetTypeId GetAssetTypeIdStatic() noexcept;

    public:
        HgeFontAsset(std::string name, std::string path, bool mipmap);
        ~HgeFontAsset();

    public:
        /**
         * 获取资源路径
         */
        [[nodiscard]] const std::string& GetPath() const noexcept { return m_stPath; }

        /**
         * 是否生成 mipmap
         */
        [[nodiscard]] bool IsGenerateMipmaps() const noexcept { return m_bGenerateMipmaps; }

        /**
         * 获取关联的纹理
         */
        [[nodiscard]] Subsystem::Asset::BasicTexture2DAssetPtr GetFontTexture() const noexcept { return m_pFontTexture; }

        /**
         * 获取字体
         */
        [[nodiscard]] const Subsystem::Render::Font::FontFacePtr& GetFontFace() const noexcept { return m_pFontFace; }

        /**
         * 获取字体集合
         */
        [[nodiscard]] const Subsystem::Render::Font::FontCollectionPtr& GetFontCollection() const noexcept { return m_pFontCollection; }

        /**
         * 获取默认的混合模式
         */
        [[nodiscard]] const BlendMode& GetDefaultBlendMode() const noexcept { return m_stDefaultBlendMode; }

        /**
         * 设置默认的混合模式
         */
        void SetDefaultBlendMode(BlendMode mode) noexcept { m_stDefaultBlendMode = mode; }

        /**
         * 获取默认的混合颜色
         */
        [[nodiscard]] Subsystem::Render::ColorRGBA32 GetDefaultBlendColor() const noexcept
        {
            return m_stDefaultBlendColor;
        }

        /**
         * 设置默认的混合颜色
         */
        void SetDefaultBlendColor(Subsystem::Render::ColorRGBA32 color) noexcept
        {
            m_stDefaultBlendColor = color;
        }

    protected:  // Asset
        [[nodiscard]] Subsystem::Asset::AssetTypeId GetAssetTypeId() const noexcept override;
        void OnRemove() noexcept override;

    private:
        void FreeResource() noexcept;
        void UpdateResource(Subsystem::Asset::BasicTexture2DAssetPtr fontTexture, Subsystem::Render::Font::FontFacePtr fontFace,
            Subsystem::Render::Font::FontCollectionPtr fontCollection) noexcept;

    private:
        // 资源属性
        const std::string m_stPath;
        const bool m_bGenerateMipmaps;

        // 关联资产
        Subsystem::Asset::BasicTexture2DAssetPtr m_pFontTexture;

        // 资产对象实例
        Subsystem::Render::Font::FontFacePtr m_pFontFace;
        Subsystem::Render::Font::FontCollectionPtr m_pFontCollection;
        BlendMode m_stDefaultBlendMode;  // 默认混合模式
        Subsystem::Render::ColorRGBA32 m_stDefaultBlendColor = 0xFFFFFFFFu;
    };
}
