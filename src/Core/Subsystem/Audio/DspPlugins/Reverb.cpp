/**
 * @file
 * @date 2022/9/22
 * @author 9chu
 * 此文件为 LuaSTGPlus 项目的一部分，版权与许可声明详见 COPYRIGHT.txt。
 */
#include "Reverb.hpp"

#include <glm/ext.hpp>

using namespace std;
using namespace lstg;
using namespace lstg::Subsystem::Audio::DspPlugins;

using namespace lstg::Subsystem::Audio;

namespace
{
    // 将非常小的 float 转换到 0
    inline float undenormalize(float n) noexcept
    {
        n += 9.8607615E-32f;
        return n - 9.8607615E-32f;
    }
}

// <editor-fold desc="Reverb::MonoReverb">

Reverb::MonoReverb::MonoReverb(float extraSpreadBase)
{
    static const float kCombTunings[kCombs] = {
        0.025306122448979593f,
        0.026938775510204082f,
        0.028956916099773241f,
        0.03074829931972789f,
        0.032244897959183672f,
        0.03380952380952381f,
        0.035306122448979592f,
        0.036666666666666667f,
    };

    static const float kAllPassTunings[kAllPasses] = {
        0.0051020408163265302f,
        0.007732426303854875f,
        0.01f,
        0.012607709750566893f
    };

    // 初始化参数
    m_fRoomSize.store(0.8f, std::memory_order_relaxed);
    m_fDamp.store(0.5f, std::memory_order_relaxed);
    m_fWet.store(0.f, std::memory_order_relaxed);
    m_fDry.store(1.0f, std::memory_order_relaxed);
    m_fExtraSpread.store(1.f, std::memory_order_relaxed);
    m_fPreDelay.store(150, std::memory_order_relaxed);
    m_fPreDelayFeedback.store(0.4, std::memory_order_relaxed);
    m_fHighPassFilter.store(0.f, std::memory_order_relaxed);

    // 初始化缓冲区
    for (int i = 0; i < kCombs; ++i)
    {
        auto& c = m_stComb[i];

        c.ExtraSpreadFrames = static_cast<int32_t>(::lrint(extraSpreadBase * ISoundDecoder::kSampleRate));

        auto len = static_cast<size_t>(std::max<int32_t>(5,
            static_cast<int32_t>(::lrint(kCombTunings[i] * ISoundDecoder::kSampleRate) + c.ExtraSpreadFrames)));
        c.Buffer.resize(len);
        ::memset(c.Buffer.data(), 0, sizeof(float) * c.Buffer.size());

        c.Pos = 0;
    }

    for (int i = 0; i < kAllPasses; ++i)
    {
        auto& a = m_stAllPass[i];

        a.ExtraSpreadFrames = static_cast<int32_t>(::lrint(extraSpreadBase * ISoundDecoder::kSampleRate));

        auto len = static_cast<size_t>(std::max<int32_t>(5,
            static_cast<int32_t>(::lrint(kAllPassTunings[i] * ISoundDecoder::kSampleRate) + a.ExtraSpreadFrames)));
        a.Buffer.resize(len);
        ::memset(a.Buffer.data(), 0, sizeof(float) * a.Buffer.size());

        a.Pos = 0;
    }

    m_stEchoBuffer.fill(0);
    m_iEchoBufferPos = 0;

    // 初始化参数
    RefreshParameters();
    m_bDirtyVersion.store(0, std::memory_order_release);
}

void Reverb::MonoReverb::SetRoomSize(float roomSize) noexcept
{
    m_fRoomSize.store(roomSize, std::memory_order_relaxed);
    m_bDirtyVersion.fetch_add(1, std::memory_order_acq_rel);
}

void Reverb::MonoReverb::SetDamp(float damp) noexcept
{
    m_fDamp.store(damp, std::memory_order_relaxed);
    m_bDirtyVersion.fetch_add(1, std::memory_order_acq_rel);
}

void Reverb::MonoReverb::SetHighPassFilter(float v) noexcept
{
    m_fHighPassFilter.store(std::max(0.f, std::min(1.f, v)), std::memory_order_relaxed);
}

