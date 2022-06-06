/**
 * @file
 * @date 2022/5/31
 * @author 9chu
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#include <lstg/v2/Asset/SpriteAssetFactory.hpp>

#include <lstg/Core/Logging.hpp>
#include <lstg/Core/Subsystem/AssetSystem.hpp>
#include <lstg/Core/Subsystem/Asset/AssetError.hpp>
#include <lstg/Core/Subsystem/Asset/ArgumentHelper.hpp>
#include <lstg/v2/Asset/SpriteAsset.hpp>
#include <lstg/v2/Asset/SpriteAssetLoader.hpp>

using namespace std;
using namespace lstg;
using namespace lstg::v2::Asset;

LSTG_DEF_LOG_CATEGORY(SpriteAssetFactory);

std::string_view SpriteAssetFactory::GetAssetTypeName() const noexcept
{
    return "Sprite";
}

Subsystem::Asset::AssetTypeId SpriteAssetFactory::GetAssetTypeId() const noexcept
{
    return SpriteAsset::GetAssetTypeIdStatic();
}

Result<Subsystem::Asset::CreateAssetResult> SpriteAssetFactory::CreateAsset(Subsystem::AssetSystem& assetSystem,
    Subsystem::Asset::AssetPoolPtr pool, std::string_view name, const nlohmann::json& arguments) noexcept
{
    auto textureName = Subsystem::Asset::ReadArgument<string>(arguments, "/texture");
    if (!textureName)
        return make_error_code(Subsystem::Asset::AssetError::MissingRequiredArgument);
    auto frameX = Subsystem::Asset::ReadArgument<double>(arguments, "/left");
    auto frameY = Subsystem::Asset::ReadArgument<double>(arguments, "/top");
    auto frameW = Subsystem::Asset::ReadArgument<double>(arguments, "/width");
    auto frameH = Subsystem::Asset::ReadArgument<double>(arguments, "/height");
    if (!frameX || !frameY || !frameW || !frameH)
        return make_error_code(Subsystem::Asset::AssetError::MissingRequiredArgument);
    auto colliderHalfSizeX = Subsystem::Asset::ReadArgument<double>(arguments, "/colliderHalfSizeX", 0);
    auto colliderHalfSizeY = Subsystem::Asset::ReadArgument<double>(arguments, "/colliderHalfSizeY", 0);
    auto colliderIsRect = Subsystem::Asset::ReadArgument<bool>(arguments, "/colliderIsRect", false);

    try
    {
        // 找到依赖的纹理
        auto texture = static_pointer_cast<TextureAsset>(pool->GetAsset(*textureName));
        if (!texture)
        {
            LSTG_LOG_ERROR_CAT(SpriteAssetFactory, "Texture \"{}\" not found", *textureName);
            return make_error_code(Subsystem::Asset::AssetError::DependentAssetNotFound);
        }

        // 创建 Collider
        ColliderShape collider;
        if (colliderIsRect)
        {
            Math::Collider2D::OBBShape<double> obbShape;
            obbShape.HalfSize = Vec2(colliderHalfSizeX, colliderHalfSizeY);
            collider = obbShape;
        }
        else if (colliderHalfSizeX == colliderHalfSizeY)
        {
            Math::Collider2D::CircleShape<double> circleShape;
            circleShape.Radius = colliderHalfSizeX;
            collider = circleShape;
        }
        else
        {
            Math::Collider2D::EllipseShape<double> ellipseShape;
            ellipseShape.A = colliderHalfSizeX;
            ellipseShape.B = colliderHalfSizeY;
            collider = ellipseShape;
        }

        auto asset = make_shared<SpriteAsset>(string{name}, std::move(texture), UVRectangle(*frameX, *frameY, *frameW, *frameH), collider);
        auto loader = make_shared<SpriteAssetLoader>(asset);
        return Subsystem::Asset::CreateAssetResult {
            static_pointer_cast<Subsystem::Asset::Asset>(asset),
            static_pointer_cast<Subsystem::Asset::AssetLoader>(loader)
        };
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
