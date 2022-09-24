/**
 * @file
 * @author 9chu
 * @date 2022/4/18
 * 此文件为 LuaSTGPlus 项目的一部分，版权与许可声明详见 COPYRIGHT.txt。
 */
#include <lstg/v2/Bridge/AudioModule.hpp>

#include <lstg/Core/Logging.hpp>
#include <lstg/Core/Subsystem/AudioSystem.hpp>
#include <lstg/v2/Asset/SoundAsset.hpp>
#include <lstg/v2/Asset/MusicAsset.hpp>
#include "detail/Helper.hpp"

#define ENSURE_SUCCESS(EXPR)  \
    do {  \
        auto r_ = (EXPR);  \
        assert(r_);  \
    } while (false)

using namespace std;
using namespace lstg;
using namespace lstg::v2::Bridge;

using namespace lstg::Subsystem::Audio;

LSTG_DEF_LOG_CATEGORY(AudioModule);

void AudioModule::PlaySound(LuaStack& stack, const char* name, double vol, std::optional<double> pan /* =0.0 */)
{
    auto& audioEngine = detail::GetGlobalApp().GetSubsystem<Subsystem::AudioSystem>()->GetEngine();
    auto assetPools = detail::GetGlobalApp().GetAssetPools();

    // 获取声音对象
    auto asset = assetPools->FindAsset(AssetTypes::Sound, name);
    if (!asset)
        stack.Error("sound '%s' not found.", name);
    assert(asset);
    assert(asset->GetAssetTypeId() == Asset::SoundAsset::GetAssetTypeIdStatic());
    auto soundAsset = static_pointer_cast<Asset::SoundAsset>(asset);

    // 如果没有音频数据，则跳过
    if (!soundAsset->GetSoundData())
    {
        LSTG_LOG_WARN_CAT(AudioModule, "sound '{}' not ready", name);
        return;
    }

    // 如果已经存在发声源实例，则先停止
    if (soundAsset->GetSourceInstance())
    {
        auto sourceId = *soundAsset->GetSourceInstance();
        audioEngine.SourceDelete(sourceId);
        soundAsset->SetSourceInstance({});
    }

    // 创建发声实例
    // 音效对象总是自动销毁（由于只有一个实例，不自动销毁也行）
    auto flags = SoundSourceCreationFlags::DisposeAfterStopped | SoundSourceCreationFlags::PlayImmediately;
    auto ret = audioEngine.SourceAdd(SOUND_BUS_ID, soundAsset->GetSoundData(), flags, static_cast<float>(vol),
        pan ? static_cast<float>(*pan) : 0.f);
    if (!ret)
    {
        // 音频系统失败不终止流程
        LSTG_LOG_ERROR_CAT(AudioModule, "sound '{}' play fail: {}", name, ret.GetError());
        return;
    }

    // 设置到资源
    soundAsset->SetSourceInstance(*ret);
}

void AudioModule::StopSound(LuaStack& stack, const char* name)
{
    auto& audioEngine = detail::GetGlobalApp().GetSubsystem<Subsystem::AudioSystem>()->GetEngine();
    auto assetPools = detail::GetGlobalApp().GetAssetPools();

    // 获取声音对象
    auto asset = assetPools->FindAsset(AssetTypes::Sound, name);
    if (!asset)
        stack.Error("sound '%s' not found.", name);
    assert(asset);
    assert(asset->GetAssetTypeId() == Asset::SoundAsset::GetAssetTypeIdStatic());
    auto soundAsset = static_pointer_cast<Asset::SoundAsset>(asset);

    if (soundAsset->GetSourceInstance())
    {
        auto sourceId = *soundAsset->GetSourceInstance();
        audioEngine.SourceDelete(sourceId);
        soundAsset->SetSourceInstance({});
    }
}

void AudioModule::PauseSound(LuaStack& stack, const char* name)
{
    auto& audioEngine = detail::GetGlobalApp().GetSubsystem<Subsystem::AudioSystem>()->GetEngine();
    auto assetPools = detail::GetGlobalApp().GetAssetPools();

    // 获取声音对象
    auto asset = assetPools->FindAsset(AssetTypes::Sound, name);
    if (!asset)
        stack.Error("sound '%s' not found.", name);
    assert(asset);
    assert(asset->GetAssetTypeId() == Asset::SoundAsset::GetAssetTypeIdStatic());
    auto soundAsset = static_pointer_cast<Asset::SoundAsset>(asset);

    if (soundAsset->GetSourceInstance())
    {
        auto sourceId = *soundAsset->GetSourceInstance();
        auto ret = audioEngine.SourcePause(sourceId);
        if (!ret)
        {
            // 此时可能已经释放
            soundAsset->SetSourceInstance({});
        }
    }
}

