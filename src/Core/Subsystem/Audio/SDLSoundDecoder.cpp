/**
 * @file
 * @date 2022/9/13
 * @author 9chu
 * 此文件为 LuaSTGPlus 项目的一部分，版权与许可声明详见 COPYRIGHT.txt。
 */
#include "SDLSoundDecoder.hpp"

#include "../../detail/SDLHelper.hpp"
#include "detail/SDLSoundError.hpp"

using namespace std;
using namespace lstg;
using namespace lstg::Subsystem::Audio;

using namespace lstg::Subsystem;
using namespace lstg::Subsystem::Audio::detail;

static const size_t kBufferSampleCount = 512;  // about 10ms under 44100Hz

namespace
{
    class SDLSoundSubsystemScope
    {
    public:
        SDLSoundSubsystemScope()
        {
            ::Sound_Init();
        }

        ~SDLSoundSubsystemScope()
        {
            ::Sound_Quit();
        }
    };
}

SDLSoundDecoder::SDLSoundDecoder(VFS::StreamPtr stream)
    : m_pStream(std::move(stream))
{
    static SDLSoundSubsystemScope kSDLSoundScope;

    // 构造 RWOps
    lstg::detail::SDLRWOpsPtr rwOps;
    rwOps = lstg::detail::CreateRWOpsFromStream(m_pStream.get());
    if (!rwOps)
        throw system_error(make_error_code(errc::not_enough_memory));

    // 构造 Sound
    Sound_AudioInfo info;
    info.channels = kChannels;
    info.format = AUDIO_F32;
    info.rate = kSampleRate;
    m_pSample = ::Sound_NewSample(rwOps.release(), nullptr, &info, kBufferSampleCount * sizeof(float) * kChannels);  // 4kb buffer
    if (!m_pSample)
        throw system_error(make_error_code(FromErrorString(::Sound_GetError())));
}

SDLSoundDecoder::~SDLSoundDecoder()
{
    ::Sound_FreeSample(m_pSample);
}

Result<size_t> SDLSoundDecoder::Decode(SampleView<2> output) noexcept
{
    size_t currentSample = 0;
    auto samplesToFill = output.GetSampleCount() - currentSample;
    while (samplesToFill > 0)
    {
        // 拷贝解码结果
        if (m_uRestSamples > 0)
        {
            auto* samples = reinterpret_cast<const float*>(static_cast<const uint8_t*>(m_pSample->buffer) +
                (m_pSample->buffer_size - m_uRestSamples * (sizeof(float) * kChannels)));
            auto readSampleCount = std::min(samplesToFill, m_uRestSamples);
            for (size_t i = 0; i < readSampleCount; ++i)
            {
                for (size_t j = 0; j < kChannels; ++j)
                    output[j][currentSample] = *(samples++);
                ++currentSample;
            }
            m_uRestSamples -= readSampleCount;
            samplesToFill -= readSampleCount;
        }

        // 读取了所有的采样，终止
        if (samplesToFill == 0)
            break;
        assert(m_uRestSamples == 0);  // 没有采样可供使用

        // 调用解码方法
        auto decoded = ::Sound_Decode(m_pSample);
        if (decoded > 0)
        {
            assert(decoded % (sizeof(float) * kChannels) == 0);  // 一定是 8 的倍数
            m_uRestSamples = decoded / (sizeof(float) * kChannels);
            continue;
        }

        // 如果失败，直接返回
        if (m_pSample->flags & SOUND_SAMPLEFLAG_ERROR)
            return make_error_code(FromErrorString(::Sound_GetError()));

        // 如果 EOF
        if (m_pSample->flags & SOUND_SAMPLEFLAG_EOF)
            break;

        // 此时只可能是 EAGAIN
        assert(m_pSample->flags & SOUND_SAMPLEFLAG_EAGAIN);
    }
    return currentSample;
}

Result<uint32_t> SDLSoundDecoder::GetDuration() noexcept
{
    auto duration = ::Sound_GetDuration(m_pSample);
    if (duration < 0)
        return make_error_code(FromErrorString(::Sound_GetError()));
    return static_cast<uint32_t>(duration);
}

Result<void> SDLSoundDecoder::Seek(uint32_t timeMs) noexcept
{
    m_uRestSamples = 0;
    if (0 == ::Sound_Seek(m_pSample, timeMs))
        return make_error_code(FromErrorString(::Sound_GetError()));
    return {};
}

Result<void> SDLSoundDecoder::Reset() noexcept
{
    m_uRestSamples = 0;
    if (0 == ::Sound_Rewind(m_pSample))
        return make_error_code(FromErrorString(::Sound_GetError()));
    return {};
}
