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
        void MixSamples(float* output, const float* input, size_t samples, float scale) noexcept;
        void ScaleSamples(float* data, size_t samples, float scale) noexcept;
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
            : m_uSampleCount(org.m_uSampleCount)
        {
            for (size_t i = 0; i < ChannelCount; ++i)
                m_pChannelData[i] = org.m_pChannelData[i];
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

        SampleView& operator+=(const SampleView& rhs) noexcept
        {
            assert(GetSampleCount() == rhs.GetSampleCount());

            std::array<float, ChannelCount> scale {};
            scale.fill(1.f);

            MixSamples(rhs, scale);
            return *this;
        }

        SampleView& operator*=(float value) noexcept
        {
            for (size_t i = 0; i < ChannelCount; ++i)
                detail::ScaleSamples(operator[](i), GetSampleCount(), value);
            return *this;
        }

        SampleView& operator*=(const std::array<float, ChannelCount>& value) noexcept
        {
            for (size_t i = 0; i < ChannelCount; ++i)
                detail::ScaleSamples(operator[](i), GetSampleCount(), value[i]);
            return *this;
        }

    public:
        /**
         * 获取采样数
         */
        [[nodiscard]] size_t GetSampleCount() const noexcept { return m_uSampleCount; }

        /**
         * 切片
         * @param begin 开始
         * @param end 终止
         */
        SampleView<ChannelCount> Slice(size_t begin, size_t end) noexcept
        {
            begin = std::min(m_uSampleCount, begin);
            end = std::min(m_uSampleCount, end);
            if (begin > end)
                std::swap(begin, end);
            auto len = end - begin;

            std::array<float*, ChannelCount> data {};
            for (size_t i = 0; i < ChannelCount; ++i)
                data[i] = m_pChannelData[i] + begin;
            return { data, len };
        }

        /**
         * 混合采样
         * @param rhs 输入采样
         * @param scale 缩放
         */
        void MixSamples(const SampleView& rhs, std::array<float, ChannelCount>& scale) noexcept
        {
            assert(GetSampleCount() == rhs.GetSampleCount());

            for (size_t i = 0; i < ChannelCount; ++i)
                detail::MixSamples(operator[](i), rhs[i], GetSampleCount(), scale[i]);
        }

        /**
         * 计算峰值（绝对值）
         */
        std::array<float, ChannelCount> GetPeakValue() const noexcept
        {
            std::array<float, ChannelCount> ret {};
            ret.fill(0.f);

            for (size_t i = 0; i < ChannelCount; ++i)
            {
                for (size_t j = 0; j < m_uSampleCount; ++j)
                {
                    float l = ::abs(m_pChannelData[i][j]);
                    if (l > ret[i])
                        ret[i] = l;
                }
            }
            return ret;
        }

        /**
         * 获取通道
         * @param ch 通道 ID
         */
        SampleView<1> GetChannel(size_t ch) const noexcept
        {
            assert(ch < ChannelCount);
            float* arr[1] = { m_pChannelData[ch] };
            return { arr, m_uSampleCount };
        }

    protected:
        size_t m_uSampleCount = 0;
        float* m_pChannelData[ChannelCount];
    };
}