void AudioModule::ResumeSound(LuaStack& stack, const char* name)
{
    auto& audioEngine = detail::GetGlobalApp().GetSubsystem<Subsystem::AudioSystem>()->GetEngine();
    auto assetPools = detail::GetGlobalApp().GetAssetPools();

    // 获取声音对象
    auto asset = assetPools->FindAsset(AssetTypes::Sound, name);
    if (!asset)
        stack.Error("sound '%s' not found.", name);
    assert(asset);
    assert(asset->GetAssetTypeId() == Asset::SoundAsset::GetAssetTypeIdStatic());
    auto soundAsset = static_pointer_cast<Asset::SoundAsset>(asset);

    if (soundAsset->GetSourceInstance())
    {
        auto sourceId = *soundAsset->GetSourceInstance();
        auto ret = audioEngine.SourcePlay(sourceId);
        if (!ret)
        {
            // 此时可能已经释放
            soundAsset->SetSourceInstance({});
        }
    }
}

const char* AudioModule::GetSoundState(LuaStack& stack, const char* name)
{
    auto& audioEngine = detail::GetGlobalApp().GetSubsystem<Subsystem::AudioSystem>()->GetEngine();
    auto assetPools = detail::GetGlobalApp().GetAssetPools();

    // 获取声音对象
    auto asset = assetPools->FindAsset(AssetTypes::Sound, name);
    if (!asset)
        stack.Error("sound '%s' not found.", name);
    assert(asset);
    assert(asset->GetAssetTypeId() == Asset::SoundAsset::GetAssetTypeIdStatic());
    auto soundAsset = static_pointer_cast<Asset::SoundAsset>(asset);

    if (soundAsset->GetSourceInstance())
    {
        auto sourceId = *soundAsset->GetSourceInstance();
        auto ret = audioEngine.SourceIsPlaying(sourceId);
        if (!ret)
        {
            // 此时可能已经释放
            soundAsset->SetSourceInstance({});
        }
        else
        {
            return *ret ? "playing" : "paused";
        }
    }
    return "stopped";
}

void AudioModule::PlayMusic(LuaStack& stack, const char* name, std::optional<double> vol /* =1.0 */, std::optional<double> position)
{
    auto& audioEngine = detail::GetGlobalApp().GetSubsystem<Subsystem::AudioSystem>()->GetEngine();
    auto assetPools = detail::GetGlobalApp().GetAssetPools();

    // 获取声音对象
    auto asset = assetPools->FindAsset(AssetTypes::Music, name);
    if (!asset)
        stack.Error("music '%s' not found.", name);
    assert(asset);
    assert(asset->GetAssetTypeId() == Asset::MusicAsset::GetAssetTypeIdStatic());
    auto musicAsset = static_pointer_cast<Asset::MusicAsset>(asset);

    // 如果没有音频数据，则跳过
    if (!musicAsset->GetSoundData())
    {
        LSTG_LOG_WARN_CAT(AudioModule, "music '{}' not ready", name);
        return;
    }

    // 如果已经存在发声源实例，则先停止
    if (musicAsset->GetSourceInstance())
    {
        auto sourceId = *musicAsset->GetSourceInstance();
        audioEngine.SourceDelete(sourceId);
        musicAsset->SetSourceInstance({});
    }

    // 创建发声实例
    // 音效对象总是自动销毁（由于只有一个实例，不自动销毁也行）
    auto flags = SoundSourceCreationFlags::DisposeAfterStopped | SoundSourceCreationFlags::Looping;
    if (!position)
        flags |= SoundSourceCreationFlags::PlayImmediately;
    auto ret = audioEngine.SourceAdd(MUSIC_BUS_ID, musicAsset->GetSoundData(), flags, vol ? static_cast<float>(*vol) : 1.f, {},
        std::get<0>(musicAsset->GetLoopRange()), std::get<1>(musicAsset->GetLoopRange()));
    if (!ret)
    {
        // 音频系统失败不终止流程
        LSTG_LOG_ERROR_CAT(AudioModule, "sound '{}' play fail: {}", name, ret.GetError());
        return;
    }

    // 设置位置
    if (position)
    {
        ENSURE_SUCCESS(audioEngine.SourceSetPosition(*ret, static_cast<uint32_t>(*position * 1000)));
        ENSURE_SUCCESS(audioEngine.SourcePlay(*ret));
    }

    // 设置到资源
    musicAsset->SetSourceInstance(*ret);
}

void AudioModule::StopMusic(LuaStack& stack, const char* name)
{
    auto& audioEngine = detail::GetGlobalApp().GetSubsystem<Subsystem::AudioSystem>()->GetEngine();
    auto assetPools = detail::GetGlobalApp().GetAssetPools();

    // 获取声音对象
    auto asset = assetPools->FindAsset(AssetTypes::Music, name);
    if (!asset)
        stack.Error("music '%s' not found.", name);
    assert(asset);
    assert(asset->GetAssetTypeId() == Asset::MusicAsset::GetAssetTypeIdStatic());
    auto musicAsset = static_pointer_cast<Asset::MusicAsset>(asset);

    if (musicAsset->GetSourceInstance())
    {
        auto sourceId = *musicAsset->GetSourceInstance();
        audioEngine.SourceDelete(sourceId);
        musicAsset->SetSourceInstance({});
    }
}

