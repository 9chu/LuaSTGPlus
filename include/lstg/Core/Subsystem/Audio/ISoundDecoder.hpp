/**
 * @file
 * @date 2022/9/13
 * @author 9chu
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#pragma once
#include <memory>
#include "../../Result.hpp"
#include "SampleView.hpp"

namespace lstg::Subsystem::Audio
{
    /**
     * 音频解码器
     * 音频解码器负责将音频数据进行解码。
     * 内部统一使用 44100Hz 双通道 32bit float PCM 数据。
     */
    class ISoundDecoder
    {
    public:
        static const unsigned kChannels = 2;
        static const unsigned kSampleRate = 44100;

    public:
        ISoundDecoder() = default;
        virtual ~ISoundDecoder() noexcept = default;

    public:
        /**
         * 填充缓冲区
         * @param output 输出缓冲区
         * @return 产生的采样个数
         */
        virtual Result<size_t> Decode(SampleView<kChannels> output) noexcept = 0;

        /**
         * 获取时间
         * @return 毫秒
         */
        virtual Result<uint32_t> GetDuration() noexcept = 0;

        /**
         * 转到指定时间
         * @param timeMs 时间（毫秒）
         */
        virtual Result<void> Seek(uint32_t timeMs) noexcept = 0;

        /**
         * 重置解码状态
         */
        virtual Result<void> Reset() noexcept = 0;
    };

    using SoundDecoderPtr = std::shared_ptr<ISoundDecoder>;
}
