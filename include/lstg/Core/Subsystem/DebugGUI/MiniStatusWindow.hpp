/**
 * @file
 * @date 2022/3/11
 * @author 9chu
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#pragma once
#include <vector>
#include "Window.hpp"
#include "../ProfileSystem.hpp"

namespace lstg::Subsystem::DebugGUI
{
    /**
     * 迷你状态窗口
     */
    class MiniStatusWindow :
        public Window
    {
    public:
        MiniStatusWindow();

    public:
        /**
         * 增加显示的计数器
         * @param label 标签
         * @param counterType 计数器类型
         * @param counterName 计数器名称
         * @return 是否成功
         */
        Result<void> AddCounter(std::string label, PerformanceCounterTypes counterType, std::string counterName) noexcept;

    protected:  // Window
        void OnPrepareWindow() noexcept override;
        void OnRender() noexcept override;

    private:
        struct DrawCounters
        {
            std::string Label;
            PerformanceCounterTypes CounterType;
            std::string CounterName;
        };

        std::vector<DrawCounters> m_stCounters;
    };
}
