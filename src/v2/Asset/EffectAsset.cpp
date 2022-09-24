/**
 * @file
 * @date 2022/7/16
 * @author 9chu
 * 此文件为 LuaSTGPlus 项目的一部分，版权与许可声明详见 COPYRIGHT.txt。
 */
#include <lstg/v2/Asset/EffectAsset.hpp>

#include <lstg/Core/Subsystem/Script/LuaStack.hpp>

using namespace std;
using namespace lstg;
using namespace lstg::v2::Asset;

Subsystem::Asset::AssetTypeId EffectAsset::GetAssetTypeIdStatic() noexcept
{
    const auto& uniqueTypeName = Subsystem::Script::detail::GetUniqueTypeName<EffectAsset>();
    return uniqueTypeName.Id;
}

EffectAsset::EffectAsset(std::string name, std::string path)
    : Subsystem::Asset::Asset(std::move(name)), m_stPath(std::move(path))
{
}

Subsystem::Asset::AssetTypeId EffectAsset::GetAssetTypeId() const noexcept
{
    return GetAssetTypeIdStatic();
}

void EffectAsset::UpdateResource(Subsystem::Render::GraphDef::ImmutableEffectDefinitionPtr def, Subsystem::Render::MaterialPtr mat) noexcept
{
    assert(def && mat);
    m_pEffectDef = std::move(def);
    m_pDefaultMaterial = std::move(mat);

#if LSTG_ASSET_HOT_RELOAD
    UpdateVersion();
#endif
}
