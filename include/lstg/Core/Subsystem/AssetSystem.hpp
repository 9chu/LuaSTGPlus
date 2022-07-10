/**
 * @file
 * @date 2022/4/24
 * @author 9chu
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#pragma once
#include "VirtualFileSystem.hpp"
#include "RenderSystem.hpp"
#include "VFS/IStream.hpp"
#include "VFS/IFileSystem.hpp"
#include "Asset/IAssetFactory.hpp"
#include "Asset/AssetError.hpp"
#include "Asset/AssetPool.hpp"
#include "../ThreadPool.hpp"

namespace lstg::Subsystem
{
    /**
     * 资产系统
     */
    class AssetSystem :
        public ISubsystem
    {
    public:
        /**
         * 获取全局实例
         */
        static AssetSystem& GetInstance() noexcept;

    public:
        AssetSystem(SubsystemContainer& container);
        ~AssetSystem() override;

    public:
        /**
         * 获取虚拟文件系统
         */
        VirtualFileSystem& GetVirtualFileSystem() const noexcept { return *m_pVirtualFileSystem; }

        /**
         * 获取渲染系统
         */
        RenderSystem& GetRenderSystem() const noexcept { return *m_pRenderSystem; }

        /**
         * 异步加载是否启用
         */
        bool IsAsyncLoadingEnabled() const noexcept { return m_bAsyncLoadingEnabled; }

        /**
         * 设置是否启用异步加载
         * @param v 是否启用
         */
        void SetAsyncLoadingEnabled(bool v) noexcept { m_bAsyncLoadingEnabled = v; }

        /**
         * 打开资产流
         * @param path 路径
         * @return 流指针
         */
        Result<VFS::StreamPtr> OpenAssetStream(std::string_view path) noexcept;

        /**
         * 获取资产流属性
         * @param path 路径
         * @return 属性
         */
        Result<VFS::FileAttribute> GetAssetStreamAttribute(std::string_view path) noexcept;

        /**
         * 注册资产工厂
         */
        Result<void> RegisterAssetFactory(Asset::AssetFactoryPtr factory) noexcept;

        /**
         * 通过资源类型名和参数创建资产
         * @param pool 资产池
         * @param typeName 资源类型名
         * @param name 资产名称
         * @param arguments 构造参数
         * @return 资产对象
         */
        Result<Asset::AssetPtr> CreateAsset(Asset::AssetPoolPtr pool, std::string_view typeName, std::string_view name,
            const nlohmann::json& arguments) noexcept;

        /**
         * 通过资产类型和参数创建资产
         * @tparam T 资产类型
         * @param pool 资产池
         * @param name 资产名称
         * @param arguments 构造参数
         * @return 资产对象
         */
        template <typename T>
        Result<std::shared_ptr<T>> CreateAsset(Asset::AssetPoolPtr pool, std::string_view name, const nlohmann::json& arguments) noexcept
        {
            const auto& uniqueTypeName = Script::detail::GetUniqueTypeName<T>();
            auto id = uniqueTypeName.Id;

            auto it = m_stAssetFactories.find(id);
            if (it == m_stAssetFactories.end())
                return make_error_code(Asset::AssetError::AssetFactoryNotRegistered);

            auto ret = CreateAsset(pool, it->second, name, arguments);
            if (!ret)
                return ret.GetError();
            return std::static_pointer_cast<T>(*ret);
        }

    protected:  // ISubsystem
        void OnUpdate(double elapsedTime) noexcept override;

    private:
        void RegisterCoreAssetFactories();
        Result<Asset::AssetPtr> CreateAsset(Asset::AssetPoolPtr pool, Asset::AssetFactoryPtr factory, std::string_view name,
            const nlohmann::json& arguments) noexcept;
        void BlockUntilLoadingFinished() noexcept;
        Result<void> CommitAsyncLoadTask(Asset::AssetLoaderPtr loader) noexcept;

    private:
        std::shared_ptr<VirtualFileSystem> m_pVirtualFileSystem;
        std::shared_ptr<RenderSystem> m_pRenderSystem;

        // 配置
        bool m_bAsyncLoadingEnabled = true;

        // 资产工厂
        std::unordered_map<Asset::AssetTypeId, Asset::AssetFactoryPtr> m_stAssetFactories;
        std::map<std::string, Asset::AssetTypeId, std::less<>> m_stAssetFactoryLookupTable;

        // 资源异步加载线程
        ThreadPool<> m_stAsyncLoadingThread;

        // 加载队列
        std::vector<Asset::AssetLoaderPtr> m_stLoadingTasks;
#if LSTG_ASSET_HOT_RELOAD
        size_t m_uLastCheckedTask = 0;
        std::vector<Asset::AssetLoaderPtr> m_stWatchTasks;
#endif
    };
}
