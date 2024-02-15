/**
 * @file
 * @date 2022/9/17
 * @author 9chu
 * 此文件为 LuaSTGPlus 项目的一部分，版权与许可声明详见 COPYRIGHT.txt。
 */
#include <lstg/v2/Asset/SoundAssetLoader.hpp>

#include <lstg/Core/Logging.hpp>
#include <lstg/Core/Subsystem/AssetSystem.hpp>
#include <lstg/v2/Asset/SoundAsset.hpp>

using namespace std;
using namespace lstg;
using namespace lstg::v2::Asset;

using namespace lstg::Subsystem;
using namespace lstg::Subsystem::Asset;

LSTG_DEF_LOG_CATEGORY(SoundAssetLoader);

SoundAssetLoader::SoundAssetLoader(Subsystem::Asset::AssetPtr asset)
    : Subsystem::Asset::AssetLoader(std::move(asset))
{
    // 没有依赖
    SetState(AssetLoadingStates::Pending);
}

Result<void> SoundAssetLoader::PreLoad() noexcept
{
    assert(GetState() == AssetLoadingStates::Pending);

    // 检查是否释放了
    auto asset = static_pointer_cast<SoundAsset>(GetAsset());
    if (asset->IsWildAsset())
    {
        SetState(AssetLoadingStates::Error);
        return make_error_code(AssetError::LoadingCancelled);
    }

    // 在主线程打开文件流
    auto stream = AssetSystem::GetInstance().OpenAssetStream(asset->GetPath());
    if (!stream)
    {
        LSTG_LOG_ERROR_CAT(SoundAssetLoader, "Open asset stream from \"{}\" fail: {}", asset->GetPath(), stream.GetError().value());
        SetState(AssetLoadingStates::Error);
        return stream.GetError();
    }
#if LSTG_ASSET_HOT_RELOAD
    auto attribute = AssetSystem::GetInstance().GetAssetStreamAttribute(asset->GetPath());
    if (!attribute)
        LSTG_LOG_ERROR_CAT(SoundAssetLoader, "Get asset stream attribute from \"{}\" fail: {}", asset->GetPath(), attribute.GetError());
    else
        m_stSourceAttribute = *attribute;  // 时间刷新需要前置防止加载失败时反复重试
#endif

    m_pSourceStream = std::move(*stream);
    SetState(AssetLoadingStates::Preloaded);
    return {};
}

Result<void> SoundAssetLoader::AsyncLoad() noexcept
{
    assert(GetState() == AssetLoadingStates::AsyncLoadCommitted);

    // 在加载线程读取文件并解码
    auto asset = static_pointer_cast<SoundAsset>(GetAsset());
    auto ret = Audio::CreateMemorySoundData(std::move(m_pSourceStream));  // Stream 在使用后自动关闭
    if (!ret)
    {
        LSTG_LOG_ERROR_CAT(SoundAssetLoader, "Load sound data from \"{}\" fail: {}", asset->GetPath(), ret.GetError().value());
        SetState(AssetLoadingStates::Error);
        return ret.GetError();
    }

    m_pSoundData = std::move(*ret);
    SetState(AssetLoadingStates::AsyncLoaded);
    return {};
}

Result<void> SoundAssetLoader::PostLoad() noexcept
{
    assert(GetState() == AssetLoadingStates::AsyncLoaded);

    // 接受数据
    auto asset = static_pointer_cast<SoundAsset>(GetAsset());
    asset->UpdateResource(std::move(m_pSoundData));

    SetState(AssetLoadingStates::Loaded);
    return {};
}

void SoundAssetLoader::Update() noexcept
{
}

#if LSTG_ASSET_HOT_RELOAD
bool SoundAssetLoader::SupportHotReload() const noexcept
{
    return true;
}

bool SoundAssetLoader::CheckIsOutdated() const noexcept
{
    assert(GetState() == AssetLoadingStates::Loaded || GetState() == AssetLoadingStates::Error);

    // 获取文件属性
    auto asset = static_pointer_cast<SoundAsset>(GetAsset());
    auto attr = AssetSystem::GetInstance().GetAssetStreamAttribute(asset->GetPath());
    if (!attr)
    {
        LSTG_LOG_WARN_CAT(SoundAssetLoader, "Get stream attribute from \"{}\" fail: {}", asset->GetPath(), attr.GetError());
        return false;
    }

    // 检查是否发生修改
    return attr->LastModified != m_stSourceAttribute.LastModified;
}

void SoundAssetLoader::PrepareToReload() noexcept
{
    assert(GetState() == AssetLoadingStates::Loaded || GetState() == AssetLoadingStates::Error);

    // 开始重新加载
    SetState(AssetLoadingStates::Pending);
}
#endif
