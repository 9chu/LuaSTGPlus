/**
 * @file
 * @date 2022/7/10
 * @author 9chu
 * 此文件为 LuaSTGPlus 项目的一部分，版权与许可声明详见 COPYRIGHT.txt。
 */
#pragma once
#include <lstg/Core/Subsystem/Asset/Asset.hpp>
#include <lstg/Core/Subsystem/Render/Font/IFontFace.hpp>
#include <lstg/Core/Subsystem/Render/Font/FontCollection.hpp>

namespace lstg::v2::Asset
{
    /**
     * TrueType 字体资源
     */
    class TrueTypeFontAsset :
        public Subsystem::Asset::Asset
    {
        friend class TrueTypeFontAssetLoader;

    public:
        [[nodiscard]] static Subsystem::Asset::AssetTypeId GetAssetTypeIdStatic() noexcept;

    public:
        TrueTypeFontAsset(std::string name, std::string path, uint32_t fontSize);

    public:
        /**
         * 获取资源路径
         */
        [[nodiscard]] const std::string& GetPath() const noexcept { return m_stPath; }

        /**
         * 获取字体
         */
        [[nodiscard]] const Subsystem::Render::Font::FontFacePtr& GetFontFace() const noexcept { return m_pFontFace; }

        /**
         * 获取字体集合
         */
        [[nodiscard]] const Subsystem::Render::Font::FontCollectionPtr& GetFontCollection() const noexcept { return m_pFontCollection; }

        /**
         * 获取字体大小
         */
        [[nodiscard]] uint32_t GetFontSize() const noexcept { return m_uFontSize; }

    protected:  // Asset
        [[nodiscard]] Subsystem::Asset::AssetTypeId GetAssetTypeId() const noexcept override;

    private:
        void UpdateResource(Subsystem::Render::Font::FontFacePtr fontFace,
            Subsystem::Render::Font::FontCollectionPtr fontCollection) noexcept;

    private:
        // 资源属性
        const std::string m_stPath;

        // 资产对象实例
        Subsystem::Render::Font::FontFacePtr m_pFontFace;
        Subsystem::Render::Font::FontCollectionPtr m_pFontCollection;
        uint32_t m_uFontSize = 0u;
    };
}
