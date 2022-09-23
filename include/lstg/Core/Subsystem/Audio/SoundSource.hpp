/**
 * @file
 * @date 2022/9/17
 * @author 9chu
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#pragma once
#include "../../Flag.hpp"
#include "ISoundDecoder.hpp"

namespace lstg::Subsystem::Audio
{
    LSTG_FLAG_BEGIN(SoundSourceFlags)
        Playing = 1,
        Looping = 2,
        AutoDisposed = 4,
    LSTG_FLAG_END(SoundSourceFlags)

    /**
     * 声音源
     */
    struct SoundSource
    {
        /**
         * 版本
         * @note 线程安全（AclRel）
         */
        std::atomic<uint32_t> Version;

        /**
         * 关联的 BusId
         * @note 线程安全（Relax）
         */
        std::atomic<size_t> BusId;

        /**
         * 音频源标志位
         * @note 线程安全（Relax + Lock写）
         */
        std::atomic<SoundSourceFlags> Flags;

        /**
         * 数据源解码器
         */
        SoundDecoderPtr Decoder;

        /**
         * 已播放的采样数
         * @note 线程安全（Relax + Lock写）
         */
        std::atomic<uint32_t> Position;

        /**
         * 音量（线性）
         * [0, 1]
         * @note 线程安全（Relax + Lock写）
         */
        std::atomic<float> Volume;

        /**
         * 平衡
         * [-1, 1]
         * @note 线程安全（Relax + Lock写）
         */
        std::atomic<float> Pan;

        /**
         * 循环节开始位置
         * @note 线程安全（Relax + Lock写）
         */
        std::atomic<uint32_t> LoopBeginSamples;

        /**
         * 循环节终止位置
         * @note 线程安全（Relax + Lock写）
         */
        std::atomic<uint32_t> LoopEndSamples;

        SoundSource() noexcept
        {
            Version.store(0, std::memory_order_release);
            BusId.store(0, std::memory_order_relaxed);
            Flags.store(static_cast<SoundSourceFlags>(0), std::memory_order_relaxed);
            Position.store(0, std::memory_order_relaxed);
            Volume.store(1.f, std::memory_order_relaxed);
            Pan.store(0.f, std::memory_order_relaxed);
            LoopBeginSamples.store(0, std::memory_order_relaxed);
            LoopEndSamples.store(std::numeric_limits<uint32_t>::max(), std::memory_order_relaxed);
            Version.fetch_add(1, std::memory_order_acq_rel);
        }

        void Reset() noexcept
        {
            Version.fetch_add(1, std::memory_order_acq_rel);
            BusId.store(0, std::memory_order_relaxed);
            Flags.store(static_cast<SoundSourceFlags>(0), std::memory_order_relaxed);
            Decoder.reset();
            Position.store(0, std::memory_order_relaxed);
            Volume.store(1.f, std::memory_order_relaxed);
            Pan.store(0.f, std::memory_order_relaxed);
            LoopBeginSamples.store(0, std::memory_order_relaxed);
            LoopEndSamples.store(std::numeric_limits<uint32_t>::max(), std::memory_order_relaxed);
            Version.fetch_add(1, std::memory_order_acq_rel);  // 刷两次版本号，不过感觉没啥意义，实际上版本号变化了读端不会关心上述的值
        }
    };

    using SoundSourceId = uint64_t;

    inline uint32_t GetVersionFromSourceId(SoundSourceId id) noexcept
    {
        return static_cast<uint32_t>(id & 0xFFFFFFFFu);
    }

    inline uint32_t GetIndexFromSourceId(SoundSourceId id) noexcept
    {
        return static_cast<uint32_t>((id >> 32u) & 0xFFFFFFFFu);
    }

    inline SoundSourceId MakeSourceId(size_t index, uint32_t version) noexcept
    {
        return static_cast<uint64_t>(static_cast<uint64_t>(index) << 32u) | version;
    }
}
