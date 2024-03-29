/**
 * @file
 * @date 2022/7/16
 * @author 9chu
 * 此文件为 LuaSTGPlus 项目的一部分，版权与许可声明详见 COPYRIGHT.txt。
 */
#pragma once
#include <lstg/Core/Subsystem/Render/GraphDef/EffectDefinition.hpp>
#include <lstg/Core/Subsystem/Render/Material.hpp>
#include <lstg/Core/Subsystem/Asset/Asset.hpp>

namespace lstg::v2::Asset
{
    class EffectAssetLoader;

    /**
     * FX 资源
     */
    class EffectAsset :
        public Subsystem::Asset::Asset
    {
        friend class EffectAssetLoader;

    public:
        [[nodiscard]] static Subsystem::Asset::AssetTypeId GetAssetTypeIdStatic() noexcept;

    public:
        EffectAsset(std::string name, std::string path);

    public:
        /**
         * 获取路径
         */
        [[nodiscard]] const std::string& GetPath() const noexcept { return m_stPath; }

        /**
         * 获取 FX 对象
         */
        [[nodiscard]] const Subsystem::Render::GraphDef::ImmutableEffectDefinitionPtr& GetEffect() const noexcept { return m_pEffectDef; }

        /**
         * 获取默认材质实例
         */
        [[nodiscard]] const Subsystem::Render::MaterialPtr& GetDefaultMaterial() const noexcept { return m_pDefaultMaterial; }

    protected:  // Asset
        [[nodiscard]] Subsystem::Asset::AssetTypeId GetAssetTypeId() const noexcept override;

    private:
        void UpdateResource(Subsystem::Render::GraphDef::ImmutableEffectDefinitionPtr def, Subsystem::Render::MaterialPtr mat) noexcept;

    private:
        const std::string m_stPath;
        Subsystem::Render::GraphDef::ImmutableEffectDefinitionPtr m_pEffectDef;
        Subsystem::Render::MaterialPtr m_pDefaultMaterial;
    };

    using EffectAssetPtr = std::shared_ptr<EffectAsset>;
}
