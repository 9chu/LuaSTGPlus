/**
 * @file
 * @author 9chu
 * @date 2022/2/17
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#include <lstg/Core/PreciseSleeper.hpp>

#include <cmath>
#include <thread>
#include <lstg/Core/Pal.hpp>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <timeapi.h>
#ifdef max
#undef max
#endif
#endif

using namespace std;
using namespace lstg;

PreciseSleeper::PreciseSleeper()
    : m_dFreq(static_cast<double>(Pal::GetTickFrequency()))
{
#ifdef _WIN32
    timeBeginPeriod(1);
#endif
}

PreciseSleeper::~PreciseSleeper()
{
#ifdef _WIN32
    timeEndPeriod(1);
#endif
}

void PreciseSleeper::Sleep(double seconds) noexcept
{
    // https://blat-blatnik.github.io/computerBear/making-accurate-sleep-function/
    // 由于 Sleep 并不精确，此处每次 Sleep 1ms
    // 之后会估计单次 Sleep 的实际耗时
    while (seconds > m_dEstimate)
    {
        auto start = Pal::GetCurrentTick();
        this_thread::sleep_for(chrono::milliseconds(1));
        auto end = Pal::GetCurrentTick();

        // 实际睡眠时间
        double observed = std::max(0., static_cast<double>(end - start)) / m_dFreq;

        // 根据实际值估算单次 Sleep 实际时间
        // NOTE: 无限序列求均值
        // mean' = mean + (observed - mean) / (count + 1)
        //       = (mean * count + mean + observed - mean) / (count + 1)
        //       = (mean * count + observed) / (count + 1)
        //       = (sum + observed) / (count + 1)
        m_dCount += 1.0;
        double delta = observed - m_dMean;
        m_dMean += delta / m_dCount;

        // 平方差这个推导有点繁琐，略
        m_dM2 += delta * (observed - m_dMean);
        double stddev = std::sqrt(m_dM2 / (m_dCount - 1));

        // 取上界
        m_dEstimate = m_dMean + stddev;

        seconds -= observed;
    }

    // 对于 Sleep(1ms) 不能保证精度的情况，采取自旋锁进行逼近
    auto start = Pal::GetCurrentTick();
    auto target = seconds * m_dFreq;
    while (static_cast<double>(Pal::GetCurrentTick() - start) < target)
        this_thread::yield();
}
