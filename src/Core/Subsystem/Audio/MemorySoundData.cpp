/**
 * @file
 * @date 2022/9/13
 * @author 9chu
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#include "MemorySoundData.hpp"

#include "SDLSoundDecoder.hpp"
#include "NullSoundDecoder.hpp"

using namespace std;
using namespace lstg;
using namespace lstg::Subsystem::Audio;

MemorySoundData::MemorySoundData(VFS::StreamPtr stream)
{
    // 在这里进行预解码
    SDLSoundDecoder decoder(std::move(stream));

    // 猜测长度，进行内存预分配
    auto duration = decoder.GetDuration();
    if (duration)
    {
        auto samples = duration * ISoundDecoder::kSampleRate / 1000;
        for (size_t i = 0; i < ISoundDecoder::kChannels; ++i)
            m_stPCMData[i].resize(samples);

        // 在预分配完成的情况下，直接进行解码操作
        auto decoded = decoder.Decode(m_stPCMData).ThrowIfError();
        for (size_t i = 0; i < ISoundDecoder::kChannels; ++i)
            m_stPCMData[i].resize(decoded);
        if (decoded < samples)
        {
            m_stPCMDataView = m_stPCMData;
            return;
        }
    }

    // 不能进行长度猜测或者长度猜测不准确
    while (true)
    {
        static const size_t kExpandSamples = 4096;
        auto decodedCount = m_stPCMData[0].size();
        auto expandCount = decodedCount + kExpandSamples;
        float* channels[ISoundDecoder::kChannels];
        for (size_t i = 0; i < ISoundDecoder::kChannels; ++i)
        {
            m_stPCMData[i].resize(expandCount);
            channels[i] = m_stPCMData[i].data() + decodedCount;
        }

        // 解码
        auto decoded = decoder.Decode(SampleView<ISoundDecoder::kChannels>{channels, kExpandSamples}).ThrowIfError();
        auto reservedCount = decodedCount + decoded;
        for (size_t i = 0; i < ISoundDecoder::kChannels; ++i)
            m_stPCMData[i].resize(reservedCount);
        if (decoded < kExpandSamples)
        {
            m_stPCMDataView = m_stPCMData;
            break;
        }
    }
}

Result<SoundDecoderPtr> MemorySoundData::CreateDecoder() noexcept
{
    try
    {
        shared_ptr<const SampleView<ISoundDecoder::kChannels>> pcmData(shared_from_this(), &m_stPCMDataView);
        return make_shared<NullSoundDecoder>(std::move(pcmData));
    }
    catch (const std::bad_weak_ptr&)
    {
        assert(false);
        return make_error_code(errc::not_enough_memory);
    }
    catch (...)
    {
        return make_error_code(errc::not_enough_memory);
    }
}

Result<SoundDataPtr> Subsystem::Audio::CreateMemorySoundData(VFS::StreamPtr stream) noexcept
{
    try
    {
        assert(stream);

        auto seekableStream = ConvertToSeekableStream(std::move(stream));
        if (!seekableStream)
            return seekableStream.GetError();

        return make_shared<MemorySoundData>(std::move(*seekableStream));
    }
    catch (const std::system_error& ex)
    {
        return ex.code();
    }
    catch (...)
    {
        return make_error_code(errc::not_enough_memory);
    }
}
