/**
 * @file
 * @date 2022/9/3
 * @author 9chu
 * 此文件为 LuaSTGPlus 项目的一部分，版权与许可声明详见 COPYRIGHT.txt。
 */
#include <lstg/Core/Subsystem/Audio/AudioEngine.hpp>

#include <lstg/Core/Logging.hpp>
#include <lstg/Core/Subsystem/ProfileSystem.hpp>
#include "detail/AudioDevice.hpp"
#include "detail/StaticTopologicalSorter.hpp"
#include "detail/AudioEngineError.hpp"

using namespace std;
using namespace lstg;
using namespace lstg::Subsystem::Audio;

LSTG_DEF_LOG_CATEGORY(AudioEngine);

#ifndef LSTG_AUDIO_SINGLE_THREADED
#define LOCK_MASTER_SCOPE std::unique_lock<std::mutex> lockGuardMaster_(m_stMasterMutex)
#define LOCK_BUS_SCOPE(BUS) std::unique_lock<std::mutex> lockGuardBus_(BUS.Mutex)
#else
#define LOCK_MASTER_SCOPE
#define LOCK_BUS_SCOPE(BUS)
#endif

#define CLAMP_VOLUME(VOL) std::max(0.f, std::min(1.f, (VOL)))
#define CLAMP_PAN(PAN) std::max(-1.f, std::min(1.f, (PAN)))
#define MS_TO_SAMPLES(MS) static_cast<uint32_t>(static_cast<uint64_t>(MS) * ISoundDecoder::kSampleRate / 1000u)
#define SAMPLES_TO_MS(SAMPLES) static_cast<uint32_t>(static_cast<uint64_t>((SAMPLES) * 1000) / ISoundDecoder::kSampleRate)

namespace
{
    inline array<float, 2> CalculatePanScale(float pan) noexcept
    {
        return { (pan <= 0.f ? 1.f : 1.f - pan), (pan >= 0.f ? 1.f : 1.f + pan) };
    }
}

AudioEngine::AudioEngine()
{
#ifdef LSTG_DEVELOPMENT
    m_stUpdateTime.store(0, std::memory_order_release);
#endif

#ifndef LSTG_AUDIO_SINGLE_THREADED
    m_bMixerStopNotifier.store(false, std::memory_order_release);
    m_bMixerThreadReady.store(false, std::memory_order_release);
    m_stMixerThread = thread([this]() {
        LSTG_LOG_TRACE_CAT(AudioEngine, "Mixer thread created");

        // 初始化音频设备
        std::shared_ptr<detail::AudioDevice> device;
        try
        {
            device = make_shared<detail::AudioDevice>();
            device->SetStreamingCallback([this]() { return RenderAudio(); });
            device->Start();
            m_bMixerThreadReady.store(true, std::memory_order_release);
        }
        catch (...)
        {
            m_stMixerThreadException = std::current_exception();
            m_bMixerThreadReady.store(true, std::memory_order_release);
            return;
        }
        LSTG_LOG_TRACE_CAT(AudioEngine, "Audio device created");

        // 进入音频更新循环
        while (!m_bMixerStopNotifier.load(std::memory_order_acquire))
        {
            auto busy = device->Update();
            if (busy)
                continue;

            // 无工作时睡眠 5ms
            std::this_thread::sleep_for(std::chrono::microseconds(5));
        }

        LSTG_LOG_TRACE_CAT(AudioEngine, "Mixer thread exit");
    });

    // 忙等待线程初始化完毕
    while (!m_bMixerThreadReady.load(memory_order_acquire))
        std::this_thread::yield();

    // 检查是否有异常
    if (m_stMixerThreadException)
    {
        if (m_stMixerThread.joinable())
            m_stMixerThread.join();
        std::rethrow_exception(m_stMixerThreadException);
    }
#else
    // 初始化音频设备
    m_pDevice = make_shared<detail::AudioDevice>();
    m_pDevice->SetStreamingCallback([this]() { return RenderAudio(); });
    m_pDevice->Start();
    LSTG_LOG_TRACE_CAT(AudioEngine, "Audio device created");
#endif

    // 初始化 Bus 更新顺序
    for (size_t i = 0; i < kBusChannelCount; ++i)
        m_stBusesUpdateList[i] = i;

    // 初始化 FreeList
    m_stFreeSources.reserve(kSoundSourceCount);
    for (size_t i = 0; i < kSoundSourceCount; ++i)
        m_stFreeSources.push_back(i);
}

