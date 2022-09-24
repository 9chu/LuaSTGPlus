/**
 * @file
 * @date 2022/9/15
 * @author 9chu
 * 此文件为 LuaSTGPlus 项目的一部分，版权与许可声明详见 COPYRIGHT.txt。
 */
#include "NullSoundDecoder.hpp"

using namespace std;
using namespace lstg;
using namespace lstg::Subsystem::Audio;

NullSoundDecoder::NullSoundDecoder(std::shared_ptr<const SampleView<kChannels>> pcmData)
    : m_pPCMData(std::move(pcmData))
{
    assert(m_pPCMData);
}

Result<size_t> NullSoundDecoder::Decode(SampleView<kChannels> output) noexcept
{
    assert(m_uPosition <= m_pPCMData->GetSampleCount());
    auto restSamples = m_pPCMData->GetSampleCount() - m_uPosition;
    auto readSamples = std::min<size_t>(restSamples, output.GetSampleCount());
    for (size_t i = 0; i < kChannels; ++i)
        ::memcpy(&output[i][0], &m_pPCMData->operator[](i)[m_uPosition], readSamples * sizeof(float));
    m_uPosition += readSamples;
    return readSamples;
}

Result<uint32_t> NullSoundDecoder::GetDuration() noexcept
{
    // 采样转时间
    return static_cast<uint32_t>((m_pPCMData->GetSampleCount() * 1000) / ISoundDecoder::kSampleRate);
}

Result<void> NullSoundDecoder::Seek(uint32_t timeMs) noexcept
{
    // 时间转采样
    auto samples = std::min<size_t>(m_pPCMData->GetSampleCount(), timeMs * ISoundDecoder::kSampleRate / 1000);
    m_uPosition = samples;
    return {};
}

Result<void> NullSoundDecoder::Reset() noexcept
{
    m_uPosition = 0;
    return {};
}
