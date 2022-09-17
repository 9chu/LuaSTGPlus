/**
 * @file
 * @date 2022/9/3
 * @author 9chu
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#pragma once
#include <cassert>
#include <cstdlib>
#include <cstring>
#include <array>
#include <vector>

namespace lstg::Subsystem::Audio
{
    namespace detail
    {
        void MixSamples(float* output, const float* input, size_t samples) noexcept;
        void ScaleSamples(float* output, float scale, size_t samples) noexcept;
    }

    /**
     * 采样视图
     * @tparam ChannelCount 通道数量
     */
    template <size_t ChannelCount>
    class SampleView
    {
    public:
        SampleView() noexcept
        {
            ::memset(m_pChannelData, 0, sizeof(m_pChannelData));
        }

        SampleView(float* (&data)[ChannelCount], size_t count) noexcept
            : m_uSampleCount(count)
        {
            for (size_t i = 0; i < ChannelCount; ++i)
                m_pChannelData[i] = data[i];
        }

        SampleView(std::array<float*, ChannelCount> data, size_t count) noexcept
            : m_uSampleCount(count)
        {
            for (size_t i = 0; i < ChannelCount; ++i)
                m_pChannelData[i] = data[i];
        }

        SampleView(std::vector<float> (&data)[ChannelCount]) noexcept
        {
            m_uSampleCount = data[0].size();
            for (size_t i = 0; i < ChannelCount; ++i)
            {
                m_pChannelData[i] = data[i].data();
                assert(data[i].size() == m_uSampleCount);
            }
        }

        SampleView(const SampleView& org) noexcept
            : m_uSampleCount(org.m_uSampleCount), m_pChannelData(org.m_pChannelData)
        {
        }

        SampleView& operator=(const SampleView& rhs) noexcept
        {
            if (this == &rhs)
                return *this;
            m_uSampleCount = rhs.m_uSampleCount;
            for (size_t i = 0; i < ChannelCount; ++i)
                m_pChannelData[i] = rhs.m_pChannelData[i];
            return *this;
        }

        const float* operator[](size_t channel) const noexcept
        {
            assert(channel < ChannelCount);
            return m_pChannelData[channel];
        }

        float* operator[](size_t channel) noexcept
        {
            assert(channel < ChannelCount);
            return m_pChannelData[channel];
        }

        SampleView& operator+=(const SampleView<1>& rhs) noexcept
        {
            assert(GetSampleCount() == rhs.GetSampleCount());

            for (size_t i = 0; i < ChannelCount; ++i)
                detail::MixSamples(operator[](i), rhs[0], GetSampleCount());
            return *this;
        }

        template <size_t Cnt>
        SampleView& operator+=(std::enable_if_t<(Cnt > 1), const SampleView&> rhs) noexcept
        {
            assert(GetSampleCount() == rhs.GetSampleCount());

            for (size_t i = 0; i < ChannelCount; ++i)
                detail::MixSamples(operator[](i), rhs[i], GetSampleCount());
            return *this;
        }

        SampleView& operator*=(float value) noexcept
        {
            for (size_t i = 0; i < ChannelCount; ++i)
                detail::ScaleSamples(operator[](i), value, GetSampleCount());
            return *this;
        }

        SampleView& operator*=(const std::array<float, ChannelCount>& value) noexcept
        {
            for (size_t i = 0; i < ChannelCount; ++i)
                detail::ScaleSamples(operator[](i), value[i], GetSampleCount());
            return *this;
        }

    public:
        /**
         * 获取采样数
         */
        [[nodiscard]] size_t GetSampleCount() const noexcept { return m_uSampleCount; }

    protected:
        size_t m_uSampleCount = 0;
        float* m_pChannelData[ChannelCount];
    };
}