void Reverb::MonoReverb::Process(SampleView<1> samples) noexcept
{
    assert(samples.GetSampleCount() <= BusChannel::kSampleCount);

    CheckParameters();

    auto wet = GetWet();
    auto dry = GetDry();
    auto extraSpread = GetExtraSpread();
    auto preDelay = GetPreDelay();
    auto preDelayFeedback = GetPreDelayFeedback();
    auto highPassFilter = GetHighPassFilter();

    auto preDelayFrames = std::min(std::max(10, static_cast<int32_t>(::lrint((preDelay / 1000.f) * ISoundDecoder::kSampleRate))),
        static_cast<int32_t>(m_stEchoBuffer.size()) - 1);

    for (size_t i = 0; i < samples.GetSampleCount(); ++i)
    {
        if (m_iEchoBufferPos >= static_cast<int32_t>(m_stEchoBuffer.size()))
            m_iEchoBufferPos = 0;

        auto readPos = m_iEchoBufferPos - preDelayFrames;
        while (readPos < 0)
            readPos += static_cast<int32_t>(m_stEchoBuffer.size());

        float in = undenormalize(m_stEchoBuffer[readPos] * preDelayFeedback + samples[0][i]);
        m_stEchoBuffer[m_iEchoBufferPos++] = in;

        m_stInputBuffer[i] = in;
        m_stOutputBuffer[i] = 0;
    }

    if (highPassFilter > 0)
    {
        auto hpAux = static_cast<float>(::exp(-glm::two_pi<double>() * highPassFilter * 6000 / ISoundDecoder::kSampleRate));
        auto hpA1 = static_cast<float>((1.0 + hpAux) / 2.0);
        auto hpA2 = static_cast<float>(-(1.0 + hpAux) / 2.0);
        auto hpB1 = static_cast<float>(hpAux);

        for (size_t i = 0; i < samples.GetSampleCount(); ++i)
        {
            float in = m_stInputBuffer[i];
            m_stInputBuffer[i] = in * hpA1 + m_fHpfH1 * hpA2 + m_fHpfH2 * hpB1;
            m_fHpfH2 = m_stInputBuffer[i];
            m_fHpfH1 = in;
        }
    }

    for (int i = 0; i < kCombs; ++i)
    {
        auto& c = m_stComb[i];

        auto sizeLimit = static_cast<int32_t>(c.Buffer.size()) -
            static_cast<int32_t>(::lrintf(static_cast<float>(c.ExtraSpreadFrames) * (1.0f - extraSpread)));
        for (size_t j = 0; j < samples.GetSampleCount(); ++j)
        {
            if (static_cast<int32_t>(c.Pos) >= sizeLimit)
                c.Pos = 0;

            float out = undenormalize(c.Buffer[c.Pos] * c.Feedback);
            out = out * (1.0f - c.Damp) + c.DampHistory * c.Damp;
            c.DampHistory = out;
            c.Buffer[c.Pos] = m_stInputBuffer[j] + out;
            m_stOutputBuffer[j] += out;
            ++c.Pos;
        }
    }

    static const float kAllPassFeedback = 0.7f;
    for (int i = 0; i < kAllPasses; ++i)
    {
        auto& a = m_stAllPass[i];
        auto sizeLimit = static_cast<int32_t>(a.Buffer.size()) -
            static_cast<int32_t>(::lrintf(static_cast<float>(a.ExtraSpreadFrames) * (1.0f - extraSpread)));

        for (size_t j = 0; j < samples.GetSampleCount(); ++j)
        {
            if (static_cast<int32_t>(a.Pos) >= sizeLimit)
                a.Pos = 0;

            float aux = a.Buffer[a.Pos];
            a.Buffer[a.Pos] = undenormalize(kAllPassFeedback * aux + m_stOutputBuffer[j]);
            m_stOutputBuffer[j] = aux - kAllPassFeedback * a.Buffer[a.Pos];
            ++a.Pos;
        }
    }

    static const float kWetScale = 0.6f;
    for (size_t i = 0; i < samples.GetSampleCount(); ++i)
        samples[0][i] = m_stOutputBuffer[i] * wet * kWetScale + samples[0][i] * dry;
}

void Reverb::MonoReverb::CheckParameters() noexcept
{
    auto version = m_bDirtyVersion.load(std::memory_order_acquire);
    if (m_uCurrentVersion < version)
    {
        m_uCurrentVersion = version;
        RefreshParameters();
    }
}

void Reverb::MonoReverb::RefreshParameters() noexcept
{
    static const float kRoomScale = 0.28f;
    static const float kRoomOffset = 0.7f;

    auto roomSize = GetRoomSize();
    auto damp = GetDamp();

    for (int i = 0; i < kCombs; ++i)
    {
        auto& c = m_stComb[i];
        c.Feedback = kRoomOffset + roomSize * kRoomScale;
        if (c.Feedback < kRoomOffset)
            c.Feedback = kRoomOffset;
        else if (c.Feedback > (kRoomOffset + kRoomScale))
            c.Feedback = (kRoomOffset + kRoomScale);

        auto auxDmp = static_cast<double>(damp) / 2.0 + 0.5;
        auxDmp *= auxDmp;

        c.Damp = static_cast<float>(std::exp(-glm::two_pi<double>() * auxDmp * 10000 / ISoundDecoder::kSampleRate));
    }
}

// </editor-fold>

