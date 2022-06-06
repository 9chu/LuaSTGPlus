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

TextureAsset::TextureAsset(std::string name, Subsystem::Asset::BasicTextureAssetPtr basicTexture, double pixelPerUnit)
    : Subsystem::Asset::Asset(std::move(name)), m_pBasicTexture(std::move(basicTexture)),
    m_dPixelPerUnit(std::abs(pixelPerUnit) <= std::numeric_limits<double>::epsilon() ? 1. : pixelPerUnit)
{
}

const Subsystem::Asset::BasicTextureAssetPtr& TextureAsset::GetBasicTexture() const noexcept
{
    return m_pBasicTexture;
}

double TextureAsset::GetPixelPerUnit() const noexcept
{
    return m_dPixelPerUnit;
}

double TextureAsset::GetWidth() const noexcept
{
    return m_pBasicTexture->GetWidth() / GetPixelPerUnit();
}

double TextureAsset::GetHeight() const noexcept
{
    return m_pBasicTexture->GetHeight() / GetPixelPerUnit();
}

Subsystem::Asset::AssetTypeId TextureAsset::GetAssetTypeId() const noexcept
{
    return GetAssetTypeIdStatic();
}
