/**
 * @file
 * @date 2022/7/13
 * @author 9chu
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#pragma once
#include <lstg/Core/Subsystem/Asset/AssetLoader.hpp>
#include <lstg/Core/Subsystem/Asset/BasicTexture2DAsset.hpp>
#include <lstg/Core/Subsystem/VFS/IFileSystem.hpp>
#include <lstg/Core/Subsystem/Render/Font/IFontFactory.hpp>

namespace lstg::v2::Asset
{
    /**
     * HGE 字体加载器
     */
    class HgeFontAssetLoader :
        public Subsystem::Asset::AssetLoader,
        public Subsystem::Render::Font::IFontDependencyLoader
    {
    public:
        HgeFontAssetLoader(Subsystem::Asset::AssetPtr asset, Subsystem::Render::Font::FontFactoryPtr factory);

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

    public:  // IFontDependencyLoader
        virtual Result<Subsystem::Render::TexturePtr> OnLoadTexture(std::string_view path) noexcept override;

    private:
        Subsystem::Render::Font::FontFactoryPtr m_pFontFactory;

        // 状态
        bool m_bWaitForTextureLoaded = false;
        std::string m_stTextureFilename;
        Subsystem::Asset::BasicTexture2DAssetPtr m_pLoadedTexture;
        Subsystem::Render::Font::FontFacePtr m_pLoadedFontFace;
#if LSTG_ASSET_HOT_RELOAD
        uint32_t m_uLoadedTextureVersion = 0;
        Subsystem::VFS::FileAttribute m_stSourceAttribute;
#endif
    };
}
