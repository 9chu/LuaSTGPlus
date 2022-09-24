/**
 * @file
 * @date 2022/9/20
 * @author 9chu
 * 此文件为 LuaSTGPlus 项目的一部分，版权与许可声明详见 COPYRIGHT.txt。
 */
#pragma once
#include <lstg/Core/Subsystem/Audio/IDspPlugin.hpp>

namespace lstg::Subsystem::Audio::DspPlugins
{
    /**
     * 压限器
     */
    class Limiter :
        public IDspPlugin
    {
    public:
        static const char* GetNameStatic() noexcept;

    public:
        Limiter();

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
        std::atomic<float> m_fCeilingDb;
        std::atomic<float> m_fThresholdDb;
        std::atomic<float> m_fSoftClipDb;
        std::atomic<float> m_fSoftClipRatio;
    };
}
