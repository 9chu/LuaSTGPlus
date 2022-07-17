/**
 * @file
 * @date 2022/7/16
 * @author 9chu
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
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

void EffectAsset::UpdateResource(Subsystem::Render::GraphDef::ImmutableEffectDefinitionPtr def) noexcept
{
    assert(def);
    m_pEffectDef = std::move(def);

#if LSTG_ASSET_HOT_RELOAD
    UpdateVersion();
#endif
}
