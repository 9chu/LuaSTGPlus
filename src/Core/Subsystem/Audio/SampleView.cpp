/**
 * @file
 * @date 2022/9/3
 * @author 9chu
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#include <lstg/Core/Subsystem/Audio/SampleView.hpp>

using namespace std;
using namespace lstg;
using namespace lstg::Subsystem::Audio;

void detail::MixSamples(float* output, const float* input, size_t samples, float scale) noexcept
{
    auto unrolled = samples / 4;
    auto leftover = samples % 4;

    // TODO: SSE \ NEON
    if (scale == 1.f)
    {
        for (size_t i = 0; i < unrolled; ++i)
        {
            output[0] += input[0];
            output[1] += input[1];
            output[2] += input[2];
            output[3] += input[3];
            output += 4;
            input += 4;
        }
        for (size_t i = 0; i < leftover; ++i)
        {
            output[0] += input[0];
            output += 1;
            input += 1;
        }
    }
    else
    {
        for (size_t i = 0; i < unrolled; ++i)
        {
            output[0] += input[0] * scale;
            output[1] += input[1] * scale;
            output[2] += input[2] * scale;
            output[3] += input[3] * scale;
            output += 4;
            input += 4;
        }
        for (size_t i = 0; i < leftover; ++i)
        {
            output[0] += input[0] * scale;
            output += 1;
            input += 1;
        }
    }
}

void detail::ScaleSamples(float* output, float scale, size_t samples) noexcept
{
    auto unrolled = samples / 4;
    auto leftover = samples % 4;

    // TODO: SSE \ NEON
    for (size_t i = 0; i < unrolled; ++i)
    {
        output[0] *= scale;
        output[1] *= scale;
        output[2] *= scale;
        output[3] *= scale;
        output += 4;
    }
    for (size_t i = 0; i < leftover; ++i)
    {
        output[0] *= scale;
        output += 1;
    }
}
