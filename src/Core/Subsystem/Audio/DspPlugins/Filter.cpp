/**
 * @file
 * @date 2022/9/20
 * @author 9chu
 * 此文件为 LuaSTGPlus 项目的一部分，版权与许可声明详见 COPYRIGHT.txt。
 */
#include "Filter.hpp"

#include <glm/ext.hpp>
#include <lstg/Core/Subsystem/Audio/ISoundDecoder.hpp>

using namespace std;
using namespace lstg;
using namespace lstg::Subsystem::Audio::DspPlugins;

using namespace lstg::Subsystem::Audio;

enum {
    FILTER_TYPE_LOW_PASS = 1,  // 低通
    FILTER_TYPE_HIGH_PASS = 2,  // 高通
    FILTER_TYPE_BAND_PASS = 3,  // 带通
    FILTER_TYPE_NOTCH = 4,  // 带阻
    FILTER_TYPE_LOW_SHELF = 5,
    FILTER_TYPE_HIGH_SHELF = 6,
};

enum {
    STAGES_6_DB = 1,
    STAGES_12_DB = 2,
    STAGES_18_DB = 3,
    STAGES_24_DB = 4,
};

#define ID_TYPE "Type"
#define ID_CUTOFF "CutOff"
#define ID_RESONANCE "Resonance"
#define ID_GAIN "Gain"
#define ID_STAGES "Stages"

static const DspPluginParameterInfo kParameterInfo[] = {
    {
        ID_TYPE,
        "Type",
        DspPluginEnumParameter {
            {
                { "Low pass", FILTER_TYPE_LOW_PASS },
                { "High pass", FILTER_TYPE_HIGH_PASS },
                { "Band pass", FILTER_TYPE_BAND_PASS },
                { "Notch", FILTER_TYPE_NOTCH },
                { "Low Shelf", FILTER_TYPE_LOW_SHELF },
                { "High Shelf", FILTER_TYPE_HIGH_SHELF },
            }
        },
    },
    {
        ID_CUTOFF,
        "Cut Off (Hz)",
        DspPluginSliderParameter { 1.f, 20500.f, },
    },
    {
        ID_RESONANCE,
        "Resonance (%)",
        DspPluginSliderParameter { 0.f, 1.f },
    },
    {
        ID_GAIN,
        "Gain",
        DspPluginSliderParameter { 0.f, 4.f },
    },
    {
        ID_STAGES,
        "Stages",
        DspPluginEnumParameter {
            {
                { "06 dB", STAGES_6_DB },
                { "12 dB", STAGES_12_DB },
                { "18 dB", STAGES_18_DB },
                { "24 dB", STAGES_24_DB },
            }
        },
    },
};

const char* Filter::GetNameStatic() noexcept
{
    return "Filter";
}

Filter::Filter()
{
    m_iType.store(FILTER_TYPE_LOW_PASS, std::memory_order_relaxed);
    m_fCutOff.store(1000.f, std::memory_order_relaxed);
    m_fResonance.store(1.f, std::memory_order_relaxed);
    m_fGain.store(0.f, std::memory_order_relaxed);
    m_iStages.store(1, std::memory_order_relaxed);
    m_bDirtyVersion.store(1, std::memory_order_release);
}

const char* Filter::GetName() const noexcept
{
    return GetNameStatic();
}

size_t Filter::GetParameterCount() const noexcept
{
    return std::extent_v<decltype(kParameterInfo)>;
}

const DspPluginParameterInfo& Filter::GetParameterInfo(size_t index) const noexcept
{
    assert(index < GetParameterCount());
    return kParameterInfo[index];
}

Result<float> Filter::GetSliderParameter(std::string_view id) const noexcept
{
    if (id == ID_CUTOFF)
        return m_fCutOff.load(std::memory_order_relaxed);
    else if (id == ID_RESONANCE)
        return m_fResonance.load(std::memory_order_relaxed);
    else if (id == ID_GAIN)
        return m_fGain.load(std::memory_order_relaxed);
    return make_error_code(errc::invalid_argument);
}

