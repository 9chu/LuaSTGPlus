/**
 * @file
 * @date 2022/7/10
 * @author 9chu
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#include <lstg/v2/Asset/TrueTypeFontAsset.hpp>

#include <lstg/Core/Subsystem/Script/LuaStack.hpp>

using namespace std;
using namespace lstg;
using namespace lstg::v2::Asset;

Subsystem::Asset::AssetTypeId TrueTypeFontAsset::GetAssetTypeIdStatic() noexcept
{
    const auto& uniqueTypeName = Subsystem::Script::detail::GetUniqueTypeName<TrueTypeFontAsset>();
    return uniqueTypeName.Id;
}

TrueTypeFontAsset::TrueTypeFontAsset(std::string name, std::string path, uint32_t fontSize)
    : Subsystem::Asset::Asset(std::move(name)), m_stPath(std::move(path)), m_uFontSize(fontSize)
{
}

Subsystem::Asset::AssetTypeId TrueTypeFontAsset::GetAssetTypeId() const noexcept
{
    return GetAssetTypeIdStatic();
}

void TrueTypeFontAsset::UpdateResource(Subsystem::Render::Font::FontFacePtr fontFace,
    Subsystem::Render::Font::FontCollectionPtr fontCollection) noexcept
{
    m_pFontFace = std::move(fontFace);
    m_pFontCollection = std::move(fontCollection);

#if LSTG_ASSET_HOT_RELOAD
    UpdateVersion();
#endif
}