#define ID_PRE_DELAY "PreDelay"
#define ID_PRE_DELAY_FEEDBACK "PreDelayFeedback"
#define ID_ROOM_SIZE "RoomSize"
#define ID_DAMPING "Damping"
#define ID_SPREAD "Spread"
#define ID_DRY "Dry"
#define ID_WET "Wet"
#define ID_HPF "Hpf"

static const DspPluginParameterInfo kParameterInfo[] = {
    {
        ID_PRE_DELAY,
        "Pre Delay (ms)",
        DspPluginSliderParameter { 20.f, 500.f },
    },
    {
        ID_PRE_DELAY_FEEDBACK,
        "Pre Delay Feedback",
        DspPluginSliderParameter { 0.f, 0.98f, },
    },
    {
        ID_ROOM_SIZE,
        "Room Size",
        DspPluginSliderParameter { 0.f, 1.f },
    },
    {
        ID_DAMPING,
        "Damping",
        DspPluginSliderParameter { 0.f, 1.f },
    },
    {
        ID_SPREAD,
        "Spread",
        DspPluginSliderParameter { 0.f, 1.f },
    },
    {
        ID_HPF,
        "High Pass",
        DspPluginSliderParameter { 0.f, 1.f },
    },
    {
        ID_DRY,
        "Dry",
        DspPluginSliderParameter { 0.f, 1.f },
    },
    {
        ID_WET,
        "Wet",
        DspPluginSliderParameter { 0.f, 1.f },
    },
};

Reverb::Reverb()
{
}

const char* Reverb::GetNameStatic() noexcept
{
    return "Reverb";
}

const char* Reverb::GetName() const noexcept
{
    return GetNameStatic();
}

size_t Reverb::GetParameterCount() const noexcept
{
    return std::extent_v<decltype(kParameterInfo)>;
}

const DspPluginParameterInfo& Reverb::GetParameterInfo(size_t index) const noexcept
{
    assert(index < GetParameterCount());
    return kParameterInfo[index];
}

Result<float> Reverb::GetSliderParameter(std::string_view id) const noexcept
{
    auto& reverb = m_stReverb[0];
    if (id == ID_PRE_DELAY)
        return reverb.GetPreDelay();
    else if (id == ID_PRE_DELAY_FEEDBACK)
        return reverb.GetPreDelayFeedback();
    else if (id == ID_ROOM_SIZE)
        return reverb.GetRoomSize();
    else if (id == ID_DAMPING)
        return reverb.GetDamp();
    else if (id == ID_SPREAD)
        return reverb.GetExtraSpread();
    else if (id == ID_DRY)
        return reverb.GetDry();
    else if (id == ID_WET)
        return reverb.GetWet();
    else if (id == ID_HPF)
        return reverb.GetHighPassFilter();
    return make_error_code(errc::not_supported);
}

Result<void> Reverb::SetSliderParameter(std::string_view id, float value) noexcept
{
    for (auto& reverb : m_stReverb)
    {
        if (id == ID_PRE_DELAY)
        {
            assert(20.f <= value && value <= 500.f);
            reverb.SetPreDelay(value);
        }
        else if (id == ID_PRE_DELAY_FEEDBACK)
        {
            assert(0.f <= value && value <= 0.98f);
            reverb.SetPreDelayFeedback(value);
        }
        else if (id == ID_ROOM_SIZE)
        {
            assert(0.f <= value && value <= 1.f);
            reverb.SetRoomSize(value);
        }
        else if (id == ID_DAMPING)
        {
            assert(0.f <= value && value <= 1.f);
            reverb.SetDamp(value);
        }
        else if (id == ID_SPREAD)
        {
            assert(0.f <= value && value <= 1.f);
            reverb.SetExtraSpread(value);
        }
        else if (id == ID_DRY)
        {
            assert(0.f <= value && value <= 1.f);
            reverb.SetDry(value);
        }
        else if (id == ID_WET)
        {
            assert(0.f <= value && value <= 1.f);
            reverb.SetWet(value);
        }
        else if (id == ID_HPF)
        {
            assert(0.f <= value && value <= 1.f);
            reverb.SetHighPassFilter(value);
        }
        else
        {
            return make_error_code(errc::not_supported);
        }
    }
    return {};
}

Result<int32_t> Reverb::GetEnumParameter(std::string_view id) const noexcept
{
    return make_error_code(errc::invalid_argument);
}

Result<void> Reverb::SetEnumParameter(std::string_view id, int32_t value) noexcept
{
    return make_error_code(errc::invalid_argument);
}

void Reverb::Process(SampleView<2> samples) noexcept
{
    assert(samples.GetSampleCount() == BusChannel::kSampleCount);

    m_stReverb[0].Process(samples.GetChannel(0));  // L
    m_stReverb[1].Process(samples.GetChannel(1));  // R
}
