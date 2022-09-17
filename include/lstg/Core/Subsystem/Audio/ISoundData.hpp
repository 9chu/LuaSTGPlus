/**
 * @file
 * @date 2022/9/13
 * @author 9chu
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#pragma once
#include "ISoundDecoder.hpp"
#include <lstg/Core/Subsystem/VFS/IStream.hpp>

namespace lstg::Subsystem::Audio
{
    /**
     * 音频源
     */
    class ISoundData
    {
    public:
        ISoundData() noexcept = default;
        virtual ~ISoundData() noexcept = default;

    public:
        /**
         * 创建解码器
         * 解码器使用音频数据进行解码，解码器直接互相独立互不干扰。
         */
        virtual Result<SoundDecoderPtr> CreateDecoder() noexcept = 0;
    };

    using SoundDataPtr = std::shared_ptr<ISoundData>;

    /**
     * 创建内存音频数据
     * @param stream 流
     * @return 音频数据
     */
    Result<SoundDataPtr> CreateMemorySoundData(VFS::StreamPtr stream) noexcept;

    /**
     * 创建流音频数据
     * @param stream 流
     * @return 音频数据
     */
    Result<SoundDataPtr> CreateStreamSoundData(VFS::StreamPtr stream) noexcept;
}
