/**
 * @file
 * @date 2022/5/12
 * @author 9chu
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#pragma once
#include "Asset.hpp"

namespace lstg::Subsystem::Asset
{
    /**
     * 资产加载状态
     */
    enum class AssetLoadingStates
    {
        Uninitialized,  // 尚未初始化
        DependencyLoading,  // 等待依赖的资源，此时不能进行 PreLoad
        Pending,  // 等待加载，此时可以执行 PreLoad
        Preloaded,  // 预加载完成
        AsyncLoadCommitted,  // 已提交异步加载队列，此时可以执行 AsyncLoad
        Loading,  // 加载中，此时应该在执行 AsyncLoad
        AsyncLoaded,  // 异步加载完成，此时可以执行 PostLoad
        Loaded,  // 加载完毕
        Error,  // 加载过程失败
    };

    /**
     * 资产加载器
     */
    class AssetLoader
    {
        friend class Subsystem::AssetSystem;

    public:
        explicit AssetLoader(AssetPtr asset);
        virtual ~AssetLoader() noexcept = default;

    public:
        /**
         * 获取资产加载状态
         */
        AssetLoadingStates GetState() const noexcept;

        /**
         * 获取关联的资产对象
         * @note 需要线程安全
         */
        const AssetPtr& GetAsset() const noexcept { return m_pAsset; }

    public:  // 需要实现
        /**
         * 预加载
         * @note 总是在主线程上调用
         */
        virtual Result<void> PreLoad() noexcept = 0;

        /**
         * 加载
         * @note 视情况，在主线程或者资源加载线程调用
         */
        virtual Result<void> AsyncLoad() noexcept = 0;

        /**
         * 后加载
         * @note 总是在主线程上调用
         */
        virtual Result<void> PostLoad() noexcept = 0;

        /**
         * 更新内部状态
         * @note 总是在主线程上调用
         */
        virtual void Update() noexcept = 0;

    protected:
        void SetState(AssetLoadingStates state) noexcept;

    private:
        std::atomic<AssetLoadingStates> m_iState;
        AssetPtr m_pAsset;
    };

    using AssetLoaderPtr = std::shared_ptr<AssetLoader>;
}
