/**
 * @file
 * @date 2022/7/15
 * @author 9chu
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#include <lstg/v2/Asset/HgeParticleAsset.hpp>

using namespace std;
using namespace lstg;
using namespace lstg::v2::Asset;

Subsystem::Asset::AssetTypeId HgeParticleAsset::GetAssetTypeIdStatic() noexcept
{
    const auto& uniqueTypeName = Subsystem::Script::detail::GetUniqueTypeName<HgeParticleAsset>();
    return uniqueTypeName.Id;
}

HgeParticleAsset::HgeParticleAsset(std::string name, std::string path, SpriteAssetPtr spriteAsset, ColliderShape colliderShape)
    : Subsystem::Asset::Asset(std::move(name)), m_stPath(std::move(path)), m_pSpriteAsset(std::move(spriteAsset)),
    m_stColliderShape(colliderShape)
{
    assert(m_pSpriteAsset);
}

Subsystem::Asset::AssetTypeId HgeParticleAsset::GetAssetTypeId() const noexcept
{
    return GetAssetTypeIdStatic();
}

void HgeParticleAsset::UpdateResource(const Subsystem::Render::Drawing2D::ParticleConfig& config) noexcept
{
    m_stParticleConfig = config;
    m_stParticleConfig.ParticleSprite = &m_pSpriteAsset->GetDrawingSprite();

#if LSTG_ASSET_HOT_RELOAD
    UpdateVersion();
#endif
}
