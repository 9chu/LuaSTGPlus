/**
 * @file
 * @date 2022/7/10
 * @author 9chu
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#include <lstg/v2/Asset/TrueTypeFontAssetLoader.hpp>

#include <lstg/Core/Logging.hpp>
#include <lstg/Core/Subsystem/AssetSystem.hpp>
#include <lstg/Core/Subsystem/Asset/AssetError.hpp>
#include <lstg/v2/Asset/TrueTypeFontAsset.hpp>

using namespace std;
using namespace lstg;
using namespace lstg::v2::Asset;

using namespace lstg::Subsystem;
using namespace lstg::Subsystem::Asset;

LSTG_DEF_LOG_CATEGORY(TrueTypeFontAssetLoader);

TrueTypeFontAssetLoader::TrueTypeFontAssetLoader(AssetPtr asset, Render::Font::FontFactoryPtr factory)
    : AssetLoader(std::move(asset)), m_pFontFactory(std::move(factory))
{
    assert(m_pFontFactory);

    // 没有依赖的资源，直接转到 Pending 状态
    SetState(AssetLoadingStates::Pending);
}

Result<void> TrueTypeFontAssetLoader::PreLoad() noexcept
{
    assert(GetState() == AssetLoadingStates::Pending);

    // 检查是否释放了
    auto asset = static_pointer_cast<TrueTypeFontAsset>(GetAsset());
    if (asset->IsWildAsset())
    {
        SetState(AssetLoadingStates::Error);
        return make_error_code(AssetError::LoadingCancelled);
    }

#if LSTG_ASSET_HOT_RELOAD
    auto attribute = AssetSystem::GetInstance().GetAssetStreamAttribute(asset->GetPath());
    if (!attribute)
        LSTG_LOG_ERROR_CAT(TrueTypeFontAssetLoader, "Get asset stream attribute from \"{}\" fail: {}", asset->GetPath(), attribute.GetError());
    else
        m_stSourceAttribute = *attribute;
#endif

    // 在主线程打开文件流
    auto stream = AssetSystem::GetInstance().OpenAssetStream(asset->GetPath());
    if (!stream)
    {
        LSTG_LOG_ERROR_CAT(TrueTypeFontAssetLoader, "Open asset stream from \"{}\" fail: {}", asset->GetPath(), stream.GetError());
        SetState(AssetLoadingStates::Error);
        return stream.GetError();
    }

    // FreeTypeLibrary 原则上不能跨线程使用，需要上锁
    // 故我们直接在主线程加载，不使用异步过程

    // FontDependency 在加载 TTF 时不需要，FaceIndex 目前固定取 0
    auto face = m_pFontFactory->CreateFontFace(std::move(*stream), nullptr, 0);
    if (!face)
    {
        LSTG_LOG_ERROR_CAT(TrueTypeFontAssetLoader, "Load TrueType font from \"{}\" fail: {}", asset->GetPath(), face.GetError());
        SetState(AssetLoadingStates::Error);
        return face.GetError();
    }
    m_pLoadedFontFace = std::move(*face);

    // 创建 FontCollection
    // 目前 API 设计下，不支持字体 fallback
    Render::Font::FontCollectionPtr collection;
    try
    {
        collection = make_shared<Render::Font::FontCollection>(m_pLoadedFontFace);
    }
    catch (...)  // bad_alloc
    {
        m_pLoadedFontFace.reset();
        return make_error_code(errc::not_enough_memory);
    }

    // 提交资源
    asset->UpdateResource(std::move(m_pLoadedFontFace), std::move(collection));

    SetState(AssetLoadingStates::Loaded);
    return {};
}

Result<void> TrueTypeFontAssetLoader::AsyncLoad() noexcept
{
    assert(false);
    SetState(Subsystem::Asset::AssetLoadingStates::Error);
    return make_error_code(Subsystem::Asset::AssetError::InvalidState);
}

Result<void> TrueTypeFontAssetLoader::PostLoad() noexcept
{
    assert(false);
    SetState(Subsystem::Asset::AssetLoadingStates::Error);
    return make_error_code(Subsystem::Asset::AssetError::InvalidState);
}

void TrueTypeFontAssetLoader::Update() noexcept
{
}

#if LSTG_ASSET_HOT_RELOAD
bool TrueTypeFontAssetLoader::SupportHotReload() const noexcept
{
    return true;
}

bool TrueTypeFontAssetLoader::CheckIsOutdated() const noexcept
{
    assert(GetState() == AssetLoadingStates::Loaded || GetState() == AssetLoadingStates::Error);

    // 获取文件属性
    auto asset = static_pointer_cast<TrueTypeFontAsset>(GetAsset());
    auto attr = AssetSystem::GetInstance().GetAssetStreamAttribute(asset->GetPath());
    if (!attr)
    {
        LSTG_LOG_WARN_CAT(TrueTypeFontAssetLoader, "Get stream attribute from \"{}\" fail: {}", asset->GetPath(), attr.GetError());
        return false;
    }

    // 检查是否发生修改
    return attr->LastModified != m_stSourceAttribute.LastModified;
}

void TrueTypeFontAssetLoader::PrepareToReload() noexcept
{
    assert(GetState() == AssetLoadingStates::Loaded || GetState() == AssetLoadingStates::Error);

    // 转到 Pending 状态，准备重新加载
    SetState(AssetLoadingStates::Pending);
}
#endif
