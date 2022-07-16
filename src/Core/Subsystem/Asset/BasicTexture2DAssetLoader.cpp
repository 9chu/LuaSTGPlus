/**
 * @file
 * @date 2022/5/14
 * @author 9chu
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#include <lstg/Core/Subsystem/Asset/BasicTexture2DAssetLoader.hpp>

#include <lstg/Core/Logging.hpp>
#include <lstg/Core/Subsystem/AssetSystem.hpp>
#include <lstg/Core/Subsystem/RenderSystem.hpp>
#include <lstg/Core/Subsystem/Asset/BasicTexture2DAsset.hpp>
#include <lstg/Core/Subsystem/Asset/AssetError.hpp>
#include "../Render/detail/Texture2DDataImpl.hpp"
#include "detail/WeakPtrTraits.hpp"

using namespace std;
using namespace lstg;
using namespace lstg::Subsystem::Asset;

LSTG_DEF_LOG_CATEGORY(BasicTexture2DAssetLoader);

BasicTexture2DAssetLoader::BasicTexture2DAssetLoader(AssetPtr asset, bool mipmapEnabled)
    : AssetLoader(std::move(asset)), m_bMipmaps(mipmapEnabled)
{
    // 没有依赖的资源，直接转到 Pending 状态
    SetState(AssetLoadingStates::Pending);
}

Result<void> BasicTexture2DAssetLoader::PreLoad() noexcept
{
    assert(GetState() == AssetLoadingStates::Pending);

    // 检查是否释放了
    auto asset = static_pointer_cast<BasicTexture2DAsset>(GetAsset());
    if (asset->IsWildAsset())
    {
        SetState(AssetLoadingStates::Error);
        return make_error_code(AssetError::LoadingCancelled);
    }

    // 在主线程打开文件流
    auto stream = AssetSystem::GetInstance().OpenAssetStream(asset->GetPath());
    if (!stream)
    {
        LSTG_LOG_ERROR_CAT(BasicTexture2DAssetLoader, "Open asset stream from \"{}\" fail: {}", asset->GetPath(), stream.GetError());
        SetState(AssetLoadingStates::Error);
        return stream.GetError();
    }
#if LSTG_ASSET_HOT_RELOAD
    auto attribute = AssetSystem::GetInstance().GetAssetStreamAttribute(asset->GetPath());
    if (!attribute)
    {
        LSTG_LOG_ERROR_CAT(BasicTexture2DAssetLoader, "Get asset stream attribute from \"{}\" fail: {}", asset->GetPath(),
            attribute.GetError());
    }
    else
    {
        m_stSourceAttribute = *attribute;
    }
#endif

    m_pSourceStream = std::move(*stream);
    SetState(AssetLoadingStates::Preloaded);
    return {};
}

Result<void> BasicTexture2DAssetLoader::AsyncLoad() noexcept
{
    assert(GetState() == AssetLoadingStates::AsyncLoadCommitted);

    // 在加载线程读取并解码纹理
    auto asset = static_pointer_cast<BasicTexture2DAsset>(GetAsset());
    try
    {
        m_stTextureData.emplace(std::move(m_pSourceStream));  // Stream 在使用后自动关闭
        if (m_bMipmaps)
            m_stTextureData->GenerateMipmap();
    }
    catch (const std::system_error& ex)
    {
        LSTG_LOG_ERROR_CAT(BasicTexture2DAssetLoader, "Load image data from \"{}\" fail: {}", asset->GetPath(), ex.what());
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

Result<void> BasicTexture2DAssetLoader::PostLoad() noexcept
{
    assert(GetState() == AssetLoadingStates::AsyncLoaded);

    // 调用 RenderSystem 创建纹理
    auto asset = static_pointer_cast<BasicTexture2DAsset>(GetAsset());
    auto& renderSystem = AssetSystem::GetInstance().GetRenderSystem();
    auto tex = renderSystem.CreateTexture2D(*m_stTextureData);
    if (!tex)
    {
        LSTG_LOG_ERROR_CAT(BasicTexture2DAssetLoader, "Create texture from \"{}\" fail: {}", asset->GetPath(), tex.GetError());
        SetState(AssetLoadingStates::Error);
        return tex.GetError();
    }

    // 接受数据
    // 资源对象的状态变换由 AssetSystem 处理
    asset->ReceiveLoadedAsset(std::move(*tex));

    SetState(AssetLoadingStates::Loaded);
    return {};
}

void BasicTexture2DAssetLoader::Update() noexcept
{
}

#if LSTG_ASSET_HOT_RELOAD
bool BasicTexture2DAssetLoader::SupportHotReload() const noexcept
{
    return true;
}

bool BasicTexture2DAssetLoader::CheckIsOutdated() const noexcept
{
    assert(GetState() == AssetLoadingStates::Loaded || GetState() == AssetLoadingStates::Error);

    // 获取文件属性
    auto asset = static_pointer_cast<BasicTexture2DAsset>(GetAsset());
    auto attr = AssetSystem::GetInstance().GetAssetStreamAttribute(asset->GetPath());
    if (!attr)
    {
        LSTG_LOG_WARN_CAT(BasicTexture2DAssetLoader, "Get stream attribute from \"{}\" fail: {}", asset->GetPath(), attr.GetError());
        return false;
    }

    // 检查是否发生修改
    return attr->LastModified != m_stSourceAttribute.LastModified;
}

void BasicTexture2DAssetLoader::PrepareToReload() noexcept
{
    assert(GetState() == AssetLoadingStates::Loaded || GetState() == AssetLoadingStates::Error);

    // 转到 Pending 状态，准备重新加载
    SetState(AssetLoadingStates::Pending);
}
#endif
