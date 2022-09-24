/**
 * @file
 * @date 2022/4/26
 * @author 9chu
 * 此文件为 LuaSTGPlus 项目的一部分，版权与许可声明详见 COPYRIGHT.txt。
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
            const nlohmann::json& arguments, IAssetDependencyResolver* resolver) noexcept override;
    };
}