AudioEngine::~AudioEngine()
{
#ifndef LSTG_AUDIO_SINGLE_THREADED
    // 等待混音线程终止
    m_bMixerStopNotifier.store(true, std::memory_order_release);
    if (m_stMixerThread.joinable())
        m_stMixerThread.join();
#endif
}

// <editor-fold desc="Bus 通道控制">

bool AudioEngine::BusIsMuted(BusId id) const noexcept
{
    if (id >= kBusChannelCount)
    {
        assert(false);
        return false;
    }
    else
    {
        return m_stBuses[id].Muted.load(std::memory_order_relaxed);
    }
}

void AudioEngine::BusSetMuted(BusId id, bool muted) noexcept
{
    if (id < kBusChannelCount)
        m_stBuses[id].Muted.store(muted, std::memory_order_relaxed);
}

float AudioEngine::BusGetVolume(BusId id) const noexcept
{
    if (id >= kBusChannelCount)
    {
        assert(false);
        return false;
    }
    else
    {
        return m_stBuses[id].Volume.load(std::memory_order_relaxed);
    }
}

void AudioEngine::BusSetVolume(BusId id, float value) noexcept
{
    if (id < kBusChannelCount)
        m_stBuses[id].Volume.store(CLAMP_VOLUME(value), std::memory_order_relaxed);
}

float AudioEngine::BusGetPan(BusId id) const noexcept
{
    if (id >= kBusChannelCount)
    {
        assert(false);
        return false;
    }
    else
    {
        return m_stBuses[id].Pan.load(std::memory_order_relaxed);
    }
}

void AudioEngine::BusSetPan(BusId id, float value) noexcept
{
    if (id < kBusChannelCount)
        m_stBuses[id].Pan.store(CLAMP_PAN(value), std::memory_order_relaxed);
}

size_t AudioEngine::BusGetPluginCount(BusId id) const noexcept
{
    if (id >= kBusChannelCount)
    {
        assert(false);
        return 0u;
    }
    else
    {
        auto& bus = m_stBuses[id];

        LOCK_BUS_SCOPE(bus);
        return bus.PluginList.size();
    }
}

DspPluginPtr AudioEngine::BusGetPlugin(BusId id, size_t index) const noexcept
{
    if (id >= kBusChannelCount)
    {
        assert(false);
        return nullptr;
    }
    else
    {
        auto& bus = m_stBuses[id];

        LOCK_BUS_SCOPE(bus);
        auto& pluginList = bus.PluginList;
        if (index >= pluginList.size())
            return nullptr;
        return pluginList[index];
    }
}

Result<void> AudioEngine::BusInsertPlugin(BusId id, DspPluginPtr plugin, size_t index) noexcept
{
    if (id >= kBusChannelCount)
    {
        assert(false);
        return make_error_code(errc::invalid_argument);
    }
    else
    {
        auto& bus = m_stBuses[id];

        LOCK_BUS_SCOPE(bus);
        auto& pluginList = bus.PluginList;
        try
        {
            if (index >= pluginList.size())
                pluginList.emplace_back(std::move(plugin));
            else
                pluginList.insert(pluginList.begin() + static_cast<ptrdiff_t>(index), std::move(plugin));
        }
        catch (...)
        {
            return make_error_code(errc::not_enough_memory);
        }
        return {};
    }
}

bool AudioEngine::BusRemovePlugin(BusId id, size_t index) noexcept
{
    if (id >= kBusChannelCount)
    {
        assert(false);
        return false;
    }
    else
    {
        auto& bus = m_stBuses[id];

        LOCK_BUS_SCOPE(bus);
        auto& pluginList = bus.PluginList;
        if (index >= pluginList.size())
            return false;
        pluginList.erase(pluginList.begin() + static_cast<ptrdiff_t>(index));
        return true;
    }
}

size_t AudioEngine::BusGetSendTargetCount(BusId id, BusSendStages stage) const noexcept
{
    if (id >= kBusChannelCount || static_cast<int>(stage) >= static_cast<int>(BusSendStages::Count_))
    {
        assert(false);
        return 0u;
    }
    else
    {
        auto& bus = m_stBuses[id];

        LOCK_BUS_SCOPE(bus);
        auto& sendList = bus.SendList[static_cast<int>(stage)];
        return sendList.size();
    }
}

