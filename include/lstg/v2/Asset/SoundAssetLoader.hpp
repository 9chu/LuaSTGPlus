/**
 * @file
 * @date 2022/9/17
 * @author 9chu
 * 此文件为 LuaSTGPlus 项目的一部分，版权与许可声明详见 COPYRIGHT.txt。
 */
#pragma once
#include <lstg/Core/Subsystem/Asset/AssetLoader.hpp>
#include <lstg/Core/Subsystem/VFS/IFileSystem.hpp>
#include <lstg/Core/Subsystem/Audio/ISoundData.hpp>

namespace lstg::v2::Asset
{
    /**
     * 音频资源加载器
     */
    class SoundAssetLoader :
        public Subsystem::Asset::AssetLoader
    {
    public:
        SoundAssetLoader(Subsystem::Asset::AssetPtr asset);

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
        Subsystem::VFS::StreamPtr m_pSourceStream;  // PreLoad 时加载
#if LSTG_ASSET_HOT_RELOAD
        Subsystem::VFS::FileAttribute m_stSourceAttribute;
#endif
        Subsystem::Audio::SoundDataPtr m_pSoundData;  // AsyncLoad 时加载
    };
}
