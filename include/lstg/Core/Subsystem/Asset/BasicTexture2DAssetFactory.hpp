/**
 * @file
 * @date 2022/4/26
 * @author 9chu
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#pragma once
#include "IAssetFactory.hpp"

namespace lstg::Subsystem::Asset
{
    /**
     * 纹理资产工厂
     */
    class BasicTexture2DAssetFactory :
        public IAssetFactory
    {
    public:  // IAssetFactory
        std::string_view GetAssetTypeName() const noexcept override;
        AssetTypeId GetAssetTypeId() const noexcept override;
        Result<CreateAssetResult> CreateAsset(AssetSystem& assetSystem, AssetPoolPtr pool, std::string_view name,
            const nlohmann::json& arguments) noexcept override;
    };
}
