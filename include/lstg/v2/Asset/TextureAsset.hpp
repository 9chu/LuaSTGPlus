/**
 * @file
 * @date 2022/5/16
 * @author 9chu
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#pragma once
#include <optional>
#include <lstg/Core/Subsystem/Asset/BasicTextureAsset.hpp>

namespace lstg::v2::Asset
{
    class TextureAssetLoader;

    /**
     * 区别于 Subsystem::Asset::TextureAsset，增加 PPU 的概念。
     */
    class TextureAsset :
        public Subsystem::Asset::Asset
    {
        friend class BasicTextureAssetLoader;

    public:
        [[nodiscard]] static Subsystem::Asset::AssetTypeId GetAssetTypeIdStatic() noexcept;

    public:
        TextureAsset(std::string name, Subsystem::Asset::BasicTextureAssetPtr basicTexture, double pixelPerUnit);

    public:
        /**
         * 获取实际纹理资源
         */
        const Subsystem::Asset::BasicTextureAssetPtr& GetBasicTexture() const noexcept;

        /**
         * 获取PPU
         */
        [[nodiscard]] double GetPixelPerUnit() const noexcept;

        /**
         * 获得宽度
         * = 实际像素宽度 / PPU
         */
        [[nodiscard]] double GetWidth() const noexcept;

        /**
         * 获取高度
         * = 实际像素高度 / PPU
         */
        [[nodiscard]] double GetHeight() const noexcept;

    protected:  // Asset
        [[nodiscard]] Subsystem::Asset::AssetTypeId GetAssetTypeId() const noexcept override;

    private:
        // 依赖
        const Subsystem::Asset::BasicTextureAssetPtr m_pBasicTexture;

        // 资源属性
        const double m_dPixelPerUnit = 1.;
    };

    using TextureAssetPtr = std::shared_ptr<TextureAsset>;
}
