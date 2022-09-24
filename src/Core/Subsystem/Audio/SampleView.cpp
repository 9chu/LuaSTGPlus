/**
 * @file
 * @date 2022/9/3
 * @author 9chu
 * 此文件为 LuaSTGPlus 项目的一部分，版权与许可声明详见 COPYRIGHT.txt。
 */
#include <lstg/Core/Subsystem/Audio/SampleView.hpp>

#include <SDL_cpuinfo.h>

#if (defined(SSE) || defined(__SSE__) || defined(_M_AMD64))  // M$VC 没有 __SSE__ 定义
#define LSTG_SCALAR_FALLBACK 0
#define LSTG_SSE 1
#elif (defined(__ARM_ARCH) && (__ARM_ARCH >= 8))  // ARMv8 保证有 NEON
#define SCALAR_FALLBACK 0
#define LSTG_NEON 1
#elif (defined(__APPLE__) && defined(__ARM_ARCH) && (__ARM_ARCH >= 7))   // 苹果的 ARMv7 总是有 NEON
#define LSTG_SCALAR_FALLBACK 0
#define LSTG_NEON 1
#elif (defined(__WINDOWS__) || defined(__WINRT__)) && defined(_M_ARM)  // WINRT 总是有 NEON
#define LSTG_SCALAR_FALLBACK 0
#define LSTG_NEON 1
#else
#define LSTG_SCALAR_FALLBACK 1
#endif

// 有些平台没有定义 __ARM_NEON__ 补上
#if (defined(__ARM_ARCH) || defined(_M_ARM))
#if !LSTG_SCALAR_FALLBACK && !defined(__ARM_NEON__)
#define __ARM_NEON__ 1
#define LSTG_NEON 1
#endif
#endif

#ifdef LSTG_SSE
#include <xmmintrin.h>
#endif

#ifdef LSTG_NEON
#include <arm_neon.h>
#endif

using namespace std;
using namespace lstg;
using namespace lstg::Subsystem::Audio;

