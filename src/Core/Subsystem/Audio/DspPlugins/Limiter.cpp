/**
 * @file
 * @date 2022/9/20
 * @author 9chu
 * 此文件为 LuaSTGPlus 项目的一部分，版权与许可声明详见 COPYRIGHT.txt。
 */
#include "Limiter.hpp"

#include <lstg/Core/Math/Decibel.hpp>

using namespace std;
using namespace lstg;
using namespace lstg::Subsystem::Audio::DspPlugins;

using namespace lstg::Subsystem::Audio;

#define ID_CEILING_DB "CeilingDb"
#define ID_THRESHOLD_DB "ThresholdDb"
#define ID_SOFT_CLIP_DB "SoftClipDb"
#define ID_SOFT_CLIP_RATIO "SoftClipRatio"

static const DspPluginParameterInfo kParameterInfo[] = {
    {
        ID_CEILING_DB,
        "Ceiling DB",
        DspPluginSliderParameter { -20.f, -0.1f },
    },
    {
        ID_THRESHOLD_DB,
        "Threshold DB",
        DspPluginSliderParameter { -30.f, 0.f },
    },
    {
        ID_SOFT_CLIP_DB,
        "Soft Clip DB",
        DspPluginSliderParameter { 0.f, 6.f },
    },
    {
        ID_SOFT_CLIP_RATIO,
        "Soft Clip Ratio",
        DspPluginSliderParameter { 3.f, 20.f },
    },
};

const char* Limiter::GetNameStatic() noexcept
{
    return "Limiter";
}

Limiter::Limiter()
{
    m_fCeilingDb.store(-0.1f, std::memory_order_relaxed);
    m_fThresholdDb.store(0.f, std::memory_order_relaxed);
    m_fSoftClipDb.store(2.f, std::memory_order_relaxed);
    m_fSoftClipRatio.store(10.f, std::memory_order_relaxed);
}

const char* Limiter::GetName() const noexcept
{
    return GetNameStatic();
}

size_t Limiter::GetParameterCount() const noexcept
{
    return std::extent_v<decltype(kParameterInfo)>;
}

const DspPluginParameterInfo& Limiter::GetParameterInfo(size_t index) const noexcept
{
    assert(index < GetParameterCount());
    return kParameterInfo[index];
}

Result<float> Limiter::GetSliderParameter(std::string_view id) const noexcept
{
    if (id == ID_CEILING_DB)
        return m_fCeilingDb.load(std::memory_order_relaxed);
    else if (id == ID_THRESHOLD_DB)
        return m_fThresholdDb.load(std::memory_order_relaxed);
    else if (id == ID_SOFT_CLIP_DB)
        return m_fSoftClipDb.load(std::memory_order_relaxed);
    else if (id == ID_SOFT_CLIP_RATIO)
        return m_fSoftClipRatio.load(std::memory_order_relaxed);
    return make_error_code(errc::invalid_argument);
}

Result<void> Limiter::SetSliderParameter(std::string_view id, float value) noexcept
{
    if (id == ID_CEILING_DB)
    {
        assert(-20.f <= value && value <= -0.1f);
        m_fCeilingDb.store(value, std::memory_order_relaxed);
        return {};
    }
    else if (id == ID_THRESHOLD_DB)
    {
        assert(-30.f <= value && value <= 0.f);
        m_fThresholdDb.store(value, std::memory_order_relaxed);
        return {};
    }
    else if (id == ID_SOFT_CLIP_DB)
    {
        assert(0.f <= value && value <= 6.f);
        m_fSoftClipDb.store(value, std::memory_order_relaxed);
        return {};
    }
    else if (id == ID_SOFT_CLIP_RATIO)
    {
        assert(3.f <= value && value <= 20.f);
        m_fSoftClipRatio.store(value, std::memory_order_relaxed);
        return {};
    }
    return make_error_code(errc::invalid_argument);
}

Result<int32_t> Limiter::GetEnumParameter(std::string_view id) const noexcept
{
    return make_error_code(errc::invalid_argument);
}

Result<void> Limiter::SetEnumParameter(std::string_view id, int32_t value) noexcept
{
    return make_error_code(errc::invalid_argument);
}

void Limiter::Process(SampleView<2> samples) noexcept
{
    float thresholdDb = m_fThresholdDb.load(std::memory_order_relaxed);
    float ceilDb = m_fCeilingDb.load(std::memory_order_relaxed);
    float ceiling = Math::DecibelToLinearSafe(ceilDb);
    float makeUp = Math::DecibelToLinearSafe(ceilDb - thresholdDb);
    float sc = -m_fSoftClipDb.load(std::memory_order_relaxed);
    float scv = Math::DecibelToLinearSafe(sc);
    float peakDb = ceilDb + 25;
    float scmult = std::abs((ceilDb - sc) / (peakDb - sc));

    for (size_t i = 0; i < samples.GetSampleCount(); ++i)
    {
        float spl0 = samples[0][i];
        float spl1 = samples[1][i];
        spl0 = spl0 * makeUp;
        spl1 = spl1 * makeUp;
        float sign0 = (spl0 < 0.f ? -1.f : 1.f);
        float sign1 = (spl1 < 0.f ? -1.f : 1.f);
        float abs0 = std::abs(spl0);
        float abs1 = std::abs(spl1);
        float overDb0 = Math::LinearToDecibelSafe(abs0) - ceilDb;
        float overDb1 = Math::LinearToDecibelSafe(abs1) - ceilDb;

        if (abs0 > scv)
            spl0 = sign0 * (scv + Math::DecibelToLinearSafe(overDb0 * scmult));
        if (abs1 > scv)
            spl1 = sign1 * (scv + Math::DecibelToLinearSafe(overDb1 * scmult));

        spl0 = std::min(ceiling, std::abs(spl0)) * (spl0 < 0.f ? -1.f : 1.f);
        spl1 = std::min(ceiling, std::abs(spl1)) * (spl1 < 0.f ? -1.f : 1.f);

        samples[0][i] = spl0;
        samples[1][i] = spl1;
    }
}
