/**
 * @file
 * @date 2022/9/3
 * @author 9chu
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#pragma once
#include "BusChannel.hpp"

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
     * 音频引擎
     */
    class AudioEngine
    {
        enum {
            kBusChannelCount = 4,
        };

    public:

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
         * @return 是否成功
         */
        Result<void> BusSetOutputTarget(BusId id) noexcept;

    private:
        Result<void> RebuildBusUpdateList() noexcept;

    private:
#ifndef LSTG_AUDIO_SINGLE_THREADED
        mutable std::mutex m_stMutex;
#endif
        BusChannel m_stBuses[kBusChannelCount];
        BusId m_stBusesUpdateList[kBusChannelCount];  // 通道更新顺序
    };
}
