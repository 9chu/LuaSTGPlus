/**
 * @file
 * @date 2022/4/26
 * @author 9chu
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#pragma once
#include <memory>
#include <nlohmann/json.hpp>
#include "AssetPool.hpp"
#include "AssetLoader.hpp"

namespace lstg::Subsystem::Asset
{
    using CreateAssetResult = std::tuple<AssetPtr, AssetLoaderPtr>;

    /**
     * 资产解析器
     * 用于寻找引用资产
     */
    class IAssetDependencyResolver
    {
    public:
        IAssetDependencyResolver() noexcept = default;
        virtual ~IAssetDependencyResolver() = default;

    public:
        /**
         * 当查找资产时调用
         * @param name 名称
         * @return 依赖的对象
         */
        virtual AssetPtr OnResolveAsset(std::string_view name) const noexcept = 0;
    };

    /**
     * 资产工厂
     */
    class IAssetFactory
    {
    public:
        IAssetFactory() = default;
        virtual ~IAssetFactory() noexcept = default;

    public:
        /**
         * 获取资源标识名
         */
        [[nodiscard]] virtual std::string_view GetAssetTypeName() const noexcept = 0;

        /**
         * 获取资源标识ID
         */
        [[nodiscard]] virtual AssetTypeId GetAssetTypeId() const noexcept = 0;

        /**
         * 创建资产
         * 创建资产时可能会在关联的资源池上创建其他具名的或者无名的子资源。
         * @param assetSystem 资产系统
         * @param pool 关联的资源池
         * @param name 资源名称
         * @param arguments 参数
         * @param resolver 资产解析器
         * @return 返回 <资产对象, 加载器对象>。当加载器对象为空时，我们认为资源总是同步加载完毕。
         */
        [[nodiscard]] virtual Result<CreateAssetResult> CreateAsset(AssetSystem& assetSystem, AssetPoolPtr pool, std::string_view name,
            const nlohmann::json& arguments, IAssetDependencyResolver* resolver) noexcept = 0;
    };

    using AssetFactoryPtr = std::shared_ptr<IAssetFactory>;
}
