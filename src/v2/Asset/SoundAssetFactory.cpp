/**
 * @file
 * @date 2022/9/17
 * @author 9chu
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#include <lstg/v2/Asset/SoundAssetFactory.hpp>

#include <lstg/Core/Text/JsonHelper.hpp>
#include <lstg/Core/Subsystem/Asset/AssetError.hpp>
#include <lstg/v2/Asset/SoundAsset.hpp>
#include <lstg/v2/Asset/SoundAssetLoader.hpp>

using namespace std;
using namespace lstg;
using namespace lstg::v2::Asset;

using namespace lstg::Text;

std::string_view SoundAssetFactory::GetAssetTypeName() const noexcept
{
    return "Sound";
}

Subsystem::Asset::AssetTypeId SoundAssetFactory::GetAssetTypeId() const noexcept
{
    return SoundAsset::GetAssetTypeIdStatic();
}

Result<Subsystem::Asset::CreateAssetResult> SoundAssetFactory::CreateAsset(Subsystem::AssetSystem& assetSystem,
    Subsystem::Asset::AssetPoolPtr pool, std::string_view name, const nlohmann::json& arguments,
    Subsystem::Asset::IAssetDependencyResolver* resolver) noexcept
{
    auto path = JsonHelper::ReadValue<string>(arguments, "/path");
    if (!path)
        return make_error_code(Subsystem::Asset::AssetError::MissingRequiredArgument);

    try
    {
        auto asset = make_shared<SoundAsset>(std::string{name}, std::move(*path));
        auto loader = make_shared<SoundAssetLoader>(asset);
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
