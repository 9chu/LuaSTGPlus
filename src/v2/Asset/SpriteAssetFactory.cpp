/**
 * @file
 * @date 2022/5/31
 * @author 9chu
 * 此文件为 LuaSTGPlus 项目的一部分，版权与许可声明详见 COPYRIGHT.txt。
 */
#include <lstg/v2/Asset/SpriteAssetFactory.hpp>

#include <lstg/Core/Logging.hpp>
#include <lstg/Core/Text/JsonHelper.hpp>
#include <lstg/Core/Subsystem/AssetSystem.hpp>
#include <lstg/Core/Subsystem/Asset/AssetError.hpp>
#include <lstg/v2/Asset/SpriteAsset.hpp>
#include <lstg/v2/Asset/SpriteAssetLoader.hpp>

using namespace std;
using namespace lstg;
using namespace lstg::v2::Asset;

using namespace lstg::Text;

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
    Subsystem::Asset::AssetPoolPtr pool, std::string_view name, const nlohmann::json& arguments,
    Subsystem::Asset::IAssetDependencyResolver* resolver) noexcept
{
    auto textureName = JsonHelper::ReadValue<string>(arguments, "/texture");
    if (!textureName)
        return make_error_code(Subsystem::Asset::AssetError::MissingRequiredArgument);
    auto frameX = JsonHelper::ReadValue<float>(arguments, "/left");
    auto frameY = JsonHelper::ReadValue<float>(arguments, "/top");
    auto frameW = JsonHelper::ReadValue<float>(arguments, "/width");
    auto frameH = JsonHelper::ReadValue<float>(arguments, "/height");
    if (!frameX || !frameY || !frameW || !frameH)
        return make_error_code(Subsystem::Asset::AssetError::MissingRequiredArgument);
    auto colliderHalfSizeX = JsonHelper::ReadValue<double>(arguments, "/colliderHalfSizeX", 0);
    auto colliderHalfSizeY = JsonHelper::ReadValue<double>(arguments, "/colliderHalfSizeY", 0);
    auto colliderIsRect = JsonHelper::ReadValue<bool>(arguments, "/colliderIsRect", false);

    try
    {
        // 找到依赖的纹理
        assert(resolver);
        auto texture = static_pointer_cast<TextureAsset>(resolver->OnResolveAsset(*textureName));
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

        auto asset = make_shared<SpriteAsset>(string{name}, std::move(texture),
            Math::ImageRectangleFloat(*frameX, *frameY, *frameW, *frameH), collider);
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
