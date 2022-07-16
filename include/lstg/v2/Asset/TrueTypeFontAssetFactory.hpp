/**
 * @file
 * @date 2022/7/10
 * @author 9chu
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#pragma once
#include <lstg/Core/Subsystem/Asset/IAssetFactory.hpp>
#include <lstg/Core/Subsystem/Render/Font/IFontFactory.hpp>

namespace lstg::v2::Asset
{
    /**
     * TrueType 字体资产工厂
     */
    class TrueTypeFontAssetFactory :
        public Subsystem::Asset::IAssetFactory
    {
    public:
        TrueTypeFontAssetFactory();

    public:  // IAssetFactory
        std::string_view GetAssetTypeName() const noexcept override;
        Subsystem::Asset::AssetTypeId GetAssetTypeId() const noexcept override;
        Result<Subsystem::Asset::CreateAssetResult> CreateAsset(Subsystem::AssetSystem& assetSystem, Subsystem::Asset::AssetPoolPtr pool,
            std::string_view name, const nlohmann::json& arguments, Subsystem::Asset::IAssetDependencyResolver* resolver) noexcept override;

    private:
        Subsystem::Render::Font::FontFactoryPtr m_pTTFFontFactory;
    };
}