namespace
{
    void MixSamplesScalar(float* output, const float* input, size_t samples) noexcept
    {
        auto unrolled = samples / 4;
        auto leftover = samples % 4;

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

#ifdef LSTG_NEON
    void MixSamplesNeon(float* output, const float* input, size_t samples) noexcept
    {
        assert(reinterpret_cast<ptrdiff_t>(output) % 16 == 0 && reinterpret_cast<ptrdiff_t>(input) % 16 == 0);
        auto unrolled = samples / 4;
        auto leftover = samples % 4;

        for (size_t i = 0; i < unrolled; ++i, output += 4, input += 4)
        {
            const float32x4_t dataLoadOutput = vld1q_f32(output);
            const float32x4_t dataLoadInput = vld1q_f32(input);
            vst1q_f32(output, vaddq_f32(dataLoadOutput, dataLoadInput));
        }
        for (size_t i = 0; i < leftover; ++i)
        {
            output[0] += input[0];
            output += 1;
            input += 1;
        }
    }
#endif

#ifdef LSTG_SSE
    void MixSamplesSSE(float* output, const float* input, size_t samples) noexcept
    {
        assert(reinterpret_cast<ptrdiff_t>(output) % 16 == 0 && reinterpret_cast<ptrdiff_t>(input) % 16 == 0);
        auto unrolled = samples / 8;
        auto leftover = samples % 8;

        for (size_t i = 0; i < unrolled; ++i, output += 8, input += 8)
        {
            const __m128 dataLoadOutput1 = _mm_load_ps(output);
            const __m128 dataLoadInput1 = _mm_load_ps(input);
            const __m128 dataLoadOutput2 = _mm_load_ps(output + 4);
            const __m128 dataLoadInput2 = _mm_load_ps(input + 4);
            _mm_store_ps(output, _mm_add_ps(dataLoadOutput1, dataLoadInput1));
            _mm_store_ps(output + 4, _mm_add_ps(dataLoadOutput2, dataLoadInput2));
        }
        for (size_t i = 0; i < leftover; ++i)
        {
            output[0] += input[0];
            output += 1;
            input += 1;
        }
    }
#endif

    void MixSamplesScalar(float* output, const float* input, size_t samples, float scale) noexcept
    {
        auto unrolled = samples / 4;
        auto leftover = samples % 4;

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

#ifdef LSTG_NEON
    void MixSamplesNeon(float* output, const float* input, size_t samples, float scale) noexcept
    {
        assert(reinterpret_cast<ptrdiff_t>(output) % 16 == 0 && reinterpret_cast<ptrdiff_t>(input) % 16 == 0);
        auto unrolled = samples / 4;
        auto leftover = samples % 4;

        for (size_t i = 0; i < unrolled; ++i, output += 4, input += 4)
        {
            const float32x4_t dataLoadOutput = vld1q_f32(output);
            const float32x4_t dataLoadInput = vld1q_f32(input);
            vst1q_f32(output, vaddq_f32(dataLoadOutput, vmulq_n_f32(dataLoadInput, scale)));
        }
        for (size_t i = 0; i < leftover; ++i)
        {
            output[0] += input[0] * scale;
            output += 1;
            input += 1;
        }
    }
#endif

#ifdef LSTG_SSE
    void MixSamplesSSE(float* output, const float* input, size_t samples, float scale) noexcept
    {
        assert(reinterpret_cast<ptrdiff_t>(output) % 16 == 0 && reinterpret_cast<ptrdiff_t>(input) % 16 == 0);
        auto unrolled = samples / 8;
        auto leftover = samples % 8;

        const __m128 scalar = _mm_set1_ps(scale);
        for (size_t i = 0; i < unrolled; ++i, output += 8, input += 8)
        {
            const __m128 dataLoadOutput1 = _mm_load_ps(output);
            const __m128 dataLoadInput1 = _mm_load_ps(input);
            const __m128 dataLoadOutput2 = _mm_load_ps(output + 4);
            const __m128 dataLoadInput2 = _mm_load_ps(input + 4);
            _mm_store_ps(output, _mm_add_ps(dataLoadOutput1, _mm_mul_ps(dataLoadInput1, scalar)));
            _mm_store_ps(output + 4, _mm_add_ps(dataLoadOutput2, _mm_mul_ps(dataLoadInput2, scalar)));
        }
        for (size_t i = 0; i < leftover; ++i)
        {
            output[0] += input[0] * scale;
            output += 1;
            input += 1;
        }
    }
#endif
}

void detail::MixSamples(float* output, const float* input, size_t samples, float scale) noexcept
{
#if defined(LSTG_NEON) && LSTG_SCALAR_FALLBACK
    static const bool kNeonSupported = SDL_HasNEON();
#endif

    if (scale == 1.f)
    {
#ifdef LSTG_NEON
#ifdef LSTG_SCALAR_FALLBACK
        if (!kNeonSupported)
        {
            MixSamplesScalar(output, input, samples);
            return;
        }
#endif
        if (reinterpret_cast<ptrdiff_t>(output) % 16 != 0 || reinterpret_cast<ptrdiff_t>(input) % 16 != 0)
            MixSamplesScalar(output, input, samples);
        else
            MixSamplesNeon(output, input, samples);
#elif defined(LSTG_SSE)
        if (reinterpret_cast<ptrdiff_t>(output) % 16 != 0 || reinterpret_cast<ptrdiff_t>(input) % 16 != 0)
            MixSamplesScalar(output, input, samples);
        else
            MixSamplesSSE(output, input, samples);
#else
        MixSamplesScalar(output, input, samples);
#endif
    }
    else
    {
#ifdef LSTG_NEON
#ifdef LSTG_SCALAR_FALLBACK
        if (!kNeonSupported)
        {
            MixSamplesScalar(output, input, samples, scale);
            return;
        }
#endif
        if (reinterpret_cast<ptrdiff_t>(output) % 16 != 0 || reinterpret_cast<ptrdiff_t>(input) % 16 != 0)
            MixSamplesScalar(output, input, samples, scale);
        else
            MixSamplesNeon(output, input, samples, scale);
#elif defined(LSTG_SSE)
        if (reinterpret_cast<ptrdiff_t>(output) % 16 != 0 || reinterpret_cast<ptrdiff_t>(input) % 16 != 0)
            MixSamplesScalar(output, input, samples, scale);
        else
            MixSamplesSSE(output, input, samples, scale);
#else
        MixSamplesScalar(output, input, samples, scale);
#endif
    }
}

namespace
{
    void ScaleSamplesScalar(float* data, size_t samples, float scale) noexcept
    {
        auto unrolled = samples / 4;
        auto leftover = samples % 4;

        for (size_t i = 0; i < unrolled; ++i)
        {
            data[0] *= scale;
            data[1] *= scale;
            data[2] *= scale;
            data[3] *= scale;
            data += 4;
        }
        for (size_t i = 0; i < leftover; ++i)
        {
            data[0] *= scale;
            data += 1;
        }
    }

#ifdef LSTG_NEON
    void ScaleSamplesNeon(float* data, size_t samples, float scale) noexcept
    {
        assert(reinterpret_cast<ptrdiff_t>(data) % 16 == 0);
        auto unrolled = samples / 4;
        auto leftover = samples % 4;

        for (size_t i = 0; i < unrolled; ++i, data += 4)
        {
            const float32x4_t dataLoad = vld1q_f32(data);
            vst1q_f32(data, vmulq_n_f32(dataLoad, scale));
        }
        for (size_t i = 0; i < leftover; ++i)
        {
            data[0] *= scale;
            data += 1;
        }
    }
#endif

#ifdef LSTG_SSE
    void ScaleSamplesSSE(float* data, size_t samples, float scale) noexcept
    {
        assert(reinterpret_cast<ptrdiff_t>(data) % 16 == 0);
        auto unrolled = samples / 8;
        auto leftover = samples % 8;

        const __m128 scalar = _mm_set1_ps(scale);
        for (size_t i = 0; i < unrolled; ++i, data += 8)
        {
            const __m128 dataLoad1 = _mm_load_ps(data);
            const __m128 dataLoad2 = _mm_load_ps(data + 4);
            _mm_store_ps(data, _mm_mul_ps(dataLoad1, scalar));
            _mm_store_ps(data + 4, _mm_mul_ps(dataLoad2, scalar));
        }
        for (size_t i = 0; i < leftover; ++i)
        {
            data[0] *= scale;
            data += 1;
        }
    }
#endif
}

void detail::ScaleSamples(float* data, size_t samples, float scale) noexcept
{
#ifdef LSTG_NEON
#ifdef LSTG_SCALAR_FALLBACK
    static const bool kNeonSupported = SDL_HasNEON();

    if (!kNeonSupported)
    {
        ScaleSamplesScalar(data, samples, scale);
        return;
    }
#endif
    if (reinterpret_cast<ptrdiff_t>(data) % 16 != 0)
        ScaleSamplesScalar(data, samples, scale);
    else
        ScaleSamplesNeon(data, samples, scale);
#elif defined(LSTG_SSE)
    if (reinterpret_cast<ptrdiff_t>(data) % 16 != 0)
        ScaleSamplesScalar(data, samples, scale);
    else
        ScaleSamplesSSE(data, samples, scale);
#else
    ScaleSamplesScalar(data, samples, scale);
#endif
}
