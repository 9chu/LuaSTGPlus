/**
 * @file
 * @date 2022/4/24
 * @author 9chu
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#pragma once
#include <string>
#include "../../Result.hpp"

namespace lstg::Subsystem::Asset
{
    /**
     * 资产加载状态
     */
    enum class AssetLoadingStates
    {
        DependencyLoading,  // 等待依赖的资源
        Pending,  // 等待加载
        Loading,  // 加载中
        AsyncLoaded,  // 异步加载完成
        Loaded,  // 加载完毕
        Error,  // 加载失败
    };

    /**
     * 资产基类
     */
    class IAsset
    {
    public:
        virtual ~IAsset() noexcept = default;

    public:
        /**
         * 获取资产名称
         */
        virtual const std::string& GetName() const noexcept = 0;



        /**
         * 获取加载状态
         */
        virtual AssetLoadingStates GetLoadingState() const noexcept = 0;

        /**
         * 更新状态
         * 仅对非就绪状态资源触发。
         */
        virtual void Update() noexcept = 0;

        /**
         * 前加载资源
         * 用于在异步资源加载前进行准备工作，例如打开需要的文件。
         * @return 是否成功
         */
        virtual Result<void> PreLoad() noexcept = 0;

        /**
         * 加载资源
         * @note 方法可能在工作线程调用，需要保证线程安全性。
         * @return 是否成功
         */
        virtual Result<void> Load() noexcept = 0;

        /**
         * 后加载资源
         * 用于完成资源加载的收尾工作。
         * @note 方法总是在 Load 之后，在主线程执行。
         * @return 是否成功
         */
        virtual Result<void> PostLoad() noexcept = 0;
    };

    using AssetPtr = std::shared_ptr<IAsset>;
}
