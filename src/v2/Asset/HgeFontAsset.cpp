/**
 * @file
 * @date 2022/7/13
 * @author 9chu
 * 此文件为 LuaSTGPlus 项目的一部分，版权与许可声明详见 COPYRIGHT.txt。
 */
#include <lstg/v2/Asset/HgeFontAsset.hpp>

#include <lstg/Core/Subsystem/Asset/AssetPool.hpp>
#include <lstg/Core/Subsystem/Script/LuaStack.hpp>
#include "../../Core/Subsystem/Render/Font/HgeFontFace.hpp"

using namespace std;
using namespace lstg;
using namespace lstg::v2::Asset;

Subsystem::Asset::AssetTypeId HgeFontAsset::GetAssetTypeIdStatic() noexcept
{
    const auto& uniqueTypeName = Subsystem::Script::detail::GetUniqueTypeName<HgeFontAsset>();
    return uniqueTypeName.Id;
}

HgeFontAsset::HgeFontAsset(std::string name, std::string path, bool mipmap)
    : Subsystem::Asset::Asset(std::move(name)), m_stPath(std::move(path)), m_bGenerateMipmaps(mipmap)
{
}

HgeFontAsset::~HgeFontAsset()
{
    assert(m_pFontTexture == nullptr || m_pFontTexture->GetId() == Subsystem::Asset::kEmptyAssetId);
}

Subsystem::Asset::AssetTypeId HgeFontAsset::GetAssetTypeId() const noexcept
{
    return GetAssetTypeIdStatic();
}

void HgeFontAsset::OnRemove() noexcept
{
    FreeResource();
}

void HgeFontAsset::FreeResource() noexcept
{
    // 释放关联资源
    if (m_pFontTexture && !m_pFontTexture->IsWildAsset())
    {
        assert(m_pFontTexture->GetId() != Subsystem::Asset::kEmptyAssetId);

        auto pool = m_pFontTexture->GetPool().lock();
        if (pool)
            pool->RemoveAsset(m_pFontTexture->GetId());
        m_pFontTexture.reset();
    }
}

void HgeFontAsset::UpdateResource(Subsystem::Asset::BasicTexture2DAssetPtr fontTexture, Subsystem::Render::Font::FontFacePtr fontFace,
    Subsystem::Render::Font::FontCollectionPtr fontCollection) noexcept
{
    if (fontTexture == m_pFontTexture)
    {
        // 刷新纹理
        static_pointer_cast<Subsystem::Render::Font::HgeFontFace>(m_pFontFace)->SetAtlasTexture(m_pFontTexture->GetTexture());

        // 此时 FontFace 和 FontCollection 一定不变
        assert(fontFace == m_pFontFace && fontCollection == m_pFontCollection);
    }
    else
    {
        // 释放资源
        FreeResource();

        assert(static_pointer_cast<Subsystem::Render::Font::HgeFontFace>(fontFace)->GetAtlasTexture() == fontTexture->GetTexture());
        m_pFontTexture = std::move(fontTexture);
        m_pFontFace = std::move(fontFace);
        m_pFontCollection = std::move(fontCollection);
    }

#if LSTG_ASSET_HOT_RELOAD
    UpdateVersion();
#endif
}
