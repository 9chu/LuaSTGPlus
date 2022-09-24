/**
 * @file
 * @date 2022/9/21
 * @author 9chu
 * 此文件为 LuaSTGPlus 项目的一部分，版权与许可声明详见 COPYRIGHT.txt。
 */
#pragma once
#include <vector>
#include <lstg/Core/Subsystem/Audio/IDspPlugin.hpp>
#include <lstg/Core/Subsystem/Audio/ISoundDecoder.hpp>
#include <lstg/Core/Subsystem/Audio/BusChannel.hpp>

namespace lstg::Subsystem::Audio::DspPlugins
{
    /**
     * 混响
     */
    class Reverb :
        public IDspPlugin
    {
        class MonoReverb
        {
        public:
            MonoReverb(float extraSpreadBase = 0.f);

        public:
            void Process(SampleView<1> samples) noexcept;

        public:
            float GetRoomSize() const noexcept { return m_fRoomSize.load(std::memory_order_relaxed); }
            void SetRoomSize(float roomSize) noexcept;

            float GetDamp() const noexcept { return m_fDamp.load(std::memory_order_relaxed); }
            void SetDamp(float damp) noexcept;

            float GetWet() const noexcept { return m_fWet.load(std::memory_order_relaxed); }
            void SetWet(float wet) noexcept { m_fWet.store(wet, std::memory_order_relaxed); }

            float GetDry() const noexcept { return m_fDry.load(std::memory_order_relaxed); }
            void SetDry(float dry) noexcept { m_fDry.store(dry, std::memory_order_relaxed); }

            float GetExtraSpread() const noexcept { return m_fExtraSpread.load(std::memory_order_relaxed); }
            void SetExtraSpread(float v) noexcept { m_fExtraSpread.store(v, std::memory_order_relaxed); }

            float GetPreDelay() const noexcept { return m_fPreDelay.load(std::memory_order_relaxed); }
            void SetPreDelay(float v) noexcept { m_fPreDelay.store(v, std::memory_order_relaxed); }

            float GetPreDelayFeedback() const noexcept { return m_fPreDelayFeedback.load(std::memory_order_relaxed); }
            void SetPreDelayFeedback(float v) noexcept { m_fPreDelayFeedback.store(v, std::memory_order_relaxed); }

            float GetHighPassFilter() const noexcept { return m_fHighPassFilter.load(std::memory_order_relaxed); }
            void SetHighPassFilter(float v) noexcept;

        private:
            void CheckParameters() noexcept;
            void RefreshParameters() noexcept;

        private:
            enum {
                kCombs = 8,
                kAllPasses = 4,
                kMaxEchoMs = 500,
                kEchoBufferSize = static_cast<size_t>((static_cast<float>(kMaxEchoMs) / 1000.0) * ISoundDecoder::kSampleRate + 1.0),
            };

            // 属性
            std::atomic<float> m_fRoomSize;
            std::atomic<float> m_fDamp;
            std::atomic<float> m_fWet;
            std::atomic<float> m_fDry;
            std::atomic<float> m_fExtraSpread;
            std::atomic<float> m_fPreDelay;
            std::atomic<float> m_fPreDelayFeedback;
            std::atomic<float> m_fHighPassFilter;

            // 属性同步
            uint32_t m_uCurrentVersion = 0;
            std::atomic<uint32_t> m_bDirtyVersion;

            struct Comb
            {
                size_t Pos = 0;
                std::vector<float> Buffer;
                float Feedback = 0;
                float Damp = 0;
                float DampHistory = 0;
                int32_t ExtraSpreadFrames = 0;
            };

            struct AllPass
            {
                size_t Pos = 0;
                std::vector<float> Buffer;
                int32_t ExtraSpreadFrames = 0;
            };

            // 状态
            Comb m_stComb[kCombs];
            AllPass m_stAllPass[kAllPasses];
            int32_t m_iEchoBufferPos = 0;
            std::array<float, kEchoBufferSize> m_stEchoBuffer;
            std::array<float, BusChannel::kSampleCount> m_stInputBuffer;
            std::array<float, BusChannel::kSampleCount> m_stOutputBuffer;

            float m_fHpfH1 = 0.f;
            float m_fHpfH2 = 0.f;
        };

    public:
        static const char* GetNameStatic() noexcept;

    public:
        Reverb();

    public:  // IDspPlugin
        const char* GetName() const noexcept override;
        size_t GetParameterCount() const noexcept override;
        const DspPluginParameterInfo& GetParameterInfo(size_t index) const noexcept override;
        Result<float> GetSliderParameter(std::string_view id) const noexcept override;
        Result<void> SetSliderParameter(std::string_view id, float value) noexcept override;
        Result<int32_t> GetEnumParameter(std::string_view id) const noexcept override;
        Result<void> SetEnumParameter(std::string_view id, int32_t value) noexcept override;
        void Process(SampleView<2> samples) noexcept override;

    private:
        MonoReverb m_stReverb[2] { 0.f, 0.000521f };
    };
}
