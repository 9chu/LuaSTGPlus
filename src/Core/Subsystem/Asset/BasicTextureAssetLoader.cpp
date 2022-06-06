/**
 * @file
 * @date 2022/5/14
 * @author 9chu
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#include <lstg/Core/Subsystem/Asset/BasicTextureAssetLoader.hpp>

#include <lstg/Core/Logging.hpp>
#include <lstg/Core/Subsystem/AssetSystem.hpp>
#include <lstg/Core/Subsystem/RenderSystem.hpp>
#include <lstg/Core/Subsystem/Asset/BasicTextureAsset.hpp>
#include <lstg/Core/Subsystem/Asset/AssetError.hpp>
#include "../Render/detail/Texture2DDataImpl.hpp"
#include "detail/WeakPtrTraits.hpp"

using namespace std;
using namespace lstg;
using namespace lstg::Subsystem::Asset;

LSTG_DEF_LOG_CATEGORY(BasicTextureAssetLoader);

BasicTextureAssetLoader::BasicTextureAssetLoader(AssetPtr asset, bool mipmapEnabled)
    : AssetLoader(std::move(asset)), m_bMipmaps(mipmapEnabled)
{
    // 没有依赖的资源，直接转到 Pending 状态
    SetState(AssetLoadingStates::Pending);
}

Result<void> BasicTextureAssetLoader::PreLoad() noexcept
{
    assert(GetState() == AssetLoadingStates::Pending);

    // 检查是否释放了
    auto asset = static_pointer_cast<BasicTextureAsset>(GetAsset());
    if (detail::IsWeakPtrUninitialized(asset->GetPool()))
    {
        SetState(AssetLoadingStates::Error);
        return make_error_code(AssetError::LoadingCancelled);
    }

    // 在主线程打开文件流
    auto stream = AssetSystem::GetInstance().OpenAssetStream(asset->GetPath());
    if (!stream)
    {
        LSTG_LOG_ERROR_CAT(BasicTextureAssetLoader, "Open asset stream from \"{}\" fail: {}", asset->GetPath(), stream.GetError());
        SetState(AssetLoadingStates::Error);
        return stream.GetError();
    }

    m_pSourceStream = std::move(*stream);
    SetState(AssetLoadingStates::Preloaded);
    return {};
}

Result<void> BasicTextureAssetLoader::AsyncLoad() noexcept
{
    assert(GetState() == AssetLoadingStates::AsyncLoadCommitted);

    // 在加载线程读取并解码纹理
    auto asset = static_pointer_cast<BasicTextureAsset>(GetAsset());
    try
    {
        m_stTextureData.emplace(std::move(m_pSourceStream));
        if (m_bMipmaps)
            m_stTextureData->GenerateMipmap();
    }
    catch (const std::system_error& ex)
    {
        LSTG_LOG_ERROR_CAT(BasicTextureAssetLoader, "Load image data from \"{}\" fail: {}", asset->GetPath(), ex.what());
        SetState(AssetLoadingStates::Error);
        return ex.code();
    }
    catch (...)  // bad_alloc
    {
        SetState(AssetLoadingStates::Error);
        return make_error_code(errc::not_enough_memory);
    }

    SetState(AssetLoadingStates::AsyncLoaded);
    return {};
}

Result<void> BasicTextureAssetLoader::PostLoad() noexcept
{
    assert(GetState() == AssetLoadingStates::AsyncLoaded);

    // 调用 RenderSystem 创建纹理
    auto asset = static_pointer_cast<BasicTextureAsset>(GetAsset());
    auto& renderSystem = AssetSystem::GetInstance().GetRenderSystem();
    auto tex = renderSystem.CreateTexture2D(*m_stTextureData);
    if (!tex)
    {
        LSTG_LOG_ERROR_CAT(BasicTextureAssetLoader, "Create texture from \"{}\" fail: {}", asset->GetPath(), tex.GetError());
        SetState(AssetLoadingStates::Error);
        return tex.GetError();
    }

    // 接受数据
    // 资源对象的状态变换由 AssetSystem 处理
    asset->ReceiveLoadedAsset(std::move(*tex));

    SetState(AssetLoadingStates::Loaded);
    return {};
}

void BasicTextureAssetLoader::Update() noexcept
{
}
