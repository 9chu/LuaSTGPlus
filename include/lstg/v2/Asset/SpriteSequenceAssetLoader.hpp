/**
 * @file
 * @date 2022/6/8
 * @author 9chu
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
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
    };
}