Result<BusSend> AudioEngine::BusGetSendTarget(BusId id, BusSendStages stage, size_t index) const noexcept
{
    if (id >= kBusChannelCount || static_cast<int>(stage) >= static_cast<int>(BusSendStages::Count_))
    {
        assert(false);
        return make_error_code(errc::invalid_argument);
    }
    else
    {
        auto& bus = m_stBuses[id];

        LOCK_BUS_SCOPE(bus);
        auto& sendList = bus.SendList[static_cast<int>(stage)];
        if (index >= sendList.size())
            return make_error_code(errc::invalid_argument);
        return sendList[index];
    }
}

Result<void> AudioEngine::BusAddSendTarget(BusId id, BusSendStages stage, BusSend send) noexcept
{
    if (id >= kBusChannelCount || static_cast<int>(stage) >= static_cast<int>(BusSendStages::Count_))
    {
        assert(false);
        return make_error_code(errc::invalid_argument);
    }
    else
    {
        auto& bus = m_stBuses[id];

        // 由于需要调整拓扑，这里要对整个音频系统上锁
        LOCK_MASTER_SCOPE;
        //LOCK_BUS_SCOPE(bus);

        auto& sendList = bus.SendList[static_cast<int>(stage)];
        try
        {
            sendList.emplace_back(std::move(send));
        }
        catch (...)
        {
            return make_error_code(errc::not_enough_memory);
        }

        // 重建拓扑
        auto ret = RebuildBusUpdateList();
        if (!ret)
        {
            // rollback
            sendList.pop_back();
            return ret;
        }
        return {};
    }
}

bool AudioEngine::BusRemoveSendTarget(BusId id, BusSendStages stage, size_t index) noexcept
{
    if (id >= kBusChannelCount || static_cast<int>(stage) >= static_cast<int>(BusSendStages::Count_))
    {
        assert(false);
        return false;
    }
    else
    {
        auto& bus = m_stBuses[id];

        LOCK_BUS_SCOPE(bus);
        auto& sendList = bus.SendList[static_cast<int>(stage)];
        if (index >= sendList.size())
            return false;
        sendList.erase(sendList.begin() + static_cast<ptrdiff_t>(index));
        // 删除操作不需要重建更新列表
        return true;
    }
}

Result<void> AudioEngine::BusSetSendTargetVolume(BusId id, BusSendStages stage, size_t index, float volume) noexcept
{
    if (id >= kBusChannelCount || static_cast<int>(stage) >= static_cast<int>(BusSendStages::Count_))
    {
        assert(false);
        return make_error_code(errc::invalid_argument);
    }
    else
    {
        auto& bus = m_stBuses[id];

        LOCK_BUS_SCOPE(bus);
        auto& sendList = bus.SendList[static_cast<int>(stage)];
        if (index >= sendList.size())
            return make_error_code(errc::invalid_argument);
        sendList[index].Volume = volume;
        return {};
    }
}

BusId AudioEngine::BusGetOutputTarget(BusId id) const noexcept
{
    if (id >= kBusChannelCount)
    {
        assert(false);
        return static_cast<size_t>(-1);
    }
    else
    {
        auto& bus = m_stBuses[id];

        LOCK_BUS_SCOPE(bus);
        return bus.OutputTarget;
    }
}

Result<void> AudioEngine::BusSetOutputTarget(BusId id, BusId target) noexcept
{
    if (id >= kBusChannelCount || (target >= kBusChannelCount && target != static_cast<BusId>(-1)))
    {
        assert(false);
        return make_error_code(errc::invalid_argument);
    }
    else
    {
        auto& bus = m_stBuses[id];

        // 由于需要调整拓扑，这里要对整个音频系统上锁
        LOCK_MASTER_SCOPE;
        //LOCK_BUS_SCOPE(bus);

        auto oldTarget = bus.OutputTarget;
        bus.OutputTarget = target;
        auto ret = RebuildBusUpdateList();
        if (!ret)
        {
            // rollback
            bus.OutputTarget = oldTarget;
            return ret;
        }
        return {};
    }
}

float AudioEngine::BusGetPeakVolume(BusId id, size_t channelId) noexcept
{
    if (id >= kBusChannelCount || channelId >= ISoundDecoder::kChannels)
    {
        assert(false);
        return false;
    }
    else
    {
        return m_stBuses[id].PeakVolume[channelId].load(std::memory_order_relaxed);
    }
}

// </editor-fold>

// <editor-fold desc="音频源">

#define CHECK_SOUND_SOURCE(ID) \
    auto sourceIndex = GetIndexFromSourceId(ID);  \
    auto sourceVersion = GetVersionFromSourceId(ID);  \
    if (sourceIndex >= kSoundSourceCount) \
    { \
        assert(false); \
        return make_error_code(errc::invalid_argument); \
    } \
    auto& source = m_stSources[sourceIndex]; \
    if (source.Version.load(std::memory_order_acquire) != sourceVersion) \
        return make_error_code(detail::AudioEngineErrorCodes::SoundSourceAlreadyDisposed)

