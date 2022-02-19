/**
 * @file
 * @author 9chu
 * @date 2022/2/17
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#pragma once
#include <cstdint>

namespace lstg
{
    /**
     * 高精度睡眠器
     */
    class PreciseSleeper
    {
    public:
        PreciseSleeper();
        PreciseSleeper(const PreciseSleeper&) = delete;
        ~PreciseSleeper();

    public:
        /**
         * 获取时钟源频率
         */
        [[nodiscard]] double GetFrequency() const noexcept { return m_dFreq; }

        /**
         * 睡眠
         * @param seconds 时间（秒）
         */
        void Sleep(double seconds) noexcept;

    private:
        const double m_dFreq;
        double m_dEstimate = 5e-3;
        double m_dMean = 5e-3;
        double m_dM2 = 0.0;
        double m_dCount = 1.0;
    };
}
