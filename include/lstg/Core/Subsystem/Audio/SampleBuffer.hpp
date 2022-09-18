/**
 * @file
 * @date 2022/9/3
 * @author 9chu
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#pragma once
#include "SampleView.hpp"

namespace lstg::Subsystem::Audio
{
    /**
     * 静态采样缓冲区
     * @tparam ChannelCount
     * @tparam SampleCount
     */
    template <size_t ChannelCount, size_t SampleCount>
    class alignas(16) StaticSampleBuffer
    {
    public:
        StaticSampleBuffer()
        {
            Clear();
        }

        const float* operator[](size_t channel) const noexcept
        {
            assert(channel < ChannelCount);
            return m_stSamples[channel];
        }

        float* operator[](size_t channel) noexcept
        {
            assert(channel < ChannelCount);
            return m_stSamples[channel];
        }

    public:
        /**
         * 获取采样个数
         */
        size_t GetSampleCount() const noexcept { return SampleCount; }

        /**
         * 清空
         */
        void Clear() noexcept
        {
            ::memset(m_stSamples, 0, sizeof(m_stSamples));
        }

    private:
        static_assert((SampleCount * sizeof(float)) % 16 == 0);
        float m_stSamples[ChannelCount][SampleCount];
    };

    template <size_t ChannelCount, size_t SampleCount>
    SampleView<ChannelCount> ToSampleView(StaticSampleBuffer<ChannelCount, SampleCount>& buffer) noexcept
    {
        std::array<float*, ChannelCount> channels;
        for (size_t i = 0; i < ChannelCount; ++i)
            channels[i] = buffer[i];
        return { channels, buffer.GetSampleCount() };
    }
}