#define SAFE_LOCK_BUS \
    /* 需要注意的是，此时取到的 BusID 并不一定是期望的 */ \
    /* 仅能保证在前述版本号检查通过时，这里的 BusID 一定是版本号相同时设置过的值，但是可能此时已经发生了修改 */ \
    auto busId = source.BusId.load(std::memory_order_relaxed); \
    assert(busId < kBusChannelCount); \
    auto& bus = m_stBuses[busId]; \
    LOCK_BUS_SCOPE(bus); \
    /* 因此，我们在锁定 Bus 后再次对版本号进行比较 */ \
    if (source.Version.load(std::memory_order_acquire) != sourceVersion) \
        return make_error_code(detail::AudioEngineErrorCodes::SoundSourceAlreadyDisposed); \
    assert(source.BusId.load(std::memory_order_relaxed) == busId)

Result<SoundSourceId> AudioEngine::SourceAdd(BusId id, SoundDataPtr soundData, SoundSourceCreationFlags flags, std::optional<float> volume,
    std::optional<float> pan, std::optional<uint32_t> loopBeginMs, std::optional<uint32_t> loopEndMs) noexcept
{
    if (id >= kBusChannelCount)
    {
        assert(false);
        return make_error_code(errc::invalid_argument);
    }
    else
    {
        auto& bus = m_stBuses[id];
        size_t sourceIndex = 0;

        // 创建音频源
        auto ret = AllocSoundSource();
        if (!ret)
            return ret.GetError();
        sourceIndex = *ret;
        assert(sourceIndex < kSoundSourceCount);

        // 创建 Decoder
        assert(soundData);
        auto decoder = soundData->CreateDecoder();
        if (!decoder)
        {
            FreeSoundSource(sourceIndex);  // rollback
            return decoder.GetError();
        }

        // 获取音频源
        auto& soundSource = m_stSources[sourceIndex];
        uint32_t version = soundSource.Version.load(std::memory_order_acquire);

        // 在 Bus 中注册
        try
        {
            LOCK_BUS_SCOPE(bus);

            bus.Playlists.push_back(sourceIndex);

            soundSource.BusId.store(id, std::memory_order_relaxed);
            soundSource.Decoder = std::move(*decoder);

            // 设置初始 Flags
            auto f = soundSource.Flags.load(std::memory_order_relaxed);
            if (flags & SoundSourceCreationFlags::PlayImmediately)
                f |= SoundSourceFlags::Playing;
            if (flags & SoundSourceCreationFlags::Looping)
                f |= SoundSourceFlags::Looping;
            if (flags & SoundSourceCreationFlags::DisposeAfterStopped)
                f |= SoundSourceFlags::AutoDisposed;
            soundSource.Flags.store(f, std::memory_order_relaxed);

            // 设置初始 Volume
            if (volume)
                soundSource.Volume.store(CLAMP_VOLUME(*volume), std::memory_order_relaxed);
            if (pan)
                soundSource.Pan.store(CLAMP_PAN(*pan), std::memory_order_relaxed);
            if (loopBeginMs)
                soundSource.LoopBeginSamples.store(MS_TO_SAMPLES(*loopBeginMs), std::memory_order_relaxed);
            if (loopEndMs)
                soundSource.LoopEndSamples.store(MS_TO_SAMPLES(*loopEndMs), std::memory_order_relaxed);
        }
        catch (...)
        {
            FreeSoundSource(sourceIndex);  // rollback
            return make_error_code(errc::not_enough_memory);
        }

        return MakeSourceId(sourceIndex, version);
    }
}

Result<void> AudioEngine::SourceDelete(SoundSourceId id) noexcept
{
    CHECK_SOUND_SOURCE(id);

    // 上锁，进行删除操作
    {
        SAFE_LOCK_BUS;

        // 从 Bus 删除
        for (auto it = bus.Playlists.begin(); it != bus.Playlists.end();)
        {
            if (*it == sourceIndex)
            {
                bus.Playlists.erase(it);
                break;
            }
            ++it;
        }

        // 回收
        source.Reset();
        FreeSoundSource(sourceIndex);
        return {};
    }
}

