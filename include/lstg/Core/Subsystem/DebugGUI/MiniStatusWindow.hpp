/**
 * @file
 * @date 2022/3/11
 * @author 9chu
 * 此文件为 LuaSTGPlus 项目的一部分，版权与许可声明详见 COPYRIGHT.txt。
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
