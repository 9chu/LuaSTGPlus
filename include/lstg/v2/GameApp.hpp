/**
 * @file
 * @author 9chu
 * @date 2022/2/16
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#pragma once
#include <lstg/Core/AppBase.hpp>
#include <lstg/Core/Exception.hpp>
#include <lstg/Core/Subsystem/VFS/OverlayFileSystem.hpp>
#include <lstg/Core/Subsystem/Asset/AssetPool.hpp>

namespace lstg::v2
{
    LSTG_DEFINE_EXCEPTION(AppInitializeFailedException);

    /**
     * 游戏程序实现
     */
    class GameApp :
        public AppBase
    {
    public:
        GameApp(int argc, char** argv);

    public:  // 框架控制方法
        /**
         * 加载资源包
         * @param path 路径
         * @param password 密码
         * @return 是否成功
         */
        Result<void> MountAssetPack(const char* path, std::optional<std::string_view> password) noexcept;

        /**
         * 卸载资源包
         * @param path 路径
         * @return 是否成功
         */
        Result<void> UnmountAssetPack(const char* path) noexcept;

    public:  // 资源系统
        /**
         * 获取全局资源池
         */
        const Subsystem::Asset::AssetPoolPtr& GetGlobalAssetPool() const noexcept { return m_pGlobalAssetPool; }

        /**
         * 获取关卡资源池
         */
        const Subsystem::Asset::AssetPoolPtr& GetStageAssetPool() const noexcept { return m_pStageAssetPool; }

        /**
         * 获取当前的资源池
         */
        const Subsystem::Asset::AssetPoolPtr& GetCurrentAssetPool() const noexcept { return m_pCurrentAssetPool; }

        /**
         * 设置当前的资源池
         */
        void SetCurrentAssetPool(const Subsystem::Asset::AssetPoolPtr& pool) noexcept { m_pCurrentAssetPool = pool; }

        /**
         * 寻找资产
         * 依次在 StageAssetPool 和 GlobalAssetPool 中寻找资产。
         * @param name 资产名
         * @return 资产对象，若未找到返回 nullptr
         */
        Subsystem::Asset::AssetPtr FindAsset(std::string_view name) const noexcept;

    protected:  // 框架事件
        void OnEvent(Subsystem::SubsystemEvent& event) noexcept override;
        void OnUpdate(double elapsed) noexcept override;
        void OnRender(double elapsed) noexcept override;

    private:
        std::shared_ptr<Subsystem::VFS::OverlayFileSystem> m_pAssetsFileSystem;

        // 资源池
        Subsystem::Asset::AssetPoolPtr m_pGlobalAssetPool;
        Subsystem::Asset::AssetPoolPtr m_pStageAssetPool;
        Subsystem::Asset::AssetPoolPtr m_pCurrentAssetPool;
    };
}
