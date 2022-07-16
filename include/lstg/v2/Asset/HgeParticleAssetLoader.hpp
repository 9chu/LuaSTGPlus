/**
 * @file
 * @date 2022/7/14
 * @author 9chu
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#pragma once
#include <lstg/Core/Subsystem/Asset/AssetLoader.hpp>
#include <lstg/Core/Subsystem/Render/Drawing2D/ParticleConfig.hpp>
#include <lstg/Core/Subsystem/VFS/IFileSystem.hpp>

namespace lstg::v2::Asset
{
    /**
     * HGE 粒子系统加载器
     */
    class HgeParticleAssetLoader :
        public Subsystem::Asset::AssetLoader
    {
    public:
        HgeParticleAssetLoader(Subsystem::Asset::AssetPtr asset);

    public:  // AssetLoader
        Result<void> PreLoad() noexcept override;
        Result<void> AsyncLoad() noexcept override;
        Result<void> PostLoad() noexcept override;
        void Update() noexcept override;

#if LSTG_ASSET_HOT_RELOAD
        bool SupportHotReload() const noexcept override;
        bool CheckIsOutdated() const noexcept override;
        void PrepareToReload() noexcept override;
#endif

    private:
        // 状态
        Subsystem::Render::Drawing2D::ParticleConfig m_stParticleConfig;
#if LSTG_ASSET_HOT_RELOAD
        uint32_t m_uLoadedSpriteVersion = 0;
        Subsystem::VFS::FileAttribute m_stSourceAttribute;
#endif
    };
}
