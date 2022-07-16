/**
 * @file
 * @date 2022/7/15
 * @author 9chu
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#include <lstg/v2/Asset/HgeParticleAssetFactory.hpp>

#include <lstg/Core/Logging.hpp>
#include <lstg/Core/Text/JsonHelper.hpp>
#include <lstg/Core/Subsystem/Asset/AssetError.hpp>
#include <lstg/v2/Asset/HgeParticleAsset.hpp>
#include <lstg/v2/Asset/HgeParticleAssetLoader.hpp>

using namespace std;
using namespace lstg;
using namespace lstg::v2::Asset;

using namespace lstg::Text;

LSTG_DEF_LOG_CATEGORY(HgeParticleAssetFactory);

std::string_view HgeParticleAssetFactory::GetAssetTypeName() const noexcept
{
    return "HgeParticle";
}

Subsystem::Asset::AssetTypeId HgeParticleAssetFactory::GetAssetTypeId() const noexcept
{
    return HgeParticleAsset::GetAssetTypeIdStatic();
}

Result<Subsystem::Asset::CreateAssetResult> HgeParticleAssetFactory::CreateAsset(Subsystem::AssetSystem& assetSystem,
    Subsystem::Asset::AssetPoolPtr pool, std::string_view name, const nlohmann::json& arguments,
    Subsystem::Asset::IAssetDependencyResolver* resolver) noexcept
{
    auto path = JsonHelper::ReadValue<string>(arguments, "/path");
    auto spriteName = JsonHelper::ReadValue<string>(arguments, "/sprite");
    if (!path || !spriteName)
        return make_error_code(Subsystem::Asset::AssetError::MissingRequiredArgument);
    auto colliderHalfSizeX = JsonHelper::ReadValue<double>(arguments, "/colliderHalfSizeX", 0);
    auto colliderHalfSizeY = JsonHelper::ReadValue<double>(arguments, "/colliderHalfSizeY", 0);
    auto colliderIsRect = JsonHelper::ReadValue<bool>(arguments, "/colliderIsRect", false);

    try
    {
        // 找到依赖的精灵
        auto sprite = static_pointer_cast<SpriteAsset>(resolver->OnResolveAsset(*spriteName));
        if (!sprite)
        {
            LSTG_LOG_ERROR_CAT(HgeParticleAssetFactory, "Sprite \"{}\" not found", *spriteName);
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

        auto asset = make_shared<HgeParticleAsset>(string{name}, std::move(*path), std::move(sprite), collider);
        auto loader = make_shared<HgeParticleAssetLoader>(asset);
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
