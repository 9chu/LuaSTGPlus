/**
 * @file
 * @date 2022/9/17
 * @author 9chu
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#pragma once
#include <vector>
#include <memory>
#include <lstg/Core/Subsystem/Audio/ISoundData.hpp>
#include "SDLSoundDecoder.hpp"

namespace lstg::Subsystem::Audio
{
    /**
     * 流 PCM 数据源
     */
    class StreamSoundData :
        public ISoundData
    {
    public:
        /**
         * 从文件构造
         * @param stream 音频文件
         */
        StreamSoundData(VFS::StreamPtr stream);

    public:  // ISoundData
        Result<SoundDecoderPtr> CreateDecoder() noexcept override;

    private:
        VFS::StreamPtr m_pStream;
    };
}
