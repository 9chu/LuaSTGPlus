/**
 * @file
 * @author 9chu
 * @date 2022/7/16
 * 此文件为 LuaSTGPlus 项目的一部分，版权与许可声明详见 COPYRIGHT.txt。
 */
#pragma once
#include <lstg/Core/Subsystem/Asset/AssetPool.hpp>
#include <lstg/Core/Subsystem/Asset/IAssetFactory.hpp>
#include "AssetNaming.hpp"

namespace lstg::v2
{
    /**
     * 资源池类型
     */
    enum class AssetPoolTypes
    {
        None,
        Global,
        Stage,
    };

    /**
     * 资源池子
     *
     * lstg 存在两个资源池：关卡资源池和全局资源池。
     * 资源的查找总是先从 StagePool 开始，然后查找 GlobalPool。
     */
    class AssetPools :
        public Subsystem::Asset::IAssetDependencyResolver
    {
    public:
        AssetPools();

    public:
        /**
         * 获取全局资源池
         */
        [[nodiscard]] const Subsystem::Asset::AssetPoolPtr& GetGlobalAssetPool() const noexcept { return m_pGlobalAssetPool; }

        /**
         * 获取关卡资源池
         */
        [[nodiscard]] const Subsystem::Asset::AssetPoolPtr& GetStageAssetPool() const noexcept { return m_pStageAssetPool; }

        /**
         * 获取当前的资产池
         */
        [[nodiscard]] std::tuple<AssetPoolTypes, Subsystem::Asset::AssetPoolPtr> GetCurrentAssetPool() const noexcept;

        /**
         * 设置当前的资产池
         */
        void SetCurrentAssetPool(AssetPoolTypes t) noexcept;

        /**
         * 寻找资产
         * 依次在 StageAssetPool 和 GlobalAssetPool 中寻找资产。
         * @param type 资产类型
         * @param name 资产名
         * @return 资产对象，若未找到返回 nullptr
         */
        [[nodiscard]] Subsystem::Asset::AssetPtr FindAsset(AssetTypes type, std::string_view name) const noexcept;

        /**
         * 定位资产池
         * @param type 资产类型
         * @param name 资产名
         * @return 资产位于哪个池子，如果返回 None 表示没有找到
         */
        [[nodiscard]] std::tuple<AssetPoolTypes, Subsystem::Asset::AssetPoolPtr> LocateAsset(AssetTypes type,
            std::string_view name) const noexcept;

        /**
         * 移除资产
         * @param pool 池子
         * @param type 资产类型
         * @param name 资产名
         * @return 是否成功，如果没有找到资产则返回 false
         */
        bool RemoveAsset(AssetPoolTypes pool, AssetTypes type, std::string_view name) noexcept;

        /**
         * 清空资源池
         * @param pool 池子
         */
        void ClearAssetPool(AssetPoolTypes pool) noexcept;

    protected:  // IAssetDependencyResolver
        [[nodiscard]] Subsystem::Asset::AssetPtr OnResolveAsset(std::string_view name) const noexcept override;

    private:
        Subsystem::Asset::AssetPoolPtr m_pGlobalAssetPool;
        Subsystem::Asset::AssetPoolPtr m_pStageAssetPool;
        Subsystem::Asset::AssetPool* m_pCurrentAssetPool = nullptr;

        mutable std::string m_stTmpNameBuffer;
    };

    using AssetPoolsPtr = std::unique_ptr<AssetPools>;
}
