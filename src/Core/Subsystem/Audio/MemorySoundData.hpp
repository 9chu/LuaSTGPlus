/**
 * @file
 * @date 2022/9/13
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
     * 内存 PCM 数据源
     */
    class MemorySoundData :
        public ISoundData,
        public std::enable_shared_from_this<MemorySoundData>
    {
    public:
        /**
         * 从音频文件构造 PCM 数据
         * @param stream 音频文件
         */
        MemorySoundData(VFS::StreamPtr stream);

    public:  // ISoundData
        Result<SoundDecoderPtr> CreateDecoder() noexcept override;

    private:
        std::vector<float> m_stPCMData[ISoundDecoder::kChannels];
        SampleView<ISoundDecoder::kChannels> m_stPCMDataView;
    };
}
