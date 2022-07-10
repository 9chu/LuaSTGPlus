/**
 * @file
 * @date 2022/4/26
 * @author 9chu
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#include <lstg/Core/Subsystem/Asset/BasicTexture2DAssetFactory.hpp>

#include <lstg/Core/Text/JsonHelper.hpp>
#include <lstg/Core/Subsystem/Asset/AssetError.hpp>
#include <lstg/Core/Subsystem/Asset/BasicTexture2DAsset.hpp>
#include <lstg/Core/Subsystem/Asset/BasicTexture2DAssetLoader.hpp>

using namespace std;
using namespace lstg;
using namespace lstg::Subsystem::Asset;

using namespace lstg::Text;

std::string_view BasicTexture2DAssetFactory::GetAssetTypeName() const noexcept
{
    return "BasicTexture2D";
}

AssetTypeId BasicTexture2DAssetFactory::GetAssetTypeId() const noexcept
{
    return BasicTexture2DAsset::GetAssetTypeIdStatic();
}

Result<CreateAssetResult> BasicTexture2DAssetFactory::CreateAsset(AssetSystem& assetSystem, AssetPoolPtr pool, std::string_view name,
    const nlohmann::json& arguments) noexcept
{
    auto path = JsonHelper::ReadValue<string>(arguments, "/path");
    auto mipmaps = JsonHelper::ReadValue<bool>(arguments, "/mipmaps", true);
    if (!path)
        return make_error_code(AssetError::MissingRequiredArgument);

    try
    {
        auto asset = make_shared<BasicTexture2DAsset>(std::string{name}, std::move(*path));
        auto loader = make_shared<BasicTexture2DAssetLoader>(asset, mipmaps);
        return CreateAssetResult { static_pointer_cast<Asset>(asset), static_pointer_cast<AssetLoader>(loader) };
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
