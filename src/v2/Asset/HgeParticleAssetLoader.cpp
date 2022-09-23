/**
 * @file
 * @date 2022/7/15
 * @author 9chu
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#include <lstg/v2/Asset/HgeParticleAssetLoader.hpp>

#include <lstg/Core/Logging.hpp>
#include <lstg/Core/Subsystem/AssetSystem.hpp>
#include <lstg/Core/Subsystem/Asset/AssetError.hpp>
#include <lstg/v2/Asset/HgeParticleAsset.hpp>

using namespace std;
using namespace lstg;
using namespace lstg::v2::Asset;

using namespace lstg::Subsystem;
using namespace lstg::Subsystem::Asset;

LSTG_DEF_LOG_CATEGORY(HgeParticleAssetLoader);

HgeParticleAssetLoader::HgeParticleAssetLoader(Subsystem::Asset::AssetPtr asset,
    std::optional<Subsystem::Render::Drawing2D::ParticleEmitDirection> emitDirectionOverride)
    : Subsystem::Asset::AssetLoader(std::move(asset)), m_stEmitDirectionOverride(emitDirectionOverride)
{
    // 需要等待依赖的精灵加载完毕
    SetState(AssetLoadingStates::DependencyLoading);
}

Result<void> HgeParticleAssetLoader::PreLoad() noexcept
{
    assert(GetState() == AssetLoadingStates::Pending);

    // 检查是否释放了
    auto asset = static_pointer_cast<HgeParticleAsset>(GetAsset());
    if (asset->IsWildAsset())
    {
        SetState(AssetLoadingStates::Error);
        return make_error_code(AssetError::LoadingCancelled);
    }

#if LSTG_ASSET_HOT_RELOAD
    auto attribute = AssetSystem::GetInstance().GetAssetStreamAttribute(asset->GetPath());
    if (!attribute)
    {
        LSTG_LOG_ERROR_CAT(HgeParticleAssetLoader, "Get asset stream attribute from \"{}\" fail: {}", asset->GetPath(),
            attribute.GetError());
    }
    else
    {
        m_stSourceAttribute = *attribute;
    }
#endif

    // 在主线程打开文件流
    auto stream = AssetSystem::GetInstance().OpenAssetStream(asset->GetPath());
    if (!stream)
    {
        LSTG_LOG_ERROR_CAT(HgeParticleAssetLoader, "Open asset stream from \"{}\" fail: {}", asset->GetPath(), stream.GetError());
        SetState(AssetLoadingStates::Error);
        return stream.GetError();
    }

    // 直接读取配置
    m_stParticleConfig = {};
    auto ret = m_stParticleConfig.ReadFromHGE(stream->get());
    if (!ret)
    {
        LSTG_LOG_ERROR_CAT(HgeParticleAssetLoader, "Read hge particle from stream \"{}\" fail: {}", asset->GetPath(), ret.GetError());
        SetState(AssetLoadingStates::Error);
        return ret.GetError();
    }
    if (m_stEmitDirectionOverride)
        m_stParticleConfig.EmitDirection = *m_stEmitDirectionOverride;

    // 提交资源
    asset->UpdateResource(m_stParticleConfig);

    SetState(AssetLoadingStates::Loaded);
    return {};
}

Result<void> HgeParticleAssetLoader::AsyncLoad() noexcept
{
    assert(false);
    SetState(AssetLoadingStates::Error);
    return make_error_code(AssetError::InvalidState);
}

Result<void> HgeParticleAssetLoader::PostLoad() noexcept
{
    assert(false);
    SetState(AssetLoadingStates::Error);
    return make_error_code(AssetError::InvalidState);
}

void HgeParticleAssetLoader::Update() noexcept
{
    auto asset = static_pointer_cast<HgeParticleAsset>(GetAsset());

    // 检查依赖的资源是否加载完毕
    auto state = asset->GetSpriteAsset()->GetState();
    if (state == AssetStates::Loaded)
    {
#if LSTG_ASSET_HOT_RELOAD
        m_uLoadedSpriteVersion = asset->GetSpriteAsset()->GetVersion();
#endif

        // 准备加载
        SetState(AssetLoadingStates::Pending);
    }
    else if (state == AssetStates::Error)
    {
        SetState(AssetLoadingStates::Error);
    }
}

#if LSTG_ASSET_HOT_RELOAD
bool HgeParticleAssetLoader::SupportHotReload() const noexcept
{
    return true;
}

bool HgeParticleAssetLoader::CheckIsOutdated() const noexcept
{
    assert(GetState() == AssetLoadingStates::Loaded || GetState() == AssetLoadingStates::Error);

    auto asset = static_pointer_cast<HgeParticleAsset>(GetAsset());

    // 检查依赖的精灵是否过期
    if (asset->GetSpriteAsset()->GetVersion() != m_uLoadedSpriteVersion)
        return true;

    // 获取文件属性
    auto attr = AssetSystem::GetInstance().GetAssetStreamAttribute(asset->GetPath());
    if (!attr)
    {
        LSTG_LOG_WARN_CAT(HgeParticleAssetLoader, "Get stream attribute from \"{}\" fail: {}", asset->GetPath(), attr.GetError());
        return false;
    }

    // 检查是否发生修改
    return attr->LastModified != m_stSourceAttribute.LastModified;
}

void HgeParticleAssetLoader::PrepareToReload() noexcept
{
    assert(GetState() == AssetLoadingStates::Loaded || GetState() == AssetLoadingStates::Error);

    SetState(Subsystem::Asset::AssetLoadingStates::DependencyLoading);
}
#endif