Result<void> Filter::SetSliderParameter(std::string_view id, float value) noexcept
{
    if (id == ID_CUTOFF)
    {
        assert(1.f <= value && value <= 20500.f);
        m_fCutOff.store(value, std::memory_order_relaxed);
        m_bDirtyVersion.fetch_add(1, std::memory_order_acq_rel);
        return {};
    }
    else if (id == ID_RESONANCE)
    {
        assert(0.f <= value && value <= 1.f);
        m_fResonance.store(value, std::memory_order_relaxed);
        m_bDirtyVersion.fetch_add(1, std::memory_order_acq_rel);
        return {};
    }
    else if (id == ID_GAIN)
    {
        assert(0.f <= value && value <= 4.f);
        m_fGain.store(value, std::memory_order_relaxed);
        m_bDirtyVersion.fetch_add(1, std::memory_order_acq_rel);
        return {};
    }
    return make_error_code(errc::invalid_argument);
}

Result<int32_t> Filter::GetEnumParameter(std::string_view id) const noexcept
{
    if (id == ID_TYPE)
        return m_iType.load(std::memory_order_relaxed);
    else if (id == ID_STAGES)
        return m_iStages.load(std::memory_order_relaxed);
    return make_error_code(errc::invalid_argument);
}

Result<void> Filter::SetEnumParameter(std::string_view id, int32_t value) noexcept
{
    if (id == ID_TYPE)
    {
        assert(FILTER_TYPE_LOW_PASS <= value && value <= FILTER_TYPE_HIGH_SHELF);
        m_iType.store(value, std::memory_order_relaxed);
        m_bDirtyVersion.fetch_add(1, std::memory_order_acq_rel);
        return {};
    }
    if (id == ID_STAGES)
    {
        assert(STAGES_6_DB <= value && value <= STAGES_24_DB);
        m_iStages.store(value, std::memory_order_relaxed);
        m_bDirtyVersion.fetch_add(1, std::memory_order_acq_rel);
        return {};
    }
    return make_error_code(errc::invalid_argument);
}

void Filter::Process(SampleView<2> samples) noexcept
{
    CheckParameters();

    // 计算
    for (size_t ch = 0; ch < ISoundDecoder::kChannels; ++ch)
    {
        assert(ch < 2);
        auto* data = samples[ch];
        auto& history = m_stHistories[ch];
        for (size_t i = 0; i < samples.GetSampleCount(); ++i)
        {
            float oldSample = data[i];
            data[i] = (oldSample * m_fCoeffB0 + history.HB1 * m_fCoeffB1 + history.HB2 * m_fCoeffB2 + history.HA1 * m_fCoeffA1 +
                history.HA2 * m_fCoeffA2);
            assert(!isinf(data[i]) && !isnan(data[i]));
            history.HA2 = history.HA1;
            history.HB2 = history.HB1;
            history.HB1 = oldSample;
            history.HA1 = data[i];
        }
    }
}

void Filter::CheckParameters() noexcept
{
    auto version = m_bDirtyVersion.load(std::memory_order_acquire);
    if (m_uCurrentVersion < version)
    {
        m_uCurrentVersion = version;
        RefreshCoefficients();
    }
}

