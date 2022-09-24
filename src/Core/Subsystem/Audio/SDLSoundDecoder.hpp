/**
 * @file
 * @date 2022/9/13
 * @author 9chu
 * 此文件为 LuaSTGPlus 项目的一部分，版权与许可声明详见 COPYRIGHT.txt。
 */
#pragma once
#include <SDL_sound.h>
#include <lstg/Core/Subsystem/VFS/IStream.hpp>
#include <lstg/Core/Subsystem/Audio/ISoundDecoder.hpp>

namespace lstg::Subsystem::Audio
{
    /**
     * SDL_Sound 解码器
     */
    class SDLSoundDecoder :
        public ISoundDecoder
    {
    public:
        /**
         * 从流构造 Decoder
         * @param stream 流
         */
        SDLSoundDecoder(VFS::StreamPtr stream);
        SDLSoundDecoder(const SDLSoundDecoder&) = delete;
        SDLSoundDecoder(SDLSoundDecoder&&) noexcept = delete;
        ~SDLSoundDecoder();

    public:  // ISoundDecoder
        Result<size_t> Decode(SampleView<kChannels> output) noexcept override;
        Result<uint32_t> GetDuration() noexcept override;
        Result<void> Seek(uint32_t timeMs) noexcept override;
        Result<void> Reset() noexcept override;

    private:
        VFS::StreamPtr m_pStream;
        Sound_Sample* m_pSample = nullptr;
        size_t m_uRestSamples = 0;  // 在 m_pSample 中剩余未读取的采样数
    };
}
