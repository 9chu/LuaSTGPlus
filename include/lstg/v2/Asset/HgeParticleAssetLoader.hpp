/**
 * @file
 * @date 2022/7/14
 * @author 9chu
 * 此文件为 LuaSTGPlus 项目的一部分，版权与许可声明详见 COPYRIGHT.txt。
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
        HgeParticleAssetLoader(Subsystem::Asset::AssetPtr asset,
            std::optional<Subsystem::Render::Drawing2D::ParticleEmitDirection> emitDirectionOverride);

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
        std::optional<Subsystem::Render::Drawing2D::ParticleEmitDirection> m_stEmitDirectionOverride;

        // 状态
        Subsystem::Render::Drawing2D::ParticleConfig m_stParticleConfig;
#if LSTG_ASSET_HOT_RELOAD
        uint32_t m_uLoadedSpriteVersion = 0;
        Subsystem::VFS::FileAttribute m_stSourceAttribute;
#endif
    };
}
