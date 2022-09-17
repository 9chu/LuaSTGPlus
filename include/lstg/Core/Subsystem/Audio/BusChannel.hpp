/**
 * @file
 * @date 2022/9/3
 * @author 9chu
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#pragma once
#include <vector>
#include "IDspPlugin.hpp"
#include "SampleBuffer.hpp"

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
            kChannelCount = 2,
            kSampleCount = 1024,
        };

        /**
         * 混合缓冲区
         * 默认为双通道，1024 个采样。
         * 在 44100Hz 采样率下大约为 20ms。
         */
        StaticSampleBuffer<kChannelCount, kSampleCount> MixBuffer;

        /**
         * 是否静音
         */
        bool Muted = false;

        /**
         * 音量
         * 线性值 [0, 1]
         */
        float Volume = 0.f;

        /**
         * 平衡
         * [-1, 1]
         */
        float Pan = 0.f;

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
    };
}
