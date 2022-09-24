/**
 * @file
 * @date 2022/6/8
 * @author 9chu
 * 此文件为 LuaSTGPlus 项目的一部分，版权与许可声明详见 COPYRIGHT.txt。
 */
#pragma once
#include <lstg/Core/Subsystem/Asset/AssetLoader.hpp>

namespace lstg::v2::Asset
{
    /**
     * 精灵资产加载器
     */
    class SpriteSequenceAssetLoader :
        public Subsystem::Asset::AssetLoader
    {
    public:
        SpriteSequenceAssetLoader(Subsystem::Asset::AssetPtr asset);

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
        uint32_t m_uLastTextureVersion = 0;
#endif
    };
}
