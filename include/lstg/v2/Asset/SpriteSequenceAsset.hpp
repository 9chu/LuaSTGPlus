/**
 * @file
 * @date 2022/6/7
 * @author 9chu
 * 此文件为 LuaSTGPlus 项目的一部分，版权与许可声明详见 COPYRIGHT.txt。
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
    class SpriteSequenceAssetLoader;

    /**
     * 精灵序列资源
     */
    class SpriteSequenceAsset :
        public Subsystem::Asset::Asset
    {
        friend class SpriteSequenceAssetLoader;

    public:
        using SequenceContainer = std::vector<Subsystem::Render::Drawing2D::Sprite>;

        [[nodiscard]] static Subsystem::Asset::AssetTypeId GetAssetTypeIdStatic() noexcept;

    public:
        SpriteSequenceAsset(std::string name, TextureAssetPtr texture, SequenceContainer frames, int32_t interval,
            ColliderShape colliderShape);

    public:
        /**
         * 获取关联的纹理资产
         */
        [[nodiscard]] const TextureAssetPtr& GetTextureAsset() const noexcept { return m_pTextureAsset; }

        /**
         * 获取帧序列
         */
        [[nodiscard]] const SequenceContainer& GetSequences() const noexcept { return m_stSequences; }

        /**
         * 获取帧间隔
         */
        [[nodiscard]] int32_t GetInterval() const noexcept { return m_iInterval; }

        /**
         * 获取锚点
         */
        [[nodiscard]] const glm::vec2& GetAnchor() const noexcept;

        /**
         * 设置锚点
         */
        void SetAnchor(glm::vec2 vec) noexcept;

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
        SequenceContainer m_stSequences;  // 精灵列表
        int32_t m_iInterval = 1;  // 帧时间
        ColliderShape m_stColliderShape;  // 碰撞外形
        BlendMode m_stDefaultBlendMode;  // 默认混合模式
        Subsystem::Render::Drawing2D::SpriteColorComponents m_stDefaultBlendColor = { 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu };
    };

    using SpriteSequenceAssetPtr = std::shared_ptr<SpriteSequenceAsset>;
}
