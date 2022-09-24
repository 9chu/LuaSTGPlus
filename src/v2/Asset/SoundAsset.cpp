/**
 * @file
 * @date 2022/9/17
 * @author 9chu
 * 此文件为 LuaSTGPlus 项目的一部分，版权与许可声明详见 COPYRIGHT.txt。
 */
#include <lstg/v2/Asset/SoundAsset.hpp>

#include <lstg/Core/AppBase.hpp>
#include <lstg/Core/Subsystem/Script/LuaStack.hpp>
#include <lstg/Core/Subsystem/AudioSystem.hpp>

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

void SoundAsset::OnRemove() noexcept
{
    // 特殊处理：当被播放的音频删除时，从音频引擎中剔除
    if (m_stSourceInstance)
    {
        auto& audioSystem = *AppBase::GetInstance().GetSubsystem<Subsystem::AudioSystem>();
        audioSystem.GetEngine().SourceDelete(*m_stSourceInstance);
        m_stSourceInstance = {};
    }
}

void SoundAsset::UpdateResource(Subsystem::Audio::SoundDataPtr data) noexcept
{
    assert(data);
    m_pSoundData = std::move(data);

#if LSTG_ASSET_HOT_RELOAD
    UpdateVersion();
#endif
}
