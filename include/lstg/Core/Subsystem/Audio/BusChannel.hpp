/**
 * @file
 * @date 2022/9/3
 * @author 9chu
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#pragma once
#include <atomic>
#include <vector>
#include "IDspPlugin.hpp"
#include "SampleBuffer.hpp"
#include "ISoundDecoder.hpp"

#ifndef LSTG_AUDIO_SINGLE_THREADED
#ifdef LSTG_PLATFORM_EMSCRIPTEN
#define LSTG_AUDIO_SINGLE_THREADED
#endif
#endif

#ifndef LSTG_AUDIO_SINGLE_THREADED
#include <mutex>
#endif

namespace lstg::Subsystem::Audio
{
    /**
     * 总线 ID
     * -1 用于表示默认输出通道。
     */
    using BusId = size_t;

    enum class BusSendStages
    {
        BeforeVolume = 0,
        AfterVolume,
        AfterPan,
        Count_,
    };

    /**
     * 发送配置
     */
    struct BusSend
    {
        /**
         * 发送目标
         */
        BusId Target = 0;

        /**
         * 音量
         * 线性值，[0, 1]
         */
        float Volume = 1.f;
    };

    /**
     * 总线
     *
     * 对于单个总线来说，音频数据总是经由若干个步骤进行处理：
     *   1. 输入
     *      输入数据汇总自各个发生器（AudioSource）并累加上所有发送自当前 Bus 的音频数据。
     *      输入数据总是被规格化成 32bit 规格化单精度浮点、双通道立体声，并保证采样率和整个总线匹配。
     *   2. FX
     *      输入数据会经由效果器链依次进行处理。
     *   3. 推子前发送
     *      数据将拷贝至其他总线。
     *   4. 推子
     *      进行增益调整（GAIN）。
     *   5. 推子后发送
     *      数据将拷贝至其他总线。
     *   6. 声像
     *      进行平衡调整（PAN）。
     *   7. 后声像发送
     *      数据将拷贝至其他总线。
     *   8. 输出
     *      数据将拷贝至立体声输出或者其他总线。
     * 当总线处于 MUTE 状态时，不产生任何输出。
     */
    struct alignas(16) BusChannel
    {
        enum {
            kSampleCount = 1024,  // 在 44100Hz 下大概是 23ms
        };

#ifndef LSTG_AUDIO_SINGLE_THREADED
        mutable std::mutex Mutex;
#endif

        /**
         * 混合缓冲区
         * 默认为双通道，1024 个采样。
         * 在 44100Hz 采样率下大约为 20ms。
         */
        StaticSampleBuffer<ISoundDecoder::kChannels, kSampleCount> MixBuffer;

        /**
         * 峰值音量
         * = MAX(MixBuffer)
         */
        std::array<std::atomic<float>, ISoundDecoder::kChannels> PeakVolume {};

        /**
         * 播放列表
         */
        std::vector<size_t> Playlists;

        /**
         * 是否静音
         * @note 线程安全（Relax）
         */
        std::atomic<bool> Muted;

        /**
         * 音量
         * 线性值 [0, 1]
         * @note 线程安全（Relax）
         */
        std::atomic<float> Volume;

        /**
         * 平衡
         * [-1, 1]
         * @note 线程安全（Relax）
         */
        std::atomic<float> Pan;

        /**
         * 效果插件
         */
        std::vector<DspPluginPtr> PluginList;

        /**
         * 发送
         */
        std::vector<BusSend> SendList[static_cast<int>(BusSendStages::Count_)];

        /**
         * 输出
         */
        BusId OutputTarget = static_cast<size_t>(-1);

        BusChannel() noexcept
        {
            for (size_t i = 0; i < ISoundDecoder::kChannels; ++i)
                PeakVolume[i].store(0.f, std::memory_order_relaxed);
            Muted.store(false, std::memory_order_release);
            Volume.store(1.f, std::memory_order_release);
            Pan.store(0.f, std::memory_order_release);
        }
    };
}
