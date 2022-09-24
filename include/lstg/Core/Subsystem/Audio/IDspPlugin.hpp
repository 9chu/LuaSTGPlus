/**
 * @file
 * @date 2022/9/3
 * @author 9chu
 * 此文件为 LuaSTGPlus 项目的一部分，版权与许可声明详见 COPYRIGHT.txt。
 */
#pragma once
#include <atomic>
#include <map>
#include <memory>
#include "../../Result.hpp"
#include "SampleView.hpp"

namespace lstg::Subsystem::Audio
{
    struct DspPluginSliderParameter
    {
        float MinValue = 0.f;
        float MaxValue = 1.f;
    };

    struct DspPluginEnumParameter
    {
        std::map<std::string, int32_t> Options;
    };

    struct DspPluginParameterInfo
    {
        std::string Id;
        std::string DisplayName;
        std::variant<DspPluginSliderParameter, DspPluginEnumParameter> Desc;
    };

    /**
     * DSP 插件
     */
    class IDspPlugin
    {
    public:
        IDspPlugin() = default;
        virtual ~IDspPlugin() = default;

    public:
        /**
         * 获取名称
         * @note 线程安全
         */
        virtual const char* GetName() const noexcept = 0;

        /**
         * 返回插件参数个数
         * @note 线程安全
         */
        virtual size_t GetParameterCount() const noexcept = 0;

        /**
         * 返回参数信息
         * @note 线程安全
         * @param index 索引
         * @return 参数信息
         */
        virtual const DspPluginParameterInfo& GetParameterInfo(size_t index) const noexcept = 0;

        /**
         * 返回滑块值
         * @note 线程安全
         * @param id 参数ID
         * @return 值
         */
        virtual Result<float> GetSliderParameter(std::string_view id) const noexcept = 0;

        /**
         * 设置滑块值
         * @note 线程安全
         * @param id 参数ID
         * @param value 值
         */
        virtual Result<void> SetSliderParameter(std::string_view id, float value) noexcept = 0;

        /**
         * 获取枚举值
         * @note 线程安全
         * @param id 参数ID
         * @return 值
         */
        virtual Result<int32_t> GetEnumParameter(std::string_view id) const noexcept = 0;

        /**
         * 设置枚举值
         * @note 线程安全
         * @param id 参数ID
         * @param value 值
         */
        virtual Result<void> SetEnumParameter(std::string_view id, int32_t value) noexcept = 0;

        /**
         * 处理方法
         * @note 在 AudioThread 调用
         * @param samples 采样
         */
        virtual void Process(SampleView<2> samples) noexcept = 0;
    };

    using DspPluginPtr = std::shared_ptr<IDspPlugin>;
}
