/**
 * @file
 * @date 2022/7/13
 * @author 9chu
 * 此文件为 LuaSTGPlus 项目的一部分，版权与许可声明详见 COPYRIGHT.txt。
 */
#include <lstg/v2/Asset/HgeFontAssetFactory.hpp>

#include <lstg/Core/Text/JsonHelper.hpp>
#include <lstg/Core/Subsystem/Asset/AssetError.hpp>
#include <lstg/v2/Asset/HgeFontAsset.hpp>
#include <lstg/v2/Asset/HgeFontAssetLoader.hpp>

using namespace std;
using namespace lstg;
using namespace lstg::v2::Asset;

using namespace lstg::Text;

HgeFontAssetFactory::HgeFontAssetFactory()
{
    m_pHGEFontFactory = Subsystem::Render::Font::CreateHgeFontFactory();
}

std::string_view HgeFontAssetFactory::GetAssetTypeName() const noexcept
{
    return "HgeFont";
}

Subsystem::Asset::AssetTypeId HgeFontAssetFactory::GetAssetTypeId() const noexcept
{
    return HgeFontAsset::GetAssetTypeIdStatic();
}

Result<Subsystem::Asset::CreateAssetResult> HgeFontAssetFactory::CreateAsset(Subsystem::AssetSystem& assetSystem,
    Subsystem::Asset::AssetPoolPtr pool, std::string_view name, const nlohmann::json& arguments,
    Subsystem::Asset::IAssetDependencyResolver* resolver) noexcept
{
    auto path = JsonHelper::ReadValue<string>(arguments, "/path");
    auto mipmaps = JsonHelper::ReadValue<bool>(arguments, "/mipmaps", true);
    if (!path)
        return make_error_code(Subsystem::Asset::AssetError::MissingRequiredArgument);

    try
    {
        auto asset = make_shared<HgeFontAsset>(std::string{name}, std::move(*path), mipmaps);
        auto loader = make_shared<HgeFontAssetLoader>(asset, m_pHGEFontFactory);
        return Subsystem::Asset::CreateAssetResult { static_pointer_cast<Subsystem::Asset::Asset>(asset),
            static_pointer_cast<Subsystem::Asset::AssetLoader>(loader) };
    }
    catch (const std::system_error& ex)
    {
        return ex.code();
    }
    catch (...)
    {
        return make_error_code(errc::not_enough_memory);
    }
}
