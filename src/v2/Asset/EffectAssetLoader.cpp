/**
 * @file
 * @date 2022/7/16
 * @author 9chu
 * 此文件为 LuaSTGPlus 项目的一部分，版权与许可声明详见 COPYRIGHT.txt。
 */
#include <lstg/v2/Asset/EffectAssetLoader.hpp>

#include <lstg/Core/Logging.hpp>
#include <lstg/Core/Subsystem/AssetSystem.hpp>
#include <lstg/v2/Asset/EffectAsset.hpp>

using namespace std;
using namespace lstg;
using namespace lstg::v2::Asset;

using namespace lstg::Subsystem;
using namespace lstg::Subsystem::Asset;

LSTG_DEF_LOG_CATEGORY(EffectAssetLoader);

EffectAssetLoader::EffectAssetLoader(Subsystem::Asset::AssetPtr asset)
    : Subsystem::Asset::AssetLoader(std::move(asset))
{
    // 没有依赖
    SetState(AssetLoadingStates::Pending);
}

Result<void> EffectAssetLoader::PreLoad() noexcept
{
    assert(GetState() == AssetLoadingStates::Pending);

    // 检查是否释放了
    auto asset = static_pointer_cast<EffectAsset>(GetAsset());
    if (asset->IsWildAsset())
    {
        SetState(AssetLoadingStates::Error);
        return make_error_code(AssetError::LoadingCancelled);
    }

#if LSTG_ASSET_HOT_RELOAD
    auto attribute = AssetSystem::GetInstance().GetAssetStreamAttribute(asset->GetPath());
    if (!attribute)
        LSTG_LOG_ERROR_CAT(EffectAssetLoader, "Get asset stream attribute from \"{}\" fail: {}", asset->GetPath(), attribute.GetError());
    else
        m_stSourceAttribute = *attribute;  // 时间刷新需要前置防止加载失败时反复重试
#endif

    // 获取 RenderSystem
    auto& renderSystem = AssetSystem::GetInstance().GetRenderSystem();

    // 创建 Effect
    auto effect = renderSystem.GetEffectFactory()->CreateEffectFromFile(asset->GetPath());
    if (!effect)
    {
        LSTG_LOG_ERROR_CAT(EffectAssetLoader, "Create effect from \"{}\" fail: {}", asset->GetPath(), effect.GetError());
        SetState(AssetLoadingStates::Error);
        return effect.GetError();
    }

    // 创建 Material
    auto material = renderSystem.CreateMaterial(*effect);
    if (!material)
    {
        LSTG_LOG_ERROR_CAT(EffectAssetLoader, "Create material from \"{}\" fail: {}", asset->GetPath(), effect.GetError());
        SetState(AssetLoadingStates::Error);
        return effect.GetError();
    }

    // 提交资源
    asset->UpdateResource(std::move(*effect), std::move(*material));

    SetState(AssetLoadingStates::Loaded);
    return {};
}

Result<void> EffectAssetLoader::AsyncLoad() noexcept
{
    assert(false);
    SetState(Subsystem::Asset::AssetLoadingStates::Error);
    return make_error_code(Subsystem::Asset::AssetError::InvalidState);
}

Result<void> EffectAssetLoader::PostLoad() noexcept
{
    assert(false);
    SetState(Subsystem::Asset::AssetLoadingStates::Error);
    return make_error_code(Subsystem::Asset::AssetError::InvalidState);
}

void EffectAssetLoader::Update() noexcept
{
}

#if LSTG_ASSET_HOT_RELOAD
bool EffectAssetLoader::SupportHotReload() const noexcept
{
    return true;
}

bool EffectAssetLoader::CheckIsOutdated() const noexcept
{
    assert(GetState() == AssetLoadingStates::Loaded || GetState() == AssetLoadingStates::Error);

    auto asset = static_pointer_cast<EffectAsset>(GetAsset());

    // 获取文件属性
    auto attr = AssetSystem::GetInstance().GetAssetStreamAttribute(asset->GetPath());
    if (!attr)
    {
        LSTG_LOG_WARN_CAT(EffectAssetLoader, "Get stream attribute from \"{}\" fail: {}", asset->GetPath(), attr.GetError());
        return false;
    }

    // 检查是否发生修改
    return attr->LastModified != m_stSourceAttribute.LastModified;
}

void EffectAssetLoader::PrepareToReload() noexcept
{
    assert(GetState() == AssetLoadingStates::Loaded || GetState() == AssetLoadingStates::Error);

    // 开始重新加载
    SetState(AssetLoadingStates::Pending);
}
#endif
