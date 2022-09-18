/**
 * @file
 * @date 2022/9/17
 * @author 9chu
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#include <lstg/v2/Asset/MusicAsset.hpp>

#include <lstg/Core/AppBase.hpp>
#include <lstg/Core/Subsystem/Script/LuaStack.hpp>
#include <lstg/Core/Subsystem/AudioSystem.hpp>

using namespace std;
using namespace lstg;
using namespace lstg::v2::Asset;

Subsystem::Asset::AssetTypeId MusicAsset::GetAssetTypeIdStatic() noexcept
{
    const auto& uniqueTypeName = Subsystem::Script::detail::GetUniqueTypeName<MusicAsset>();
    return uniqueTypeName.Id;
}

MusicAsset::MusicAsset(std::string name, std::string path, MusicLoopRange loopRange)
    : Subsystem::Asset::Asset(std::move(name)), m_stPath(std::move(path)), m_stMusicLoopRange(loopRange)
{
}

Subsystem::Asset::AssetTypeId MusicAsset::GetAssetTypeId() const noexcept
{
    return GetAssetTypeIdStatic();
}

void MusicAsset::OnRemove() noexcept
{
    // 特殊处理：当被播放的音频删除时，从音频引擎中剔除
    if (m_stSourceInstance)
    {
        auto& audioSystem = *AppBase::GetInstance().GetSubsystem<Subsystem::AudioSystem>();
        audioSystem.GetEngine().SourceDelete(*m_stSourceInstance);
        m_stSourceInstance = {};
    }
}

void MusicAsset::UpdateResource(Subsystem::Audio::SoundDataPtr data) noexcept
{
    assert(data);
    m_pSoundData = std::move(data);

#if LSTG_ASSET_HOT_RELOAD
    UpdateVersion();
#endif
}
