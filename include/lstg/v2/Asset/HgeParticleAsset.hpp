/**
 * @file
 * @date 2022/7/14
 * @author 9chu
 * 此文件为 LuaSTGPlus 项目的一部分，版权与许可声明详见 COPYRIGHT.txt。
 */
#pragma once
#include <lstg/Core/Subsystem/Asset/Asset.hpp>
#include <lstg/Core/Subsystem/Render/Drawing2D/ParticleConfig.hpp>
#include "SpriteAsset.hpp"

namespace lstg::v2::Asset
{
    /**
     * HGE 粒子资源
     */
    class HgeParticleAsset :
        public Subsystem::Asset::Asset
    {
        friend class HgeParticleAssetLoader;

    public:
        [[nodiscard]] static Subsystem::Asset::AssetTypeId GetAssetTypeIdStatic() noexcept;

    public:
        HgeParticleAsset(std::string name, std::string path, SpriteAssetPtr spriteAsset, ColliderShape colliderShape);

    public:
        /**
         * 获取资源路径
         */
        [[nodiscard]] const std::string& GetPath() const noexcept { return m_stPath; }

        /**
         * 获取关联的精灵
         */
        [[nodiscard]] const SpriteAssetPtr& GetSpriteAsset() const noexcept { return m_pSpriteAsset; }

        /**
         * 获取粒子配置
         */
        [[nodiscard]] const Subsystem::Render::Drawing2D::ParticleConfig& GetParticleConfig() const noexcept { return m_stParticleConfig; }

        /**
         * 获取碰撞形状
         */
        [[nodiscard]] const ColliderShape& GetColliderShape() const noexcept { return m_stColliderShape; }

        /**
         * 设置碰撞形状
         */
        void SetColliderShape(ColliderShape shape) noexcept { m_stColliderShape = shape; }

    protected:  // Asset
        [[nodiscard]] Subsystem::Asset::AssetTypeId GetAssetTypeId() const noexcept override;

    private:
        void UpdateResource(const Subsystem::Render::Drawing2D::ParticleConfig& config) noexcept;

    private:
        // 资源属性
        const std::string m_stPath;

        // 资产对象实例
        SpriteAssetPtr m_pSpriteAsset;
        Subsystem::Render::Drawing2D::ParticleConfig m_stParticleConfig;
        ColliderShape m_stColliderShape;  // 碰撞外形
    };

    using HgeParticleAssetPtr = std::shared_ptr<HgeParticleAsset>;
}
