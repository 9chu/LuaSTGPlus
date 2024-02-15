/**
 * @file
 * @date 2022/9/17
 * @author 9chu
 * 此文件为 LuaSTGPlus 项目的一部分，版权与许可声明详见 COPYRIGHT.txt。
 */
#include <lstg/v2/Asset/MusicAssetLoader.hpp>

#include <lstg/Core/Logging.hpp>
#include <lstg/Core/Subsystem/AssetSystem.hpp>
#include <lstg/v2/Asset/MusicAsset.hpp>

using namespace std;
using namespace lstg;
using namespace lstg::v2::Asset;

using namespace lstg::Subsystem;
using namespace lstg::Subsystem::Asset;

LSTG_DEF_LOG_CATEGORY(MusicAssetLoader);

MusicAssetLoader::MusicAssetLoader(Subsystem::Asset::AssetPtr asset)
    : Subsystem::Asset::AssetLoader(std::move(asset))
{
    // 没有依赖
    SetState(AssetLoadingStates::Pending);
}

Result<void> MusicAssetLoader::PreLoad() noexcept
{
    assert(GetState() == AssetLoadingStates::Pending);

    // 检查是否释放了
    auto asset = static_pointer_cast<MusicAsset>(GetAsset());
    if (asset->IsWildAsset())
    {
        SetState(AssetLoadingStates::Error);
        return make_error_code(AssetError::LoadingCancelled);
    }

    // 在主线程打开文件流
    auto stream = AssetSystem::GetInstance().OpenAssetStream(asset->GetPath());
    if (!stream)
    {
        LSTG_LOG_ERROR_CAT(MusicAssetLoader, "Open asset stream from \"{}\" fail: {}", asset->GetPath(), stream.GetError().value());
        SetState(AssetLoadingStates::Error);
        return stream.GetError();
    }
#if LSTG_ASSET_HOT_RELOAD
    auto attribute = AssetSystem::GetInstance().GetAssetStreamAttribute(asset->GetPath());
    if (!attribute)
        LSTG_LOG_ERROR_CAT(MusicAssetLoader, "Get asset stream attribute from \"{}\" fail: {}", asset->GetPath(), attribute.GetError());
    else
        m_stSourceAttribute = *attribute;  // 时间刷新需要前置防止加载失败时反复重试
#endif

    // 创建基于流的音频源不会花费 CPU，无需异步加载
    auto ret = Audio::CreateStreamSoundData(std::move(*stream));
    if (!ret)
    {
        LSTG_LOG_ERROR_CAT(MusicAssetLoader, "Load music data from \"{}\" fail: {}", asset->GetPath(), ret.GetError().value());
        SetState(AssetLoadingStates::Error);
        return ret.GetError();
    }

    // 提交资源
    asset->UpdateResource(std::move(*ret));

    SetState(AssetLoadingStates::Loaded);
    return {};
}

Result<void> MusicAssetLoader::AsyncLoad() noexcept
{
    assert(false);
    SetState(Subsystem::Asset::AssetLoadingStates::Error);
    return make_error_code(Subsystem::Asset::AssetError::InvalidState);
}

Result<void> MusicAssetLoader::PostLoad() noexcept
{
    assert(false);
    SetState(Subsystem::Asset::AssetLoadingStates::Error);
    return make_error_code(Subsystem::Asset::AssetError::InvalidState);
}

void MusicAssetLoader::Update() noexcept
{
}

#if LSTG_ASSET_HOT_RELOAD
bool MusicAssetLoader::SupportHotReload() const noexcept
{
    return true;
}

bool MusicAssetLoader::CheckIsOutdated() const noexcept
{
    assert(GetState() == AssetLoadingStates::Loaded || GetState() == AssetLoadingStates::Error);

    // 获取文件属性
    auto asset = static_pointer_cast<MusicAsset>(GetAsset());
    auto attr = AssetSystem::GetInstance().GetAssetStreamAttribute(asset->GetPath());
    if (!attr)
    {
        LSTG_LOG_WARN_CAT(MusicAssetLoader, "Get stream attribute from \"{}\" fail: {}", asset->GetPath(), attr.GetError());
        return false;
    }

    // 检查是否发生修改
    return attr->LastModified != m_stSourceAttribute.LastModified;
}

void MusicAssetLoader::PrepareToReload() noexcept
{
    assert(GetState() == AssetLoadingStates::Loaded || GetState() == AssetLoadingStates::Error);

    // 开始重新加载
    SetState(AssetLoadingStates::Pending);
}
#endif
