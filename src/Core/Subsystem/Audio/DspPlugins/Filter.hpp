/**
 * @file
 * @date 2022/9/20
 * @author 9chu
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#pragma once
#include <lstg/Core/Subsystem/Audio/IDspPlugin.hpp>

namespace lstg::Subsystem::Audio::DspPlugins
{
    /**
     * 滤波器
     */
    class Filter :
        public IDspPlugin
    {
    public:
        static const char* GetNameStatic() noexcept;

    public:
        Filter();

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
        void CheckParameters() noexcept;
        void RefreshCoefficients() noexcept;

    private:
        // 属性
        std::atomic<int32_t> m_iType;  // 滤波器类型
        std::atomic<float> m_fCutOff;  // 截止频率
        std::atomic<float> m_fResonance;  // 共振，指定加强或者抑制截波频率上/下的信号
        std::atomic<float> m_fGain;  // 增益（针对 shelf 滤波器）
        std::atomic<int32_t> m_iStages;  // 滤波器的阶数，越大则频率响应曲线越陡峭

        // 属性同步
        uint32_t m_uCurrentVersion = 0;
        std::atomic<uint32_t> m_bDirtyVersion;

        // 滤波器系数
        float m_fCoeffB0 = 0.f;
        float m_fCoeffB1 = 0.f;
        float m_fCoeffB2 = 0.f;
        float m_fCoeffA1 = 0.f;
        float m_fCoeffA2 = 0.f;

        struct History
        {
            float HA1 = 0.f;
            float HA2 = 0.f;
            float HB1 = 0.f;
            float HB2 = 0.f;
        };

        // 历史状态
        std::array<History, 2> m_stHistories;
    };
}
