/**
 * @file
 * @date 2022/5/8
 * @author 9chu
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#pragma once
#include <map>
#include "Asset.hpp"

namespace lstg::Subsystem::Asset
{
    /**
     * 资产池
     */
    class AssetPool :
        public std::enable_shared_from_this<AssetPool>
    {
    public:
        /**
         * 资产清理类型
         */
        enum class AssetClearTypes
        {
            All,
            Unused,
        };

    public:
        AssetPool() = default;
        AssetPool(const AssetPool&) = delete;
        AssetPool(AssetPool&&) = delete;
        ~AssetPool();

    public:
        /**
         * 增加资产
         * @param asset 资产指针
         */
        Result<void> AddAsset(AssetPtr asset) noexcept;

        /**
         * 查询资产是否存在
         * @param id 资产ID
         * @return 是否存在
         */
        bool ContainsAsset(AssetId id) noexcept;

        /**
         * 查询资产是否存在
         * @param name 资产名
         * @return 是否存在
         */
        bool ContainsAsset(std::string_view name) noexcept;

        /**
         * 删除资产
         * @param id 资产ID
         */
        Result<void> RemoveAsset(AssetId id) noexcept;

        /**
         * 删除资产
         * @param name 资产名
         */
        Result<void> RemoveAsset(std::string_view name) noexcept;

        /**
         * 清除所有资产
         */
        void Clear(AssetClearTypes type = AssetClearTypes::All) noexcept;

        /**
         * 获取资产对象
         * @param name 资产名
         * @return 资产对象，如果没有找到则返回 nullptr
         */
        AssetPtr GetAsset(std::string_view name) const noexcept;

        /**
         * 获取资产对象
         * @param id 资产ID
         * @return 资产对象，如果没有找到则返回 nullptr
         */
        AssetPtr GetAsset(AssetId id) const noexcept;

        /**
         * 遍历所有资产
         * @tparam T 遍历回调类型，传入 AssetPtr，返回 [是否终止, 自定义返回内容]
         * @param visitor 遍历回调
         * @return 遍历返回值
         */
        template <typename T>
        auto Visit(T visitor) -> std::remove_cv_t<std::remove_reference_t<decltype(std::get<1>(std::declval<T>()(AssetPtr {})))>>
        {
            for (const auto& asset : m_stAssets)
            {
                auto [stop, result] = visitor(asset.second);
                if (stop)
                    return result;
            }
            return {};
        }

    private:
        size_t m_iNextAssetId = 1;
        std::map<size_t, AssetPtr> m_stAssets;
        std::map<std::string, size_t, std::less<>> m_stLookupTable;
    };

    using AssetPoolPtr = std::shared_ptr<AssetPool>;
}
