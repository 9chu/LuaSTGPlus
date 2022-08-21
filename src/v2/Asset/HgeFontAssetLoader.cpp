/**
 * @file
 * @date 2022/7/13
 * @author 9chu
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#include <lstg/v2/Asset/HgeFontAssetLoader.hpp>

#include <lstg/Core/Logging.hpp>
#include <lstg/Core/Subsystem/VFS/Path.hpp>
#include <lstg/Core/Subsystem/AssetSystem.hpp>
#include <lstg/Core/Subsystem/Asset/AssetError.hpp>
#include <lstg/v2/Asset/HgeFontAsset.hpp>
#include "../../Core/Subsystem/Render/Font/HgeFontFace.hpp"

using namespace std;
using namespace lstg;
using namespace lstg::v2::Asset;

using namespace lstg::Subsystem;
using namespace lstg::Subsystem::Asset;

LSTG_DEF_LOG_CATEGORY(HgeFontAssetLoader);

HgeFontAssetLoader::HgeFontAssetLoader(AssetPtr asset, Render::Font::FontFactoryPtr factory)
    : AssetLoader(std::move(asset)), m_pFontFactory(std::move(factory))
{
    assert(m_pFontFactory);

    // 需要解析并加载关联的纹理，转入 DependencyLoading 状态
    m_bWaitForTextureLoaded = false;
    SetState(AssetLoadingStates::DependencyLoading);
}

Result<void> HgeFontAssetLoader::PreLoad() noexcept
{
    assert(false);
    SetState(Subsystem::Asset::AssetLoadingStates::Error);
    return make_error_code(Subsystem::Asset::AssetError::InvalidState);
}

Result<void> HgeFontAssetLoader::AsyncLoad() noexcept
{
    assert(false);
    SetState(Subsystem::Asset::AssetLoadingStates::Error);
    return make_error_code(Subsystem::Asset::AssetError::InvalidState);
}

Result<void> HgeFontAssetLoader::PostLoad() noexcept
{
    assert(false);
    SetState(Subsystem::Asset::AssetLoadingStates::Error);
    return make_error_code(Subsystem::Asset::AssetError::InvalidState);
}

void HgeFontAssetLoader::Update() noexcept
{
    // HGE 字体的加载过程较为麻烦，因此放在 Update 过程中直接处理
    if (GetState() == AssetLoadingStates::DependencyLoading)
    {
        auto asset = static_pointer_cast<HgeFontAsset>(GetAsset());
        assert(!asset->IsWildAsset());
        auto pool = asset->GetPool().lock();
        assert(pool);

        // 在这里读取文件
        if (!m_bWaitForTextureLoaded)
        {
#if LSTG_ASSET_HOT_RELOAD
            auto attribute = AssetSystem::GetInstance().GetAssetStreamAttribute(asset->GetPath());
            if (!attribute)
            {
                LSTG_LOG_ERROR_CAT(HgeFontAssetLoader, "Get asset stream attribute from \"{}\" fail: {}", asset->GetPath(),
                                   attribute.GetError());
            }
            else
            {
                m_stSourceAttribute = *attribute;
            }
#endif

            // 加载字体文件
            auto stream = AssetSystem::GetInstance().OpenAssetStream(asset->GetPath());
            if (!stream)
            {
                LSTG_LOG_ERROR_CAT(HgeFontAssetLoader, "Open asset stream from \"{}\" fail: {}", asset->GetPath(), stream.GetError());
                SetState(AssetLoadingStates::Error);
                return;
            }

            // 加载字体
            m_stTextureFilename.clear();
            auto face = m_pFontFactory->CreateFontFace(*stream, this, 0);
            if (!face)
            {
                LSTG_LOG_ERROR_CAT(HgeFontAssetLoader, "Load HGE font from \"{}\" fail: {}", asset->GetPath(), face.GetError());
                SetState(AssetLoadingStates::Error);
                return;
            }
            m_pLoadedFontFace = std::move(*face);

            // 加载纹理
            try
            {
                // 生成纹理路径
                auto texPath = VFS::Path(asset->GetPath());
                texPath = texPath.GetParent() / VFS::Path(m_stTextureFilename);

                // 创建纹理资产
                auto texAsset = AssetSystem::GetInstance().CreateAsset<BasicTexture2DAsset>(pool, {}, {
                    {"path", texPath.ToStringView()},
                    {"mipmap", asset->IsGenerateMipmaps()},
                });
                if (!texAsset)
                {
                    LSTG_LOG_ERROR_CAT(HgeFontAssetLoader, "Load font texture from \"{}\" fail: {}", texPath.ToStringView(),
                        texAsset.GetError());
                    SetState(AssetLoadingStates::Error);
                    return;
                }

                m_pLoadedTexture = std::move(*texAsset);
            }
            catch (...)  // bad_alloc
            {
                LSTG_LOG_ERROR_CAT(HgeFontAssetLoader, "Not enough memory");
                SetState(AssetLoadingStates::Error);
                return;
            }

            m_bWaitForTextureLoaded = true;
        }
        else
        {
            assert(m_pLoadedTexture);
            assert(m_pLoadedFontFace);

            // 检查纹理是否加载完毕
            if (m_pLoadedTexture->GetState() == AssetStates::Loaded)
            {
                // 绑定纹理
                assert(m_pLoadedTexture->GetState() == AssetStates::Loaded);
                static_pointer_cast<Render::Font::HgeFontFace>(m_pLoadedFontFace)->SetAtlasTexture(m_pLoadedTexture->GetTexture());

                // 创建 FontCollection
                // 目前 API 设计下，不支持字体 fallback
                Render::Font::FontCollectionPtr collection;
                try
                {
                    collection = make_shared<Render::Font::FontCollection>(m_pLoadedFontFace);
                }
                catch (...)  // bad_alloc
                {
                    LSTG_LOG_ERROR_CAT(HgeFontAssetLoader, "Not enough memory");
                    SetState(AssetLoadingStates::Error);
                    return;
                }

#if LSTG_ASSET_HOT_RELOAD
                // 更新版本号
                m_uLoadedTextureVersion = m_pLoadedTexture->GetVersion();
#endif

                // 提交资源
                asset->UpdateResource(std::move(m_pLoadedTexture), std::move(m_pLoadedFontFace), std::move(collection));

                // 转换到加载完毕状态
                SetState(AssetLoadingStates::Loaded);
            }
        }
    }
}

#if LSTG_ASSET_HOT_RELOAD
bool HgeFontAssetLoader::SupportHotReload() const noexcept
{
    return true;
}

bool HgeFontAssetLoader::CheckIsOutdated() const noexcept
{
    assert(GetState() == AssetLoadingStates::Loaded || GetState() == AssetLoadingStates::Error);

    auto asset = static_pointer_cast<HgeFontAsset>(GetAsset());

    // 获取关联纹理，检查是否过期
    auto texture = asset->GetFontTexture();
    if (texture->GetVersion() != m_uLoadedTextureVersion)
        return true;

    // 获取文件属性
    auto attr = AssetSystem::GetInstance().GetAssetStreamAttribute(asset->GetPath());
    if (!attr)
    {
        LSTG_LOG_WARN_CAT(HgeFontAssetLoader, "Get stream attribute from \"{}\" fail: {}", asset->GetPath(), attr.GetError());
        return false;
    }

    // 检查是否发生修改
    return attr->LastModified != m_stSourceAttribute.LastModified;
}

void HgeFontAssetLoader::PrepareToReload() noexcept
{
    assert(GetState() == AssetLoadingStates::Loaded || GetState() == AssetLoadingStates::Error);

    auto asset = static_pointer_cast<HgeFontAsset>(GetAsset());

    // 明确加载的情形，如果是字体文件被修改，则需要整体重新加载，如果仅仅是纹理发生改变，则只需要更新纹理
    auto attr = AssetSystem::GetInstance().GetAssetStreamAttribute(asset->GetPath());
    if (!attr || attr->LastModified != m_stSourceAttribute.LastModified)  // 如果获取文件属性失败，也当做需要整个更新处理
    {
        // 转到 DependencyLoading 状态，准备重新加载
        m_bWaitForTextureLoaded = false;
        SetState(AssetLoadingStates::DependencyLoading);
        return;
    }

    // 获取关联纹理，检查是否过期
    auto texture = asset->GetFontTexture();
    if (texture->GetVersion() != m_uLoadedTextureVersion)
    {
        m_uLoadedTextureVersion = texture->GetVersion();
        asset->UpdateResource(texture, asset->GetFontFace(), asset->GetFontCollection());
    }

    SetState(AssetLoadingStates::Loaded);
}
#endif

Result<Subsystem::Render::TexturePtr> HgeFontAssetLoader::OnLoadTexture(std::string_view path) noexcept
{
    assert(m_stTextureFilename.empty());
    m_stTextureFilename = path;
    return nullptr;  // 返回空纹理进行占位
}
