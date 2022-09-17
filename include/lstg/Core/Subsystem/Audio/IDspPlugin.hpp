/**
 * @file
 * @date 2022/9/3
 * @author 9chu
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#pragma once
#include <memory>
#include "../../Result.hpp"
#include "SampleView.hpp"

namespace lstg::Subsystem::Audio
{
    /**
     * DSP 插件
     */
    class IDspPlugin
    {
    public:
        /**
         * 处理方法
         * @param samples 采样
         */
        virtual void Process(SampleView<2> samples) noexcept = 0;
    };

    using DspPluginPtr = std::shared_ptr<IDspPlugin>;
}
