/**
 * @file
 * @date 2022/9/17
 * @author 9chu
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#include <lstg/v2/Asset/SoundAsset.hpp>

#include <lstg/Core/Subsystem/Script/LuaStack.hpp>

using namespace std;
using namespace lstg;
using namespace lstg::v2::Asset;

Subsystem::Asset::AssetTypeId SoundAsset::GetAssetTypeIdStatic() noexcept
{
    const auto& uniqueTypeName = Subsystem::Script::detail::GetUniqueTypeName<SoundAsset>();
    return uniqueTypeName.Id;
}

SoundAsset::SoundAsset(std::string name, std::string path)
    : Subsystem::Asset::Asset(std::move(name)), m_stPath(std::move(path))
{
}

Subsystem::Asset::AssetTypeId SoundAsset::GetAssetTypeId() const noexcept
{
    return GetAssetTypeIdStatic();
}

void SoundAsset::UpdateResource(Subsystem::Audio::SoundDataPtr data) noexcept
{
    assert(data);
    m_pSoundData = std::move(data);

#if LSTG_ASSET_HOT_RELOAD
    UpdateVersion();
#endif
}
