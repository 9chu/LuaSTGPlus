/**
 * @file
 * @date 2022/9/15
 * @author 9chu
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#pragma once
#include <lstg/Core/Subsystem/Audio/ISoundDecoder.hpp>

namespace lstg::Subsystem::Audio
{
    /**
     * 空音频解码器
     * 用于直接推送原始 PCM 数据。
     */
    class NullSoundDecoder :
        public ISoundDecoder
    {
    public:
        /**
         * 构造空解码器
         * @param pcmData PCM数据（必须满足 44100Hz 采样率）
         */
        NullSoundDecoder(std::shared_ptr<const SampleView<kChannels>> pcmData);

    public:  // ISoundDecoder
        Result<size_t> Decode(SampleView<kChannels> output) noexcept;
        Result<uint32_t> GetDuration() noexcept;
        Result<void> Seek(uint32_t timeMs) noexcept;
        Result<void> Reset() noexcept;

    private:
        std::shared_ptr<const SampleView<kChannels>> m_pPCMData;
        size_t m_uPosition = 0;
    };
}
