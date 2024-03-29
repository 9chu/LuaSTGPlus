/**
 * @file
 * @date 2022/4/26
 * @author 9chu
 * 此文件为 LuaSTGPlus 项目的一部分，版权与许可声明详见 COPYRIGHT.txt。
 */
#pragma once
#include <optional>
#include "Asset.hpp"
#include "../Render/Texture.hpp"

namespace lstg::Subsystem::Asset
{
    class BasicTexture2DAssetLoader;

    /**
     * 纹理资产
     */
    class BasicTexture2DAsset :
        public Asset
    {
        friend class BasicTexture2DAssetLoader;

    public:
        [[nodiscard]] static AssetTypeId GetAssetTypeIdStatic() noexcept;
        
    public:
        BasicTexture2DAsset(std::string name, std::string path);

    public:
        /**
         * 获取资源路径
         */
        [[nodiscard]] const std::string& GetPath() const noexcept { return m_stPath; }

        /**
         * 获取纹理对象
         */
        [[nodiscard]] const Render::TexturePtr& GetTexture() const noexcept { return m_pTexture; }

        /**
         * 获取宽度
         */
        [[nodiscard]] uint32_t GetWidth() const noexcept;

        /**
         * 获取高度
         */
        [[nodiscard]] uint32_t GetHeight() const noexcept;

    protected:  // Asset
        [[nodiscard]] AssetTypeId GetAssetTypeId() const noexcept override;

    private:
        void InitTextureInfo() noexcept;
        void ReceiveLoadedAsset(Render::TexturePtr tex) noexcept;

    private:
        struct TextureInfo
        {
            uint32_t Width;
            uint32_t Height;
        };

        // 资源属性
        const std::string m_stPath;

        // 纹理信息
        std::optional<TextureInfo> m_stTextureInfo;
        Render::TexturePtr m_pTexture;
    };

    using BasicTexture2DAssetPtr = std::shared_ptr<BasicTexture2DAsset>;
}
