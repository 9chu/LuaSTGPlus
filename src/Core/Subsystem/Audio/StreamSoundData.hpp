/**
 * @file
 * @date 2022/9/17
 * @author 9chu
 * 此文件为 LuaSTGPlus 项目的一部分，版权与许可声明详见 COPYRIGHT.txt。
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
