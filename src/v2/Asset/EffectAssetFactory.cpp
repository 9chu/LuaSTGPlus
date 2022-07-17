/**
 * @file
 * @date 2022/7/16
 * @author 9chu
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#include <lstg/v2/Asset/EffectAssetFactory.hpp>

#include <lstg/Core/Text/JsonHelper.hpp>
#include <lstg/Core/Subsystem/Asset/AssetError.hpp>
#include <lstg/v2/Asset/EffectAsset.hpp>
#include <lstg/v2/Asset/EffectAssetLoader.hpp>

using namespace std;
using namespace lstg;
using namespace lstg::v2::Asset;

using namespace lstg::Text;

std::string_view EffectAssetFactory::GetAssetTypeName() const noexcept
{
    return "Effect";
}

Subsystem::Asset::AssetTypeId EffectAssetFactory::GetAssetTypeId() const noexcept
{
    return EffectAsset::GetAssetTypeIdStatic();
}

Result<Subsystem::Asset::CreateAssetResult> EffectAssetFactory::CreateAsset(Subsystem::AssetSystem& assetSystem,
    Subsystem::Asset::AssetPoolPtr pool, std::string_view name, const nlohmann::json& arguments,
    Subsystem::Asset::IAssetDependencyResolver* resolver) noexcept
{
    auto path = JsonHelper::ReadValue<string>(arguments, "/path");
    if (!path)
        return make_error_code(Subsystem::Asset::AssetError::MissingRequiredArgument);

    try
    {
        auto asset = make_shared<EffectAsset>(std::string{name}, std::move(*path));
        auto loader = make_shared<EffectAssetLoader>(asset);
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
