/**
 * @file
 * @date 2022/5/16
 * @author 9chu
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#pragma once
#include <optional>
#include <lstg/Core/Subsystem/Asset/BasicTexture2DAsset.hpp>
#include <lstg/Core/Subsystem/Render/Camera.hpp>
#include <lstg/Core/Subsystem/Render/Drawing2D/Texture2D.hpp>

namespace lstg::v2::Asset
{
    class TextureAssetLoader;

    /**
     * 2D纹理资源化包装
     */
    class TextureAsset :
        public Subsystem::Asset::Asset
    {
        friend class TextureAssetLoader;

    public:
        [[nodiscard]] static Subsystem::Asset::AssetTypeId GetAssetTypeIdStatic() noexcept;

    public:
        TextureAsset(std::string name, Subsystem::Asset::BasicTexture2DAssetPtr textureAsset, float pixelPerUnit);
        TextureAsset(std::string name, Subsystem::Render::Camera::OutputViews view, float pixelPerUnit);
        ~TextureAsset();

    public:
        /**
         * 是否是 RenderTarget
         */
        [[nodiscard]] bool IsRenderTarget() const noexcept;

        /**
         * 获取纹理资源
         * @pre !IsRenderTarget()
         * @note 当且仅当类型不是 RT 时可以调用
         */
        [[nodiscard]] const Subsystem::Asset::BasicTexture2DAssetPtr& GetBasicTextureAsset() const noexcept;

        /**
         * 获取 RT 视图
         * @pre IsRenderTarget()
         * @note 当且仅当类型是 RT 时可以调用
         */
        [[nodiscard]] Subsystem::Render::Camera::OutputViews& GetOutputViews() noexcept;

        /**
         * 获取实际用于渲染的纹理定义
         */
        [[nodiscard]] const Subsystem::Render::Drawing2D::Texture2D& GetDrawingTexture() const noexcept { return m_stDrawingTexture; }

        /**
         * 获取PPU
         */
        [[nodiscard]] float GetPixelPerUnit() const noexcept { return m_stDrawingTexture.GetPixelPerUnit(); }

        /**
         * 获得宽度
         * = 实际像素宽度 / PPU
         */
        [[nodiscard]] double GetWidth() const noexcept { return m_stDrawingTexture.GetWidth(); }

        /**
         * 获取高度
         * = 实际像素高度 / PPU
         */
        [[nodiscard]] double GetHeight() const noexcept { return m_stDrawingTexture.GetHeight(); }

    protected:  // Asset
        [[nodiscard]] Subsystem::Asset::AssetTypeId GetAssetTypeId() const noexcept override;
        void OnRemove() noexcept override;

    private:
        void FreeResource() noexcept;
        void UpdateResource() noexcept;

    private:
        // 依赖
        std::variant<Subsystem::Asset::BasicTexture2DAssetPtr, Subsystem::Render::Camera::OutputViews> m_stUnderlay;
        Subsystem::Render::Drawing2D::Texture2D m_stDrawingTexture;
    };

    using TextureAssetPtr = std::shared_ptr<TextureAsset>;
}
