/**
 * @file
 * @date 2022/7/13
 * @author 9chu
 * 此文件为 LuaSTGPlus 项目的一部分，版权与许可声明详见 COPYRIGHT.txt。
 */
#pragma once
#include <lstg/Core/Subsystem/Asset/IAssetFactory.hpp>
#include <lstg/Core/Subsystem/Render/Font/IFontFactory.hpp>

namespace lstg::v2::Asset
{
    /**
     * HGE 字体资产工厂
     */
    class HgeFontAssetFactory :
        public Subsystem::Asset::IAssetFactory
    {
    public:
        HgeFontAssetFactory();

    public:  // IAssetFactory
        std::string_view GetAssetTypeName() const noexcept override;
        Subsystem::Asset::AssetTypeId GetAssetTypeId() const noexcept override;
        Result<Subsystem::Asset::CreateAssetResult> CreateAsset(Subsystem::AssetSystem& assetSystem, Subsystem::Asset::AssetPoolPtr pool,
            std::string_view name, const nlohmann::json& arguments, Subsystem::Asset::IAssetDependencyResolver* resolver) noexcept override;

    private:
        Subsystem::Render::Font::FontFactoryPtr m_pHGEFontFactory;
    };
}