bool AudioEngine::SourceValid(SoundSourceId id) noexcept
{
    auto sourceIndex = GetIndexFromSourceId(id);
    auto version = GetVersionFromSourceId(id);

    // 越界
    if (sourceIndex >= kSoundSourceCount)
    {
        assert(false);
        return false;
    }

    // 如果版本号不相同，直接返回失败
    if (m_stSources[sourceIndex].Version.load(std::memory_order_acquire) != version)
        return false;
    return true;
}

Result<uint32_t> AudioEngine::SourceGetPosition(SoundSourceId id) const noexcept
{
    CHECK_SOUND_SOURCE(id);
    return SAMPLES_TO_MS(source.Position.load(std::memory_order_relaxed));
}

Result<void> AudioEngine::SourceSetPosition(SoundSourceId id, uint32_t ms) noexcept
{
    CHECK_SOUND_SOURCE(id);

    // 上锁，以便进行 Seek 操作
    {
        SAFE_LOCK_BUS;

        // 计算 Seek 位置
        // FIXME: 由于 Decoder 基于时间进行 Seek，这里可能造成 SeekPosition 和实际在 PCM 数据中的位置出现偏差
        auto seekPosition = MS_TO_SAMPLES(ms);

        // 调用 Decoder
        assert(source.Decoder);
        auto ret = source.Decoder->Seek(ms);
        if (!ret)
        {
            LSTG_LOG_WARN_CAT(AudioEngine, "Failed to perform seek operation on source {}", id);
            source.Decoder->Reset();  // Seek 失败时，调用 Reset 重置状态
            seekPosition = 0;
        }

        source.Position.store(seekPosition, std::memory_order_relaxed);
        return {};
    }
}

Result<float> AudioEngine::SourceGetVolume(SoundSourceId id) const noexcept
{
    CHECK_SOUND_SOURCE(id);
    return source.Volume.load(std::memory_order_relaxed);
}

Result<void> AudioEngine::SourceSetVolume(SoundSourceId id, float vol) noexcept
{
    CHECK_SOUND_SOURCE(id);

    // 上锁，防止写入一个已经释放的 Bus
    {
        SAFE_LOCK_BUS;

        source.Volume.store(CLAMP_VOLUME(vol), std::memory_order_relaxed);
        return {};
    }
}

Result<float> AudioEngine::SourceGetPan(SoundSourceId id) const noexcept
{
    CHECK_SOUND_SOURCE(id);
    return source.Pan.load(std::memory_order_relaxed);
}

Result<void> AudioEngine::SourceSetPan(SoundSourceId id, float pan) noexcept
{
    CHECK_SOUND_SOURCE(id);

    // 上锁，防止写入一个已经释放的 Bus
    {
        SAFE_LOCK_BUS;

        source.Pan.store(CLAMP_PAN(pan), std::memory_order_relaxed);
        return {};
    }
}

Result<std::tuple<uint32_t, uint32_t>> AudioEngine::SourceGetLoopRange(SoundSourceId id) const noexcept
{
    CHECK_SOUND_SOURCE(id);
    auto begin = source.LoopBeginSamples.load(std::memory_order_relaxed);
    auto end = source.LoopEndSamples.load(std::memory_order_relaxed);
    return tuple {SAMPLES_TO_MS(begin), SAMPLES_TO_MS(end) };
}

Result<void> AudioEngine::SourceSetLoopRange(SoundSourceId id, uint32_t loopBeginMs, uint32_t loopEndMs) noexcept
{
    CHECK_SOUND_SOURCE(id);

    // 上锁，防止写入一个已经释放的 Bus
    {
        SAFE_LOCK_BUS;

        source.LoopBeginSamples.store(MS_TO_SAMPLES(loopBeginMs), std::memory_order_relaxed);
        source.LoopEndSamples.store(MS_TO_SAMPLES(loopEndMs), std::memory_order_relaxed);
        return {};
    }
}

Result<bool> AudioEngine::SourceIsPlaying(SoundSourceId id) const noexcept
{
    CHECK_SOUND_SOURCE(id);
    auto flag = source.Flags.load(std::memory_order_relaxed);
    return flag & SoundSourceFlags::Playing;
}

Result<void> AudioEngine::SourcePlay(SoundSourceId id) noexcept
{
    CHECK_SOUND_SOURCE(id);

    // 上锁，确保播放操作不发生在音频渲染时
    {
        SAFE_LOCK_BUS;

        auto flag = source.Flags.load(std::memory_order_relaxed);
        if (flag & SoundSourceFlags::Playing)
            return {};

        source.Flags.store(flag | SoundSourceFlags::Playing, std::memory_order_relaxed);
        return {};
    }
}

