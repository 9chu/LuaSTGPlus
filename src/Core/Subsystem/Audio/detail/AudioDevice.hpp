/**
 * @file
 * @date 2022/9/18
 * @author 9chu
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#pragma once
#include <functional>
#include <lstg/Core/Exception.hpp>
#include <lstg/Core/Subsystem/Audio/SampleView.hpp>
#include "ALHandler.hpp"

namespace lstg::Subsystem::Audio::detail
{
    LSTG_DEFINE_EXCEPTION(AudioDeviceInitializeFailedException);

    /**
     * 音频设备
     */
    class AudioDevice
    {
    public:
        AudioDevice();
        ~AudioDevice();

    public:
        /**
         * 启动 Buffering
         */
        void Start() noexcept;

        /**
         * 暂停 Buffering
         */
        void Pause() noexcept;

        /**
         * 设置流回调
         * @param cb 回调
         */
        void SetStreamingCallback(std::function<const SampleView<2>()> cb) noexcept { m_stStreamingCallback = std::move(cb); }

        /**
         * 更新状态
         * @return 更新的 Buffer 数
         */
        size_t Update() noexcept;

    private:
        enum {
#ifdef LSTG_PLATFORM_EMSCRIPTEN
            kMainBufferCount = 5,
#else
            kMainBufferCount = 3,  // 一个缓冲区1024采样，共计3072采样，在44100Hz下预计产生70ms延迟，可以容忍的计算时间为46ms
#endif
        };

        ALDeviceHandler m_pDevice;
        ALContextHandler m_pContext;
        ALuint m_uMainSourceHandle = 0;
        ALuint m_stMainBuffers[kMainBufferCount] = { 0, 0, 0 };

        bool m_bPlaying = false;
        std::function<const SampleView<2>()> m_stStreamingCallback;
        std::vector<float> m_stSampleBuffer;
    };
}
