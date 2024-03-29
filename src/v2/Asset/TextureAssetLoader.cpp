/**
 * @file
 * @date 2022/5/30
 * @author 9chu
 * 此文件为 LuaSTGPlus 项目的一部分，版权与许可声明详见 COPYRIGHT.txt。
 */
#include <lstg/v2/Asset/TextureAssetLoader.hpp>

#include <lstg/Core/Subsystem/Asset/AssetError.hpp>
#include <lstg/v2/Asset/TextureAsset.hpp>

using namespace std;
using namespace lstg;
using namespace lstg::v2::Asset;

TextureAssetLoader::TextureAssetLoader(Subsystem::Asset::AssetPtr asset)
    : Subsystem::Asset::AssetLoader(std::move(asset))
{
    auto texAsset = static_pointer_cast<TextureAsset>(GetAsset());

    if (texAsset->IsRenderTarget())
    {
        // RT 不需要加载过程
        SetState(Subsystem::Asset::AssetLoadingStates::Loaded);
    }
    else
    {
        // 需要等待依赖的纹理加载完毕
        SetState(Subsystem::Asset::AssetLoadingStates::DependencyLoading);
    }
}

Result<void> TextureAssetLoader::PreLoad() noexcept
{
    assert(false);
    SetState(Subsystem::Asset::AssetLoadingStates::Error);
    return make_error_code(Subsystem::Asset::AssetError::InvalidState);
}

Result<void> TextureAssetLoader::AsyncLoad() noexcept
{
    assert(false);
    SetState(Subsystem::Asset::AssetLoadingStates::Error);
    return make_error_code(Subsystem::Asset::AssetError::InvalidState);
}

Result<void> TextureAssetLoader::PostLoad() noexcept
{
    assert(false);
    SetState(Subsystem::Asset::AssetLoadingStates::Error);
    return make_error_code(Subsystem::Asset::AssetError::InvalidState);
}

void TextureAssetLoader::Update() noexcept
{
    auto asset = static_pointer_cast<TextureAsset>(GetAsset());
    if (asset->IsRenderTarget())
        return;

    // 检查依赖的资源是否加载完毕
    auto state = asset->GetBasicTextureAsset()->GetState();
    if (state == Subsystem::Asset::AssetStates::Loaded)
    {
#if LSTG_ASSET_HOT_RELOAD
        m_uLastTextureVersion = asset->GetBasicTextureAsset()->GetVersion();
#endif
        asset->UpdateResource();

        SetState(Subsystem::Asset::AssetLoadingStates::Loaded);
    }
    else if (state == Subsystem::Asset::AssetStates::Error)
    {
        SetState(Subsystem::Asset::AssetLoadingStates::Error);
    }
}

#if LSTG_ASSET_HOT_RELOAD
bool TextureAssetLoader::SupportHotReload() const noexcept
{
    auto asset = static_pointer_cast<TextureAsset>(GetAsset());
    return !asset->IsRenderTarget();
}

bool TextureAssetLoader::CheckIsOutdated() const noexcept
{
    auto asset = static_pointer_cast<TextureAsset>(GetAsset());
    assert(!asset->IsRenderTarget());
    return asset->GetBasicTextureAsset()->GetVersion() != m_uLastTextureVersion;
}

void TextureAssetLoader::PrepareToReload() noexcept
{
    auto asset = static_pointer_cast<TextureAsset>(GetAsset());
    assert(!asset->IsRenderTarget());
    SetState(Subsystem::Asset::AssetLoadingStates::DependencyLoading);
}
#endif