Result<void> AudioEngine::SourcePause(SoundSourceId id) noexcept
{
    CHECK_SOUND_SOURCE(id);

    // 上锁，确保播放操作不发生在音频渲染时
    {
        SAFE_LOCK_BUS;

        auto flag = source.Flags.load(std::memory_order_relaxed);
        if (!(flag & SoundSourceFlags::Playing))
            return {};

        source.Flags.store(flag ^ SoundSourceFlags::Playing, std::memory_order_relaxed);
        return {};
    }
}

Result<bool> AudioEngine::SourceIsLooping(SoundSourceId id) const noexcept
{
    CHECK_SOUND_SOURCE(id);
    auto flag = source.Flags.load(std::memory_order_acquire);
    return flag & SoundSourceFlags::Looping;
}

Result<void> AudioEngine::SourceSetLooping(SoundSourceId id, bool loop) noexcept
{
    CHECK_SOUND_SOURCE(id);

    // 上锁，确保播放操作不发生在音频渲染时
    {
        SAFE_LOCK_BUS;

        auto flag = source.Flags.load(std::memory_order_relaxed);
        auto newFlag = loop ? (flag | SoundSourceFlags::Looping) : (flag ^ SoundSourceFlags::Looping);

        source.Flags.store(newFlag, std::memory_order_relaxed);
        return {};
    }
}

// </editor-fold>

void AudioEngine::Update(double elapsedTime) noexcept
{
#ifdef LSTG_AUDIO_SINGLE_THREADED
    m_pDevice->Update();
#endif

#ifdef LSTG_DEVELOPMENT
    // 更新时间到 Profile 系统
    auto updateTimeMs = m_stUpdateTime.load(std::memory_order_acquire);
    m_stUpdateTime.store(0, std::memory_order_release);
    ProfileSystem::GetInstance().IncrementPerformanceCounter(lstg::Subsystem::PerformanceCounterTypes::PerFrame, "AudioUpdateTime",
        updateTimeMs / 1000.);
#endif
}

Result<void> AudioEngine::RebuildBusUpdateList() noexcept
{
    detail::StaticTopologicalSorter<kBusChannelCount> sorter;

    // 构建邻接矩阵
    sorter.Reset();
    for (size_t i = 0; i < kBusChannelCount; ++i)
    {
        // 检查 Send
        for (size_t j = 0; j < static_cast<int>(BusSendStages::Count_); ++j)
        {
            // 通道 i 发送到 send.Target
            for (const auto& send : m_stBuses[i].SendList[j])
                sorter.SetAdjacency(i, send.Target);
        }

        // 通道 i 输出到 OutputTarget
        if (m_stBuses[i].OutputTarget != static_cast<BusId>(-1))
            sorter.SetAdjacency(i, m_stBuses[i].OutputTarget);
    }

    // 执行拓扑排序
    if (!sorter.Sort())
    {
        LSTG_LOG_ERROR_CAT(AudioEngine, "Topological sort fail, circular reference detected");
        return make_error_code(detail::AudioEngineErrorCodes::BusChannelCircularSendingDetected);
    }

    // 刷新更新顺序
    for (size_t i = 0; i < kBusChannelCount; ++i)
        m_stBusesUpdateList[i] = sorter.GetResult(i);
    return {};
}

Result<size_t> AudioEngine::AllocSoundSource() noexcept
{
#ifndef LSTG_AUDIO_SINGLE_THREADED
    std::unique_lock<std::mutex> lockGuard(m_stFreeSourcesMutex);
#endif

    if (m_stFreeSources.empty())
        return make_error_code(detail::AudioEngineErrorCodes::NoSoundSourceAvailable);
    auto ret = m_stFreeSources.back();
    m_stFreeSources.pop_back();
    return ret;
}

void AudioEngine::FreeSoundSource(size_t index) noexcept
{
#ifndef LSTG_AUDIO_SINGLE_THREADED
    std::unique_lock<std::mutex> lockGuard(m_stFreeSourcesMutex);
#endif

    try
    {
        m_stFreeSources.push_back(index);  // rollback
    }
    catch (...)
    {
        assert(false);  // 一定不会发生异常
    }
}

