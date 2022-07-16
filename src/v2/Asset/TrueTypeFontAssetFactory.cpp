/**
 * @file
 * @date 2022/7/10
 * @author 9chu
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#include <lstg/v2/Asset/TrueTypeFontAssetFactory.hpp>

#include <lstg/Core/Text/JsonHelper.hpp>
#include <lstg/Core/Subsystem/Asset/AssetError.hpp>
#include <lstg/v2/Asset/TrueTypeFontAsset.hpp>
#include <lstg/v2/Asset/TrueTypeFontAssetLoader.hpp>

using namespace std;
using namespace lstg;
using namespace lstg::v2::Asset;

using namespace lstg::Text;

TrueTypeFontAssetFactory::TrueTypeFontAssetFactory()
{
    m_pTTFFontFactory = Subsystem::Render::Font::CreateFreeTypeFactory();
}

std::string_view TrueTypeFontAssetFactory::GetAssetTypeName() const noexcept
{
    return "TrueTypeFont";
}

Subsystem::Asset::AssetTypeId TrueTypeFontAssetFactory::GetAssetTypeId() const noexcept
{
    return TrueTypeFontAsset::GetAssetTypeIdStatic();
}

Result<Subsystem::Asset::CreateAssetResult> TrueTypeFontAssetFactory::CreateAsset(Subsystem::AssetSystem& assetSystem,
    Subsystem::Asset::AssetPoolPtr pool, std::string_view name, const nlohmann::json& arguments,
    Subsystem::Asset::IAssetDependencyResolver* resolver) noexcept
{
    auto path = JsonHelper::ReadValue<string>(arguments, "/path");
    auto fontSize = JsonHelper::ReadValue<uint32_t>(arguments, "/size");
    if (!path || !fontSize)
        return make_error_code(Subsystem::Asset::AssetError::MissingRequiredArgument);

    try
    {
        auto asset = make_shared<TrueTypeFontAsset>(std::string{name}, std::move(*path), *fontSize);
        auto loader = make_shared<TrueTypeFontAssetLoader>(asset, m_pTTFFontFactory);
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
