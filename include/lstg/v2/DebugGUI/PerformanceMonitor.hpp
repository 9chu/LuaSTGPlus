/**
 * @file
 * @date 2022/8/15
 * @author 9chu
 * 此文件为 LuaSTGPlus 项目的一部分，版权与许可声明详见 COPYRIGHT.txt。
 */
#pragma once
#include <map>
#include <vector>
#include <string_view>
#include <lstg/Core/Result.hpp>
#include <lstg/Core/Subsystem/DebugGUI/Window.hpp>

namespace lstg::v2::DebugGUI
{
    /**
     * 性能监控窗口
     */
    class PerformanceMonitor :
        public Subsystem::DebugGUI::Window
    {
    public:
        PerformanceMonitor();

    public:
        /**
         * 增加监控指标
         * @param group 组
         * @param chart 图表
         * @param serial 系列
         * @param metrics 指标名
         */
        Result<void> AddInstrument(std::string_view group, std::string_view chart, std::string_view serial,
            std::string_view metrics) noexcept;

    protected:  // 需要实现
        void OnPrepareWindow() noexcept override;
        void OnUpdate(double elapsedTime) noexcept override;
        void OnRender() noexcept override;

    private:
        struct ChartSerial
        {
            std::string Name;
            std::string Metrics;
            std::vector<double> Data;
        };

        struct Chart
        {
            std::vector<ChartSerial> Series;
        };

        using ChartContainer = std::map<std::string, Chart, std::less<>>;
        using GroupContainer = std::map<std::string, ChartContainer, std::less<>>;

        GroupContainer m_stGroups;
        std::vector<std::string> m_stGroupSelects;
        std::string m_stCurrentSelectedGroup;
    };
}
