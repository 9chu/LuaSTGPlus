/**
 * @file
 * @date 2022/7/16
 * @author 9chu
 * 此文件为 LuaSTGPlus 项目的一部分，版权与许可声明详见 COPYRIGHT.txt。
 */
#pragma once
#include <lstg/Core/Subsystem/Asset/AssetLoader.hpp>
#include <lstg/Core/Subsystem/VFS/IFileSystem.hpp>

namespace lstg::v2::Asset
{
    /**
     * FX 加载器
     */
    class EffectAssetLoader :
        public Subsystem::Asset::AssetLoader
    {
    public:
        EffectAssetLoader(Subsystem::Asset::AssetPtr asset);

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
#if LSTG_ASSET_HOT_RELOAD
        Subsystem::VFS::FileAttribute m_stSourceAttribute;
#endif
    };
}