void Filter::RefreshCoefficients() noexcept
{
    const auto kSampleRateLimit = static_cast<double>((ISoundDecoder::kSampleRate / 2.) + 512);

    auto cutOff = static_cast<double>(m_fCutOff.load(std::memory_order_relaxed));
    auto finalCutOff = std::max(1., (cutOff > kSampleRateLimit) ? kSampleRateLimit : cutOff);

    auto omega = glm::two_pi<double>() * finalCutOff / ISoundDecoder::kSampleRate;

    auto sinValue = glm::sin(omega);
    auto cosValue = glm::cos(omega);

    double Q = m_fResonance.load(std::memory_order_relaxed);
    if (Q <= 0.0)
        Q = 0.0001;
    auto t = m_iType.load(std::memory_order_relaxed);
    if (t == FILTER_TYPE_BAND_PASS)
        Q *= 2.0;

    double gain = m_fGain.load(std::memory_order_relaxed);
    if (gain < 0.001)
        gain = 0.001;

    auto stages = m_iStages.load(std::memory_order_relaxed);
    if (stages > 1)
    {
        Q = Q > 1.0 ? glm::pow(Q, 1.0 / stages) : Q;
        gain = glm::pow(gain, 1.0 / (stages + 1));
    }

    auto alpha = sinValue / (2 * Q);
    auto a0 = 1.0 + alpha;

    switch (t)
    {
        default:
            assert(false);
        case FILTER_TYPE_LOW_PASS:
            m_fCoeffB0 = static_cast<float>((1.0 - cosValue) / 2.0);
            m_fCoeffB1 = static_cast<float>(1.0 - cosValue);
            m_fCoeffB2 = static_cast<float>((1.0 - cosValue) / 2.0);
            m_fCoeffA1 = static_cast<float>(-2.0 * cosValue);
            m_fCoeffA2 = static_cast<float>(1.0 - alpha);
            break;
        case FILTER_TYPE_HIGH_PASS:
            m_fCoeffB0 = static_cast<float>((1.0 + cosValue) / 2.0);
            m_fCoeffB1 = static_cast<float>(-(1.0 + cosValue));
            m_fCoeffB2 = static_cast<float>((1.0 + cosValue) / 2.0);
            m_fCoeffA1 = static_cast<float>(-2.0 * cosValue);
            m_fCoeffA2 = static_cast<float>(1.0 - alpha);
            break;
        case FILTER_TYPE_BAND_PASS:
            m_fCoeffB0 = static_cast<float>(alpha * sqrt(Q + 1));
            m_fCoeffB1 = static_cast<float>(0.0);
            m_fCoeffB2 = static_cast<float>(-alpha * sqrt(Q + 1));
            m_fCoeffA1 = static_cast<float>(-2.0 * cosValue);
            m_fCoeffA2 = static_cast<float>(1.0 - alpha);
            break;
        case FILTER_TYPE_NOTCH:
            m_fCoeffB0 = static_cast<float>(1.0);
            m_fCoeffB1 = static_cast<float>(-2.0 * cosValue);
            m_fCoeffB2 = static_cast<float>(1.0);
            m_fCoeffA1 = static_cast<float>(-2.0 * cosValue);
            m_fCoeffA2 = static_cast<float>(1.0 - alpha);
            break;
        case FILTER_TYPE_LOW_SHELF:
            {
                auto q = glm::sqrt(Q);
                if (q <= 0)
                    q = 0.001;
                double beta = glm::sqrt(gain) / q;

                a0 = (gain + 1.0) + (gain - 1.0) * cosValue + beta * sinValue;
                m_fCoeffB0 = static_cast<float>(gain * ((gain + 1.0) - (gain - 1.0) * cosValue + beta * sinValue));
                m_fCoeffB1 = static_cast<float>(2.0 * gain * ((gain - 1.0) - (gain + 1.0) * cosValue));
                m_fCoeffB2 = static_cast<float>(gain * ((gain + 1.0) - (gain - 1.0) * cosValue - beta * sinValue));
                m_fCoeffA1 = static_cast<float>(-2.0 * ((gain - 1.0) + (gain + 1.0) * cosValue));
                m_fCoeffA2 = static_cast<float>(((gain + 1.0) + (gain - 1.0) * cosValue - beta * sinValue));
            }
            break;
        case FILTER_TYPE_HIGH_SHELF:
            {
                auto q = glm::sqrt(Q);
                if (q <= 0)
                    q = 0.001;
                double beta = glm::sqrt(gain) / q;

                a0 = (gain + 1.0) + (gain - 1.0) * cosValue + beta * sinValue;
                m_fCoeffB0 = static_cast<float>(gain * ((gain + 1.0) + (gain - 1.0) * cosValue + beta * sinValue));
                m_fCoeffB1 = static_cast<float>(-2.0 * gain * ((gain - 1.0) + (gain + 1.0) * cosValue));
                m_fCoeffB2 = static_cast<float>(gain * ((gain + 1.0) + (gain - 1.0) * cosValue - beta * sinValue));
                m_fCoeffA1 = static_cast<float>(2.0 * ((gain - 1.0) - (gain + 1.0) * cosValue));
                m_fCoeffA2 = static_cast<float>(((gain + 1.0) - (gain - 1.0) * cosValue - beta * sinValue));
            }
            break;
    }

    m_fCoeffB0 /= static_cast<float>(a0);
    m_fCoeffB1 /= static_cast<float>(a0);
    m_fCoeffB2 /= static_cast<float>(a0);
    m_fCoeffA1 /= static_cast<float>(0.0 - a0);
    m_fCoeffA2 /= static_cast<float>(0.0 - a0);
}