SampleView<2> AudioEngine::RenderAudio() noexcept
{
#ifdef LSTG_DEVELOPMENT
    auto beginTime = std::chrono::steady_clock::now();
#endif

    LOCK_MASTER_SCOPE;

    m_stFinalMixBuffer.Clear();
    auto finalMixBufferView = ToSampleView(m_stFinalMixBuffer);

    StaticSampleBuffer<ISoundDecoder::kChannels, BusChannel::kSampleCount> tempBuffer;
    auto tempBufferView = ToSampleView(tempBuffer);

    // 清理每个 BUS 的缓冲区
    for (size_t i = 0; i < kBusChannelCount; ++i)
        m_stBuses[i].MixBuffer.Clear();

    // 更新每一个 BUS
    for (size_t i = 0; i < kBusChannelCount; ++i)
    {
        BusId busId = m_stBusesUpdateList[i];
        assert(busId < kBusChannelCount);
        auto& bus = m_stBuses[busId];
        LOCK_BUS_SCOPE(bus);

        auto mixBufferView = ToSampleView(bus.MixBuffer);

        // 更新 Playlist
        for (auto it = bus.Playlists.begin(); it != bus.Playlists.end(); )
        {
            auto sourceIndex = *it;
            assert(sourceIndex < kSoundSourceCount);
            auto& source = m_stSources[sourceIndex];
            auto sourceVolume = source.Volume.load(std::memory_order_relaxed);
            auto sourcePan = source.Pan.load(std::memory_order_relaxed);
            if (!RenderSoundSource(tempBufferView, source))
            {
                // 删除 SoundSource
                it = bus.Playlists.erase(it);

                // 回收
                source.Reset();
                FreeSoundSource(sourceIndex);
            }
            else
            {
                ++it;
            }

            // 混合
            if (sourceVolume > 0.f)
            {
                if (sourceVolume != 1.f || sourcePan != 0.f)
                {
                    array<float, 2> scale = CalculatePanScale(sourcePan);
                    scale[0] *= sourceVolume;
                    scale[1] *= sourceVolume;
                    mixBufferView.MixSamples(tempBufferView, scale);
                }
                else
                {
                    mixBufferView += tempBufferView;
                }
            }
            tempBuffer.Clear();
        }

        // 调用 FX 效果链
        for (auto& fx : bus.PluginList)
            fx->Process(mixBufferView);

        // 是否静音
        auto muted = bus.Muted.load(std::memory_order_relaxed);
        if (muted)
        {
            for (size_t j = 0; j < ISoundDecoder::kChannels; ++j)
                bus.PeakVolume[j].store(0.f, std::memory_order_relaxed);
            continue;
        }

        // 推子前发送
        RenderSend(finalMixBufferView, bus, BusSendStages::BeforeVolume);

        auto vol = bus.Volume.load(std::memory_order_relaxed);
        auto pan = bus.Pan.load(std::memory_order_relaxed);

        // fast path
        if (bus.SendList[static_cast<int>(BusSendStages::AfterVolume)].empty())
        {
            if (vol != 1.f || pan != 0.f)
            {
                array<float, 2> scale = CalculatePanScale(pan);
                scale[0] *= vol;
                scale[1] *= vol;
                mixBufferView *= scale;
            }
        }
        else
        {
            // 音量调整
            if (vol != 1.f)
                mixBufferView *= vol;

            // 推子后发送
            RenderSend(finalMixBufferView, bus, BusSendStages::AfterVolume);

            // 声像调整
            if (pan != 0.f)
            {
                array<float, 2> scale = CalculatePanScale(pan);
                mixBufferView *= scale;
            }
        }

        // 后声像发送
        RenderSend(finalMixBufferView, bus, BusSendStages::AfterPan);

        // 输出
        if (bus.OutputTarget == static_cast<BusId>(-1))
        {
            finalMixBufferView += mixBufferView;
        }
        else
        {
            assert(bus.OutputTarget < kBusChannelCount);
            auto targetMixBuffer = ToSampleView(m_stBuses[bus.OutputTarget].MixBuffer);
            targetMixBuffer += mixBufferView;
        }

        // 计算峰值音量并保存
        auto peekValue = mixBufferView.GetPeakValue();
        for (size_t j = 0; j < ISoundDecoder::kChannels; ++j)
            bus.PeakVolume[j].store(peekValue[j], std::memory_order_relaxed);
    }

#ifdef LSTG_DEVELOPMENT
    // 统计耗时
    auto endTime = std::chrono::steady_clock::now();
    auto elapsedTime = chrono::duration_cast<chrono::milliseconds>(endTime - beginTime).count();
    m_stUpdateTime.fetch_add(static_cast<uint32_t>(elapsedTime), std::memory_order_acq_rel);
#endif
    return finalMixBufferView;
}