void AudioModule::PauseMusic(LuaStack& stack, const char* name)
{
    auto& audioEngine = detail::GetGlobalApp().GetSubsystem<Subsystem::AudioSystem>()->GetEngine();
    auto assetPools = detail::GetGlobalApp().GetAssetPools();

    // 获取声音对象
    auto asset = assetPools->FindAsset(AssetTypes::Music, name);
    if (!asset)
        stack.Error("music '%s' not found.", name);
    assert(asset);
    assert(asset->GetAssetTypeId() == Asset::MusicAsset::GetAssetTypeIdStatic());
    auto musicAsset = static_pointer_cast<Asset::MusicAsset>(asset);

    if (musicAsset->GetSourceInstance())
    {
        auto sourceId = *musicAsset->GetSourceInstance();
        auto ret = audioEngine.SourcePause(sourceId);
        if (!ret)
        {
            // 此时可能已经释放
            musicAsset->SetSourceInstance({});
        }
    }
}

void AudioModule::ResumeMusic(LuaStack& stack, const char* name)
{
    auto& audioEngine = detail::GetGlobalApp().GetSubsystem<Subsystem::AudioSystem>()->GetEngine();
    auto assetPools = detail::GetGlobalApp().GetAssetPools();

    // 获取声音对象
    auto asset = assetPools->FindAsset(AssetTypes::Music, name);
    if (!asset)
        stack.Error("music '%s' not found.", name);
    assert(asset);
    assert(asset->GetAssetTypeId() == Asset::MusicAsset::GetAssetTypeIdStatic());
    auto musicAsset = static_pointer_cast<Asset::MusicAsset>(asset);

    if (musicAsset->GetSourceInstance())
    {
        auto sourceId = *musicAsset->GetSourceInstance();
        auto ret = audioEngine.SourcePlay(sourceId);
        if (!ret)
        {
            // 此时可能已经释放
            musicAsset->SetSourceInstance({});
        }
    }
}

const char* AudioModule::GetMusicState(LuaStack& stack, const char* name)
{
    auto& audioEngine = detail::GetGlobalApp().GetSubsystem<Subsystem::AudioSystem>()->GetEngine();
    auto assetPools = detail::GetGlobalApp().GetAssetPools();

    // 获取声音对象
    auto asset = assetPools->FindAsset(AssetTypes::Music, name);
    if (!asset)
        stack.Error("music '%s' not found.", name);
    assert(asset);
    assert(asset->GetAssetTypeId() == Asset::MusicAsset::GetAssetTypeIdStatic());
    auto musicAsset = static_pointer_cast<Asset::MusicAsset>(asset);

    if (musicAsset->GetSourceInstance())
    {
        auto sourceId = *musicAsset->GetSourceInstance();
        auto ret = audioEngine.SourceIsPlaying(sourceId);
        if (!ret)
        {
            // 此时可能已经释放
            musicAsset->SetSourceInstance({});
        }
        else
        {
            return *ret ? "playing" : "paused";
        }
    }
    return "stopped";
}

void AudioModule::UpdateSound()
{
    LSTG_LOG_DEPRECATED(AudioModule, UpdateSound);
}

void AudioModule::SetSEVolume(double vol)
{
    auto& audioEngine = detail::GetGlobalApp().GetSubsystem<Subsystem::AudioSystem>()->GetEngine();
    audioEngine.BusSetVolume(SOUND_BUS_ID, static_cast<float>(vol));
}

void AudioModule::SetBGMVolume(LuaStack& stack, std::variant<double, const char*> arg1, std::optional<double> arg2)
{
    auto& audioEngine = detail::GetGlobalApp().GetSubsystem<Subsystem::AudioSystem>()->GetEngine();
    if (stack.GetTop() == 1)
    {
        if (arg1.index() != 0)
        {
            stack.Error("Argument #1 shall be a number");
            return;
        }
        audioEngine.BusSetVolume(MUSIC_BUS_ID, static_cast<float>(std::get<0>(arg1)));
    }
    else
    {
        if (arg1.index() != 1)
        {
            stack.Error("Argument #1 shall be a string");
            return;
        }
        if (!arg2)
        {
            stack.Error("Argument #2 required");
            return;
        }

        auto assetPools = detail::GetGlobalApp().GetAssetPools();

        // 获取声音对象
        auto asset = assetPools->FindAsset(AssetTypes::Music, std::get<1>(arg1));
        if (!asset)
            stack.Error("music '%s' not found.", std::get<1>(arg1));
        assert(asset);
        assert(asset->GetAssetTypeId() == Asset::MusicAsset::GetAssetTypeIdStatic());
        auto musicAsset = static_pointer_cast<Asset::MusicAsset>(asset);

        if (musicAsset->GetSourceInstance())
        {
            auto sourceId = *musicAsset->GetSourceInstance();
            auto ret = audioEngine.SourceSetVolume(sourceId, static_cast<float>(*arg2));
            if (!ret)
            {
                // 此时可能已经释放
                musicAsset->SetSourceInstance({});
            }
        }
    }
}
