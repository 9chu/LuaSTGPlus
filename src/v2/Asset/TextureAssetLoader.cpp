/**
 * @file
 * @date 2022/5/30
 * @author 9chu
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
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
    // 需要等待依赖的纹理加载完毕
    SetState(Subsystem::Asset::AssetLoadingStates::DependencyLoading);
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

    // 检查依赖的资源是否加载完毕
    auto state = asset->GetBasicTexture()->GetState();
    if (state == Subsystem::Asset::AssetStates::Loaded)
        SetState(Subsystem::Asset::AssetLoadingStates::Loaded);
    else if (state == Subsystem::Asset::AssetStates::Error)
        SetState(Subsystem::Asset::AssetLoadingStates::Error);
}