bool AudioEngine::RenderSoundSource(SampleView<ISoundDecoder::kChannels> output, SoundSource& source) noexcept
{
    bool stopped = false;
    auto flags = source.Flags.load(std::memory_order_acquire);

    // 未处在播放模式，跳过
    if (!(flags & SoundSourceFlags::Playing))
        return true;

    // 针对循环特殊处理
    auto position = source.Position.load(std::memory_order_relaxed);
    assert(source.Decoder);
    if (flags & SoundSourceFlags::Looping)
    {
        auto loopBegin = source.LoopBeginSamples.load(std::memory_order_relaxed);
        auto loopEnd = source.LoopEndSamples.load(std::memory_order_relaxed);
        if (loopBegin > loopEnd)
            std::swap(loopBegin, loopEnd);

        // 调整 Seek 位置，防止外部修改了循环节
        if (position > loopEnd)
        {
            auto ret = source.Decoder->Seek(SAMPLES_TO_MS(loopBegin));
            if (!ret)
            {
                // 如果跳到循环头失败，则从头开始
                LSTG_LOG_ERROR_CAT(AudioEngine, "Seek sound source fail: {}", ret.GetError());
                source.Decoder->Reset();
                position = 0;
            }
            else
            {
                position = loopBegin;
            }
            source.Position.store(position, std::memory_order_relaxed);
        }

        // 当且仅当循环节有效时进行处理
        if (loopBegin != loopEnd)
        {
            size_t readStart = 0;
            while (readStart < output.GetSampleCount())
            {
                assert(position <= loopEnd);
                auto samplesRest = loopEnd - position;
                auto readCount = std::min<size_t>(output.GetSampleCount() - readStart, samplesRest);
                auto ret = source.Decoder->Decode(output.Slice(readStart, readStart + readCount));
                if (!ret)
                {
                    // 解码失败，直接停止
                    LSTG_LOG_ERROR_CAT(AudioEngine, "Decode sound source fail: {}", ret.GetError());
                    stopped = true;
                    break;
                }
                else
                {
                    assert(position + *ret <= loopEnd);
                    position += *ret;
                    readStart += *ret;
                }

                // 如果达到了循环尾，跳到循环头
                // 还有一种可能是循环尾超过了音频总长度，此时也要跳到循环头
                if (position >= loopEnd || *ret < readCount)
                {
                    auto seekRet = source.Decoder->Seek(SAMPLES_TO_MS(loopBegin));
                    if (!seekRet)
                    {
                        // 如果跳到循环头失败，则从头开始
                        LSTG_LOG_ERROR_CAT(AudioEngine, "Seek sound source fail: {}", ret.GetError());
                        source.Decoder->Reset();
                        position = 0;
                    }
                    else
                    {
                        position = loopBegin;
                    }
                }
                source.Position.store(position, std::memory_order_relaxed);
            }
        }
    }
    else
    {
        auto ret = source.Decoder->Decode(output);
        if (!ret)
        {
            LSTG_LOG_ERROR_CAT(AudioEngine, "Decode sound source fail: {}", ret.GetError());
            stopped = true;
        }
        else
        {
            source.Position.store(position + *ret, std::memory_order_relaxed);
            if (*ret < output.GetSampleCount())
                stopped = true;
        }
    }

    // 停止状态处理
    if (stopped)
    {
        flags ^= SoundSourceFlags::Playing;
        source.Decoder->Reset();
        source.Position.store(0, std::memory_order_relaxed);
    }
    source.Flags.store(flags, std::memory_order_relaxed);

    // 检查是否需要销毁
    if (flags & SoundSourceFlags::AutoDisposed)
        return !stopped;
    return true;
}

void AudioEngine::RenderSend(const SampleView<ISoundDecoder::kChannels>& finalMixOutput, BusChannel& bus, BusSendStages stages) noexcept
{
    auto& sendList = bus.SendList[static_cast<int>(stages)];
    if (sendList.empty())
        return;

    auto mixInput = ToSampleView(bus.MixBuffer);

    for (auto& send : sendList)
    {
        array<float, ISoundDecoder::kChannels> volumeScale {};
        volumeScale.fill(send.Volume);

        SampleView<ISoundDecoder::kChannels> targetMixBufferView;
        if (send.Target == static_cast<BusId>(-1))
        {
            targetMixBufferView = finalMixOutput;
        }
        else
        {
            assert(send.Target < kBusChannelCount);
            targetMixBufferView = ToSampleView(m_stBuses[send.Target].MixBuffer);
        }

        // 混合
        targetMixBufferView.MixSamples(mixInput, volumeScale);
    }
}
