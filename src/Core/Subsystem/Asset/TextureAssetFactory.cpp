/**
 * @file
 * @date 2022/4/26
 * @author 9chu
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#include <lstg/Core/Subsystem/Asset/TextureAssetFactory.hpp>

#include <lstg/Core/Subsystem/Asset/ArgumentHelper.hpp>
#include <lstg/Core/Subsystem/Asset/AssetError.hpp>
#include <lstg/Core/Subsystem/Asset/TextureAsset.hpp>
#include <lstg/Core/Subsystem/Asset/TextureAssetLoader.hpp>

using namespace std;
using namespace lstg;
using namespace lstg::Subsystem::Asset;

std::string_view TextureAssetFactory::GetAssetTypeName() const noexcept
{
    return "Texture";
}

AssetTypeId TextureAssetFactory::GetAssetTypeId() const noexcept
{
    return TextureAsset::GetAssetTypeIdStatic();
}

Result<CreateAssetResult> TextureAssetFactory::CreateAsset(AssetSystem& assetSystem, AssetPoolPtr pool, std::string_view name,
    const nlohmann::json& arguments) noexcept
{
    auto path = ReadArgument<string>(arguments, "/path");
    auto mipmaps = ReadArgument<bool>(arguments, "/mipmaps", true);
    if (!path)
        return make_error_code(AssetError::MissingRequiredArgument);

    try
    {
        auto asset = make_shared<TextureAsset>(std::string{name}, std::move(*path));
        auto loader = make_shared<TextureAssetLoader>(asset, mipmaps);
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
