/**
 * @file
 * @date 2022/9/3
 * @author 9chu
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#pragma once
#include <optional>
#include "BusChannel.hpp"
#include "ISoundData.hpp"
#include "SoundSource.hpp"

#ifndef LSTG_AUDIO_SINGLE_THREADED
#include <thread>
#endif

namespace lstg::Subsystem::Audio
{
    namespace detail
    {
        class AudioDevice;
    }

    LSTG_FLAG_BEGIN(SoundSourceCreationFlags)
        PlayImmediately = 1,
        Looping = 2,
        DisposeAfterStopped = 4,
    LSTG_FLAG_END(SoundSourceCreationFlags)

    /**
     * 音频引擎
     */
    class AudioEngine
    {
        enum {
            kBusChannelCount = 4,
            kSoundSourceCount = 1024,
        };

    public:
        AudioEngine();
        AudioEngine(const AudioEngine&) = delete;
        AudioEngine(AudioEngine&&) noexcept = delete;
        ~AudioEngine();

    public:  // Bus 通道控制
        /**
         * 检查 Bus 是否静音
         * @param id Bus ID
         * @return 是否静音
         */
        bool BusIsMuted(BusId id) const noexcept;

        /**
         * 设置 Bus 是否静音
         * @param id Bus ID
         * @param muted 是否静音
         */
        void BusSetMuted(BusId id, bool muted) noexcept;

        /**
         * 获取 Bus 音量
         * @param id Bus ID
         * @return 音量
         */
        float BusGetVolume(BusId id) const noexcept;

        /**
         * 设置 Bus 音量
         * @param id Bus ID
         * @param value 值
         */
        void BusSetVolume(BusId id, float value) noexcept;

        /**
         * 获取 Bus 平衡
         * @param id Bus ID
         * @return 值
         */
        float BusGetPan(BusId id) const noexcept;

        /**
         * 设置 Bus 平衡
         * @param id Bus ID
         * @param value 值
         */
        void BusSetPan(BusId id, float value) noexcept;

        /**
         * 获取 Bus 上的插件数
         * @param id Bus ID
         * @return 插件数
         */
        size_t BusGetPluginCount(BusId id) const noexcept;

        /**
         * 获取 Bus 上的插件
         * @param id Bus ID
         * @param index 插件索引
         * @return 插件对象
         */
        DspPluginPtr BusGetPlugin(BusId id, size_t index) const noexcept;

        /**
         * 增加 Bus 上的插件
         * @param id Bus ID
         * @param plugin 插件对象
         * @param index -1 表示在插件链末尾追加，否则指示插入到哪个位置
         * @return 是否成功
         */
        Result<void> BusInsertPlugin(BusId id, DspPluginPtr plugin, size_t index = static_cast<size_t>(-1)) noexcept;

        /**
         * 移除 Bus 上的插件
         * @param id Bus ID
         * @param index 插件的索引
         * @return 是否成功，如果越界则返回 false
         */
        bool BusRemovePlugin(BusId id, size_t index) noexcept;

        /**
         * 获取 Bus 上的发送目标个数
         * @param id Bus ID
         * @return 发送目标个数
         */
        size_t BusGetSendTargetCount(BusId id, BusSendStages stage) const noexcept;

        /**
         * 获取 Bus 上的发送目标
         * @param id Bus ID
         * @param stage 阶段
         * @param index 索引
         * @return 发送目标
         */
        Result<BusSend> BusGetSendTarget(BusId id, BusSendStages stage, size_t index) const noexcept;

        /**
         * 增加 Bus 上的发送目标
         * @param id Bus ID
         * @param stage 阶段
         * @param send 发送目标信息
         * @return 是否成功
         */
        Result<void> BusAddSendTarget(BusId id, BusSendStages stage, BusSend send) noexcept;

        /**
         * 移除 Bus 上的发送目标
         * @param id Bus ID
         * @param stage 阶段
         * @param index 发送目标的索引
         * @return 是否成功
         */
        bool BusRemoveSendTarget(BusId id, BusSendStages stage, size_t index) noexcept;

        /**
         * 获取 Bus 的输出目标
         * @param id Bus ID
         * @return 输出目标
         */
        BusId BusGetOutputTarget(BusId id) const noexcept;

        /**
         * 设置 Bus 的输出目标
         * @param id Bus ID
         * @param target 目标 Bus
         * @return 是否成功
         */
        Result<void> BusSetOutputTarget(BusId id, BusId target) noexcept;

    public:  // 音频源
        /**
         * 添加音频源
         * 音频源添加后会暂停，需要手动播放。
         * @param id Bus ID
         * @param soundData 音频数据
         * @param flags 初始化标记
         * @param volume 可选，初始音量
         * @param pan 可选，初始平衡
         * @param loopBeginMs 可选，初始循环节起始位置
         * @param loopEndMs 可选，初始循环节终止位置
         * @return 音频源ID
         */
        Result<SoundSourceId> SourceAdd(BusId id, SoundDataPtr soundData, SoundSourceCreationFlags flags, std::optional<float> volume = {},
            std::optional<float> pan = {}, std::optional<uint32_t> loopBeginMs = {}, std::optional<uint32_t> loopEndMs = {}) noexcept;

        /**
         * 删除音频源
         * @param id 源ID
         */
        Result<void> SourceDelete(SoundSourceId id) noexcept;

        /**
         * 音频源是否有效
         * @param id 源ID
         */
        bool SourceValid(SoundSourceId id) noexcept;

        /**
         * 获取当前的播放位置
         * @param id 源ID
         * @return 播放位置，毫秒
         */
        Result<uint32_t> SourceGetPosition(SoundSourceId id) const noexcept;

        /**
         * 设置当前的播放位置
         * @param id 源ID
         * @param ms 毫秒
         */
        Result<void> SourceSetPosition(SoundSourceId id, uint32_t ms) noexcept;

        /**
         * 获取音量
         * @param id 源ID
         * @return 音量大小
         */
        Result<float> SourceGetVolume(SoundSourceId id) const noexcept;

        /**
         * 设置音量
         * @param id 源ID
         * @param vol 音量（线性，[0, 1]）
         */
        Result<void> SourceSetVolume(SoundSourceId id, float vol) noexcept;

        /**
         * 获取平衡
         * @param id 源ID
         * @return 平衡
         */
        Result<float> SourceGetPan(SoundSourceId id) const noexcept;

        /**
         * 设置平衡
         * @param id 源ID
         * @param pan 平衡值（[-1, 1]）
         */
        Result<void> SourceSetPan(SoundSourceId id, float pan) noexcept;

        /**
         * 获取循环节
         * @param id 源ID
         * @return 循环节（起始毫秒，终止毫秒）
         */
        Result<std::tuple<uint32_t, uint32_t>> SourceGetLoopRange(SoundSourceId id) const noexcept;

        /**
         * 设置循环节
         * @param id 源ID
         * @param loopBeginMs 循环开始（毫秒）
         * @param loopEndMs 循环终止（毫秒）
         */
        Result<void> SourceSetLoopRange(SoundSourceId id, uint32_t loopBeginMs, uint32_t loopEndMs) noexcept;

        /**
         * 音频源是否正在播放
         * @param id 源ID
         */
        Result<bool> SourceIsPlaying(SoundSourceId id) const noexcept;

        /**
         * 播放音频
         * @param id 源ID
         */
        Result<void> SourcePlay(SoundSourceId id) noexcept;

        /**
         * 暂停音频
         * @param id 源ID
         */
        Result<void> SourcePause(SoundSourceId id) noexcept;

        /**
         * 音频源是否正在循环
         * @param id 源ID
         */
        Result<bool> SourceIsLooping(SoundSourceId id) const noexcept;

        /**
         * 设置音频源是否循环
         * @param id 源ID
         * @param loop 是否循环
         */
        Result<void> SourceSetLooping(SoundSourceId id, bool loop) noexcept;

    public:
        /**
         * 更新状态
         * 总是在主线程调用。
         * @param elapsedTime 流逝时间
         */
        void Update(double elapsedTime) noexcept;

    private:
        Result<void> RebuildBusUpdateList() noexcept;
        Result<size_t> AllocSoundSource() noexcept;
        void FreeSoundSource(size_t index) noexcept;

        SampleView<2> RenderAudio() noexcept;
        bool RenderSoundSource(SampleView<ISoundDecoder::kChannels> output, SoundSource& source) noexcept;
        void RenderSend(const SampleView<ISoundDecoder::kChannels>& finalMixOutput, BusChannel& bus, BusSendStages stages) noexcept;

    private:
#ifndef LSTG_AUDIO_SINGLE_THREADED
        std::atomic<bool> m_bMixerStopNotifier;
        std::atomic<bool> m_bMixerThreadReady;
        std::exception_ptr m_stMixerThreadException;
        std::thread m_stMixerThread;
#else
        std::shared_ptr<detail::AudioDevice> m_pDevice;
#endif

        BusChannel m_stBuses[kBusChannelCount];
        SoundSource m_stSources[kSoundSourceCount];  // 音频源（FreeList）

#ifndef LSTG_AUDIO_SINGLE_THREADED
        mutable std::mutex m_stMasterMutex;  // 主锁，当需要调整或锁定拓扑时上锁，保证在音频渲染时锁定
#endif
        BusId m_stBusesUpdateList[kBusChannelCount];  // 通道更新顺序

#ifndef LSTG_AUDIO_SINGLE_THREADED
        mutable std::mutex m_stFreeSourcesMutex;  // FreeList 锁，当操作 FreeSources 时上锁
#endif
        std::vector<size_t> m_stFreeSources;  // 可用源下标

        StaticSampleBuffer<ISoundDecoder::kChannels, BusChannel::kSampleCount> m_stFinalMixBuffer;
    };
}
