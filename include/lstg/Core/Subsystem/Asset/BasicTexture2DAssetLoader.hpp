/**
 * @file
 * @date 2022/5/14
 * @author 9chu
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#pragma once
#include <optional>
#include "AssetLoader.hpp"
#include "../Render/Texture2DData.hpp"
#include "../VFS/IStream.hpp"
#include "../VFS/IFileSystem.hpp"

namespace lstg::Subsystem::Asset
{
    /**
     * 纹理资产加载器
     */
    class BasicTexture2DAssetLoader :
        public AssetLoader
    {
    public:
        BasicTexture2DAssetLoader(AssetPtr asset, bool mipmapEnabled);

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
        // 资源属性
        const bool m_bMipmaps = false;

        // 状态
        VFS::StreamPtr m_pSourceStream;  // PreLoad 时加载
#if LSTG_ASSET_HOT_RELOAD
        VFS::FileAttribute m_stSourceAttribute;
#endif
        std::optional<Render::Texture2DData> m_stTextureData;  // AsyncLoad 时加载
    };
}
