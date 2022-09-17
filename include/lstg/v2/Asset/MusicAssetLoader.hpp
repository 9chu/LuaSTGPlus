/**
 * @file
 * @date 2022/9/17
 * @author 9chu
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#pragma once
#include <lstg/Core/Subsystem/Asset/AssetLoader.hpp>
#include <lstg/Core/Subsystem/VFS/IFileSystem.hpp>
#include <lstg/Core/Subsystem/Audio/ISoundData.hpp>

namespace lstg::v2::Asset
{
    /**
     * 音乐资源加载器
     */
    class MusicAssetLoader :
        public Subsystem::Asset::AssetLoader
    {
    public:
        MusicAssetLoader(Subsystem::Asset::AssetPtr asset);

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
#if LSTG_ASSET_HOT_RELOAD
        Subsystem::VFS::FileAttribute m_stSourceAttribute;
#endif
    };
}
