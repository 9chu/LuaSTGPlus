/**
 * @file
 * @date 2022/4/24
 * @author 9chu
 * 此文件为 LuaSTGPlus 项目的一部分，版权与许可声明详见 COPYRIGHT.txt。
 */
#pragma once
#include <string>
#include <memory>
#include "../../Result.hpp"

#ifndef LSTG_ASSET_HOT_RELOAD
#ifdef LSTG_DEVELOPMENT
#define LSTG_ASSET_HOT_RELOAD 1
#else
#define LSTG_ASSET_HOT_RELOAD 0
#endif
#endif

namespace lstg::Subsystem
{
    class AssetSystem;
}

namespace lstg::Subsystem::Asset
{
    class AssetPool;

    /**
     * 资产状态
     */
    enum class AssetStates
    {
        Uninitialized,
        Error,
        Loaded,
    };

    using AssetId = size_t;
    using AssetTypeId = size_t;

    constexpr const AssetId kEmptyAssetId = static_cast<AssetId>(-1);

    /**
     * 资产基类
     *
     * - 资产总是位于一个 Pool 中，并且具备 Pool 中的唯一 ID。
     * - 资产可以没有名称，此时不能由名称查询资产，名称也是 Pool 中唯一的。
     * - 资产可以持有其他资产的引用，此时称为“引用资产”。
     * - 资产也可以依赖其他资产，此时被依赖的资产称为“子资产”。
     * - 资产需要主动释放“子资产”，否则会造成泄漏。
     * - “子资产”通常没有名字。
     */
    class Asset
    {
        friend class AssetPool;
        friend class lstg::Subsystem::AssetSystem;

    public:
        Asset(std::string name);
        Asset(const Asset&) = delete;
        Asset(Asset&&) noexcept = delete;
        virtual ~Asset() noexcept;

        Asset& operator=(const Asset&) = delete;
        Asset& operator=(Asset&&) noexcept = delete;

    public:
        /**
         * 获取资产 ID
         */
        [[nodiscard]] AssetId GetId() const noexcept { return m_uId; }

        /**
         * 获取资产状态
         */
        [[nodiscard]] AssetStates GetState() const noexcept { return m_iState; }

        /**
         * 获取资产名称
         */
        [[nodiscard]] const std::string& GetName() const noexcept { return m_stName; }

        /**
         * 获取关联的资源池
         */
        [[nodiscard]] const std::weak_ptr<AssetPool>& GetPool() const noexcept { return m_pPool; }

        /**
         * 是否是空悬资产
         * 指示资产被从 AssetPool 释放。
         * 当资产处于空悬状态时，对应的 Loader 将会终止并释放 Asset。
         */
        [[nodiscard]] bool IsWildAsset() const noexcept;

#if LSTG_ASSET_HOT_RELOAD
        /**
         * 获取资产版本号
         */
        [[nodiscard]] uint32_t GetVersion() const noexcept { return m_uVersion; }
#endif

    public:  // 需要实现
        /**
         * 获取类型 ID
         */
        [[nodiscard]] virtual AssetTypeId GetAssetTypeId() const noexcept = 0;

    protected:
        /**
         * 设置状态
         * @param s 状态
         */
        void SetState(AssetStates s) noexcept;

#if LSTG_ASSET_HOT_RELOAD
        /**
         * 更新版本号
         */
        void UpdateVersion() noexcept;
#endif

    protected:  // 需要实现
        /**
         * 当从 Pool 中移除时调用
         * 用于删除资产关联的子资产
         * 默认无行为
         * @note 不能在 OnRemove 过程中创建资产，可能引发崩溃
         */
        virtual void OnRemove() noexcept;

    private:
        AssetId m_uId = kEmptyAssetId;
        AssetStates m_iState = AssetStates::Uninitialized;
        std::string m_stName;
        std::weak_ptr<AssetPool> m_pPool;
#if LSTG_ASSET_HOT_RELOAD
        uint32_t m_uVersion = 0;
#endif
    };

    using AssetPtr = std::shared_ptr<Asset>;
}
