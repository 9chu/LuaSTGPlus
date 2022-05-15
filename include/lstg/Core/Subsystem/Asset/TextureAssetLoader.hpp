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

namespace lstg::Subsystem::Asset
{
    /**
     * 纹理资产加载器
     */
    class TextureAssetLoader :
        public AssetLoader
    {
    public:
        TextureAssetLoader(AssetPtr asset, bool mipmapEnabled);

    public:  // AssetLoader
        Result<void> PreLoad() noexcept override;
        Result<void> AsyncLoad() noexcept override;
        Result<void> PostLoad() noexcept override;
        void Update() noexcept override;

    private:
        // 资源属性
        const bool m_bMipmaps = false;

        // 状态
        VFS::StreamPtr m_pSourceStream;  // PreLoad 时加载
        std::optional<Render::Texture2DData> m_stTextureData;  // AsyncLoad 时加载
    };
}
