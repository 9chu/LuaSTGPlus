/**
 * @file
 * @date 2022/9/18
 * @author 9chu
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#include "AudioDevice.hpp"

#include <lstg/Core/Subsystem/Audio/ISoundDecoder.hpp>
#include <lstg/Core/Subsystem/Audio/BusChannel.hpp>

#ifndef AL_FORMAT_STEREO_FLOAT32
#define AL_FORMAT_STEREO_FLOAT32 0x10011
#endif

using namespace std;
using namespace lstg;
using namespace lstg::Subsystem::Audio::detail;

AudioDevice::AudioDevice()
{
    // 扩展
    // FIXME: 支持常规的 SINT16
    auto extensions = ::alGetString(AL_EXTENSIONS);
    if (::strstr(extensions, "AL_EXT_FLOAT32") == nullptr)
        LSTG_THROW(AudioDeviceInitializeFailedException, "AL_EXT_FLOAT32 extension is required");

    // 创建设备
    m_pDevice.reset(::alcOpenDevice(nullptr));
    if (!m_pDevice)
        LSTG_THROW(AudioDeviceInitializeFailedException, "alcOpenDevice failed to create audio device");

    // 创建上下文
    const ALCint attr[] = { ALC_FREQUENCY, ISoundDecoder::kSampleRate, 0 };
    m_pContext.reset(::alcCreateContext(m_pDevice.get(), attr));
    if (!m_pContext)
        LSTG_THROW(AudioDeviceInitializeFailedException, "alcCreateContext failed, alcGetError={}", alcGetError(m_pDevice.get()));

    // 设置上下文
    if (!alcMakeContextCurrent(m_pContext.get()))
        LSTG_THROW(AudioDeviceInitializeFailedException, "alcMakeContextCurrent failed, alcGetError={}", alcGetError(m_pDevice.get()));

    // 创建主发生源
    ::alGenSources(1, &m_uMainSourceHandle);
    if (!m_uMainSourceHandle)
        LSTG_THROW(AudioDeviceInitializeFailedException, "alGenSources failed, alGetError={}", alGetError());
    ::alSourcef(m_uMainSourceHandle, AL_GAIN, 1.f);
    ::alSource3f(m_uMainSourceHandle, AL_POSITION, 0.f, 0.f, 0.f);
    ::alSource3f(m_uMainSourceHandle, AL_VELOCITY, 0.f, 0.f, 0.f);
    ::alSourcei(m_uMainSourceHandle, AL_LOOPING, AL_TRUE);

    // 创建主 Buffer
    ::alGenBuffers(std::extent_v<decltype(m_stMainBuffers)>, m_stMainBuffers);

    // 设置初始数据
    m_stSampleBuffer.resize(2 * BusChannel::kSampleCount);
    for (auto i : m_stMainBuffers)
    {
        if (!i)
            LSTG_THROW(AudioDeviceInitializeFailedException, "alGenBuffers failed, alGetError={}", alGetError());
        ::alBufferData(i, AL_FORMAT_STEREO_FLOAT32, m_stSampleBuffer.data(), static_cast<ALint>(m_stSampleBuffer.size() * sizeof(float)),
            ISoundDecoder::kSampleRate);
    }

    // 绑定缓冲区
    ::alSourceQueueBuffers(m_uMainSourceHandle, kMainBufferCount, m_stMainBuffers);
}

AudioDevice::~AudioDevice()
{
    ::alDeleteBuffers(std::extent_v<decltype(m_stMainBuffers)>, m_stMainBuffers);
    ::alDeleteSources(1, &m_uMainSourceHandle);
    ::alcMakeContextCurrent(nullptr);
}

void AudioDevice::Start() noexcept
{
    ALint state = 0;
    ::alGetSourcei(m_uMainSourceHandle, AL_SOURCE_STATE, &state);

    if (state != AL_PLAYING)
        ::alSourcePlay(m_uMainSourceHandle);
    m_bPlaying = true;
}

void AudioDevice::Pause() noexcept
{
    ALint state = 0;
    ::alGetSourcei(m_uMainSourceHandle, AL_SOURCE_STATE, &state);

    if (state == AL_PLAYING)
        ::alSourcePause(m_uMainSourceHandle);
    m_bPlaying = false;
}

size_t AudioDevice::Update() noexcept
{
    // 获取当前已经处理的 Buffer 数量
    ALint buffersProcessed = 0;
    ::alGetSourcei(m_uMainSourceHandle, AL_BUFFERS_PROCESSED, &buffersProcessed);
    if (buffersProcessed <= 0)
        return 0;

    size_t cnt = 0;
    while (--buffersProcessed)
    {
        ALuint processedBuffer = 0;
        ::alSourceUnqueueBuffers(m_uMainSourceHandle, 1, &processedBuffer);
        assert(processedBuffer != 0);

        // 如果没有提供 Feed 方法，则用空白数据填充
        if (!m_stStreamingCallback)
        {
            m_stSampleBuffer.resize(2 * BusChannel::kSampleCount);
            ::memset(m_stSampleBuffer.data(), 0, sizeof(float) * m_stSampleBuffer.size());
        }
        else
        {
            auto feedBuffer = m_stStreamingCallback();

            m_stSampleBuffer.resize(2 * feedBuffer.GetSampleCount());
            ::memset(m_stSampleBuffer.data(), 0, sizeof(float) * m_stSampleBuffer.size());

            // 将双通道数据交错拷贝
            for (size_t i = 0; i < feedBuffer.GetSampleCount(); ++i)
            {
                m_stSampleBuffer[i * 2] = feedBuffer[0][i];  // L
                m_stSampleBuffer[i * 2 + 1] = feedBuffer[1][i];  // R
            }
        }

        // 提交缓冲区
        ::alBufferData(processedBuffer, AL_FORMAT_STEREO_FLOAT32, m_stSampleBuffer.data(),
            static_cast<ALint>(m_stSampleBuffer.size() * sizeof(float)), ISoundDecoder::kSampleRate);
        ::alSourceQueueBuffers(m_uMainSourceHandle, 1, &processedBuffer);

        ++cnt;
    }

    // 检查状态
    if (m_bPlaying)
    {
        ALint state = 0;
        ::alGetSourcei(m_uMainSourceHandle, AL_SOURCE_STATE, &state);

        if (state != AL_PLAYING)
            ::alSourcePlay(m_uMainSourceHandle);
    }
    return cnt;
}
