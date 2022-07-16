/**
 * @file
 * @date 2022/5/15
 * @author 9chu
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#pragma once
#include <array>
#include <optional>
#include <lstg/Core/Subsystem/Render/Drawing2D/Sprite.hpp>
#include "TextureAsset.hpp"
#include "../BlendMode.hpp"
#include "../MathAlias.hpp"

namespace lstg::v2::Asset
{
    class SpriteAssetLoader;

    /**
     * 精灵资源
     */
    class SpriteAsset :
        public Subsystem::Asset::Asset
    {
        friend class SpriteAssetLoader;

    public:
        [[nodiscard]] static Subsystem::Asset::AssetTypeId GetAssetTypeIdStatic() noexcept;

    public:
        SpriteAsset(std::string name, TextureAssetPtr texture, Math::ImageRectangleFloat frame, ColliderShape colliderShape);

    public:
        /**
         * 获取关联的纹理资产
         */
        [[nodiscard]] const TextureAssetPtr& GetTextureAsset() const noexcept { return m_pTextureAsset; }

        /**
         * 获取绘制用精灵定义
         */
        [[nodiscard]] const Subsystem::Render::Drawing2D::Sprite& GetDrawingSprite() const noexcept { return m_stSprite; }

        /**
         * 获取帧
         */
        [[nodiscard]] const Math::ImageRectangleFloat& GetFrame() const noexcept { return m_stSprite.GetFrame(); }

        /**
         * 获取锚点
         */
        [[nodiscard]] const glm::vec2& GetAnchor() const noexcept { return m_stSprite.GetAnchor(); }

        /**
         * 设置锚点
         */
        void SetAnchor(glm::vec2 vec) noexcept { m_stSprite.SetAnchor(vec); }

        /**
         * 获取碰撞形状
         */
        [[nodiscard]] const ColliderShape& GetColliderShape() const noexcept { return m_stColliderShape; }

        /**
         * 设置碰撞形状
         */
        void SetColliderShape(ColliderShape shape) noexcept { m_stColliderShape = shape; }

        /**
         * 获取默认的混合模式
         */
        [[nodiscard]] const BlendMode& GetDefaultBlendMode() const noexcept { return m_stDefaultBlendMode; }

        /**
         * 设置默认的混合模式
         */
        void SetDefaultBlendMode(BlendMode mode) noexcept;

        /**
         * 获取默认的混合颜色
         */
        [[nodiscard]] const Subsystem::Render::Drawing2D::SpriteColorComponents& GetDefaultBlendColor() const noexcept
        {
            return m_stDefaultBlendColor;
        }

        /**
         * 设置默认的混合颜色
         */
        void SetDefaultBlendColor(const Subsystem::Render::Drawing2D::SpriteColorComponents& color) noexcept;

    protected:  // Asset
        [[nodiscard]] Subsystem::Asset::AssetTypeId GetAssetTypeId() const noexcept override;

    private:
        void UpdateResource() noexcept;
        void SyncBlendMode(bool updateVertex = true) noexcept;

    private:
        TextureAssetPtr m_pTextureAsset;
        Subsystem::Render::Drawing2D::Sprite m_stSprite;
        ColliderShape m_stColliderShape;  // 碰撞外形
        BlendMode m_stDefaultBlendMode;  // 默认混合模式
        Subsystem::Render::Drawing2D::SpriteColorComponents m_stDefaultBlendColor = { 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu };
    };

    using SpriteAssetPtr = std::shared_ptr<SpriteAsset>;
}
