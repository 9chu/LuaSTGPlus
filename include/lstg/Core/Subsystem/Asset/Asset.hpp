/**
 * @file
 * @date 2022/4/24
 * @author 9chu
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#pragma once
#include <string>
#include "../../Result.hpp"

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
     */
    class Asset
    {
        friend class AssetPool;
        friend class lstg::Subsystem::AssetSystem;

    public:
        Asset(std::string name);
        virtual ~Asset() noexcept;

    public:
        /**
         * 获取资产状态
         */
        AssetStates GetState() const noexcept { return m_iState; }

        /**
         * 获取资产名称
         */
        const std::string& GetName() const noexcept { return m_stName; }

        /**
         * 获取关联的资源池
         */
        const std::weak_ptr<AssetPool>& GetPool() const noexcept { return m_pPool; }

    public:  // 需要实现
        /**
         * 获取类型 ID
         */
        virtual AssetTypeId GetAssetTypeId() const noexcept = 0;

    protected:
        /**
         * 设置状态
         * @param s 状态
         */
        void SetState(AssetStates s) noexcept;

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
    };

    using AssetPtr = std::shared_ptr<Asset>;
}
