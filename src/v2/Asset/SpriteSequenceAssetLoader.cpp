/**
* @file
* @date 2022/6/8
* @author 9chu
* 此文件为 LuaSTGPlus 项目的一部分，版权与许可声明详见 COPYRIGHT.txt。
*/
#include <lstg/v2/Asset/SpriteSequenceAssetLoader.hpp>

#include <lstg/Core/Subsystem/Asset/AssetError.hpp>
#include <lstg/v2/Asset/SpriteSequenceAsset.hpp>

using namespace std;
using namespace lstg;
using namespace lstg::v2::Asset;

SpriteSequenceAssetLoader::SpriteSequenceAssetLoader(Subsystem::Asset::AssetPtr asset)
    : Subsystem::Asset::AssetLoader(std::move(asset))
{
    // 需要等待依赖的纹理加载完毕
    SetState(Subsystem::Asset::AssetLoadingStates::DependencyLoading);
}

Result<void> SpriteSequenceAssetLoader::PreLoad() noexcept
{
    assert(false);
    SetState(Subsystem::Asset::AssetLoadingStates::Error);
    return make_error_code(Subsystem::Asset::AssetError::InvalidState);
}

Result<void> SpriteSequenceAssetLoader::AsyncLoad() noexcept
{
    assert(false);
    SetState(Subsystem::Asset::AssetLoadingStates::Error);
    return make_error_code(Subsystem::Asset::AssetError::InvalidState);
}

Result<void> SpriteSequenceAssetLoader::PostLoad() noexcept
{
    assert(false);
    SetState(Subsystem::Asset::AssetLoadingStates::Error);
    return make_error_code(Subsystem::Asset::AssetError::InvalidState);
}

void SpriteSequenceAssetLoader::Update() noexcept
{
    auto asset = static_pointer_cast<SpriteSequenceAsset>(GetAsset());

    // 检查依赖的资源是否加载完毕
    auto state = asset->GetTextureAsset()->GetState();
    if (state == Subsystem::Asset::AssetStates::Loaded)
    {
#if LSTG_ASSET_HOT_RELOAD
        m_uLastTextureVersion = asset->GetTextureAsset()->GetVersion();
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
bool SpriteSequenceAssetLoader::SupportHotReload() const noexcept
{
    return true;
}

bool SpriteSequenceAssetLoader::CheckIsOutdated() const noexcept
{
    auto asset = static_pointer_cast<SpriteSequenceAsset>(GetAsset());
    return asset->GetTextureAsset()->GetVersion() != m_uLastTextureVersion;
}

void SpriteSequenceAssetLoader::PrepareToReload() noexcept
{
    SetState(Subsystem::Asset::AssetLoadingStates::DependencyLoading);
}
#endif
