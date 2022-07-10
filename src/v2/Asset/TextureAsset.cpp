/**
 * @file
 * @date 2022/5/30
 * @author 9chu
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#include <lstg/v2/Asset/TextureAsset.hpp>

#include <lstg/Core/Subsystem/Script/LuaStack.hpp>

using namespace std;
using namespace lstg;
using namespace lstg::v2::Asset;

Subsystem::Asset::AssetTypeId TextureAsset::GetAssetTypeIdStatic() noexcept
{
    const auto& uniqueTypeName = Subsystem::Script::detail::GetUniqueTypeName<TextureAsset>();
    return uniqueTypeName.Id;
}

TextureAsset::TextureAsset(std::string name, Subsystem::Asset::BasicTexture2DAssetPtr textureAsset, float pixelPerUnit)
    : Subsystem::Asset::Asset(std::move(name)), m_pTextureAsset(std::move(textureAsset))
{
    m_stDrawingTexture.SetUnderlayTexture(m_pTextureAsset->GetTexture());
    m_stDrawingTexture.SetPixelPerUnit(std::abs(pixelPerUnit) <= std::numeric_limits<float>::epsilon() ? 1.f : pixelPerUnit);
}

Subsystem::Asset::AssetTypeId TextureAsset::GetAssetTypeId() const noexcept
{
    return GetAssetTypeIdStatic();
}

void TextureAsset::UpdateResource() noexcept
{
    m_stDrawingTexture.SetUnderlayTexture(m_pTextureAsset->GetTexture());

#if LSTG_ASSET_HOT_RELOAD
    UpdateVersion();
#endif
}
