/**
 * @file
 * @date 2022/8/15
 * @author 9chu
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#include <lstg/v2/DebugGUI/PerformanceMonitor.hpp>

#include <imgui.h>
#include <implot.h>
#include <lstg/Core/Subsystem/ProfileSystem.hpp>

using namespace std;
using namespace lstg;
using namespace lstg::v2::DebugGUI;

using namespace lstg::Subsystem::DebugGUI;

static const DebugWindowFlags kWindowStyle = DebugWindowFlags::NoSavedSettings | DebugWindowFlags::AlwaysAutoResize;
static const size_t kSampleCounts = 600;  // ~10s

PerformanceMonitor::PerformanceMonitor()
    : Window("PerformanceMonitor", "Performance Monitor", kWindowStyle)
{
    // GameWorld.cpp
    AddInstrument("GameWorld - Time", "Object Method Time", "New", "GameWorld_New");
    AddInstrument("GameWorld - Time", "Object Method Time", "Del", "GameWorld_Del");
    AddInstrument("GameWorld - Time", "Object Method Time", "Kill", "GameWorld_Kill");
    AddInstrument("GameWorld - Time", "Run Loop Method Time", "ObjFrame", "GameWorld_ObjFrame");
    AddInstrument("GameWorld - Time", "Run Loop Method Time", "ObjRender", "GameWorld_ObjRender");
    AddInstrument("GameWorld - Time", "Run Loop Method Time", "UpdateXY", "GameWorld_UpdateXY");
    AddInstrument("GameWorld - Time", "Run Loop Method Time", "AfterFrame", "GameWorld_AfterFrame");
    AddInstrument("GameWorld - Time", "Run Loop Method Time", "CollisionCheck", "GameWorld_CollisionCheck");
    AddInstrument("GameWorld - Memory", "Entity Count", "Allocated", "GameWorld_ECSAllocatedCount");
    AddInstrument("GameWorld - Memory", "Entity Count", "Used", "GameWorld_ECSUsedCount");
    AddInstrument("GameWorld - Memory", "Memory Usage (KB)", "ECSAllocated", "GameWorld_ECSAllocated");
    AddInstrument("GameWorld - Memory", "Memory Usage (KB)", "ECSUsed", "GameWorld_ECSUsed");

    // GameApp.cpp
    AddInstrument("Draw", "Execution Time", "Time", "Draw_ExecutionTime");
    AddInstrument("Draw", "Primitives", "Vertex", "Draw_VertexCount");
    AddInstrument("Draw", "Primitives", "Primitive", "Draw_PrimitiveCount");
    AddInstrument("Draw", "Draw Calls", "Count", "Draw_DrawCallCount");

    // ScriptSystem.cpp
    AddInstrument("ScriptSystem", "VM Memory Usage (KB)", "Usage", "ScriptSystem_VMHeapSize");
    AddInstrument("ScriptSystem", "GC Time", "Aggressive GC", "ScriptSystem_AggressiveGC");

    // AssetSystem.cpp
    AddInstrument("AssetSystem", "Loading Task", "Update", "AssetTask_Update");
    AddInstrument("AssetSystem", "Loading Task", "PreLoad", "AssetTask_PreLoad");
    AddInstrument("AssetSystem", "Loading Task", "PostLoad", "AssetTask_PostLoad");
    AddInstrument("AssetSystem", "Loading Task", "WatchTasks", "AssetTask_WatchTasks");
    AddInstrument("AssetSystem", "Loading Task", "PrepareToReload", "AssetTask_PrepareToReload");
    AddInstrument("AssetSystem", "Loading Task", "ThreadUpdate", "AssetTask_ThreadUpadte");

    if (!m_stGroupSelects.empty())
        m_stCurrentSelectedGroup = m_stGroupSelects[0];
}

Result<void> PerformanceMonitor::AddInstrument(std::string_view group, std::string_view chart, std::string_view serial,
    std::string_view metrics) noexcept
{
    try
    {
        // 查找组
        auto groupIt = m_stGroups.find(group);
        if (groupIt == m_stGroups.end())
        {
            m_stGroupSelects.reserve(m_stGroupSelects.size() + 1);
            m_stCurrentSelectedGroup.reserve(group.size());
            groupIt = m_stGroups.emplace(string{group}, ChartContainer{}).first;
            m_stGroupSelects.emplace_back(string{group});
        }

        // 查找图表
        auto chartIt = groupIt->second.find(chart);
        if (chartIt == groupIt->second.end())
            chartIt = groupIt->second.emplace(string{chart}, Chart {}).first;

        Chart& chartObj = chartIt->second;

        // 添加序列
        ChartSerial chartSerialObj;
        chartSerialObj.Name = serial;
        chartSerialObj.Metrics = metrics;
        chartSerialObj.Data.resize(kSampleCounts);
        chartObj.Series.emplace_back(std::move(chartSerialObj));
    }
    catch (...)  // bad_alloc
    {
        return make_error_code(errc::not_enough_memory);
    }
    return {};
}

void PerformanceMonitor::OnPrepareWindow() noexcept
{
    ImGui::SetNextWindowPos(ImVec2(5.f, 60.f), ImGuiCond_FirstUseEver);
}

void PerformanceMonitor::OnUpdate(double elapsedTime) noexcept
{
    const auto& profiler = Subsystem::ProfileSystem::GetInstance();

    // 收集所有数据
    for (auto& group : m_stGroups)
    {
        for (auto& chart : group.second)
        {
            for (auto& serial : chart.second.Series)
            {
                double t = profiler.GetPerformanceCounter(Subsystem::PerformanceCounterTypes::PerFrame, serial.Metrics);
                serial.Data.erase(serial.Data.begin());
                serial.Data.push_back(t);
            }
        }
    }
}

void PerformanceMonitor::OnRender() noexcept
{
    // 绘制下拉框
    if (ImGui::BeginCombo("##GroupCombo", m_stCurrentSelectedGroup.c_str()))
    {
        for (size_t i = 0; i < m_stGroupSelects.size(); i++)
        {
            bool selected = (m_stCurrentSelectedGroup == m_stGroupSelects[i]);
            if (ImGui::Selectable(m_stGroupSelects[i].c_str(), selected))
                m_stCurrentSelectedGroup = m_stGroupSelects[i];
            if (selected)
                ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
    }

    // 绘制组
    auto it = m_stGroups.find(m_stCurrentSelectedGroup);
    if (it == m_stGroups.end())
        return;

    // 绘制所有图表
    for (const auto& chart : it->second)
    {
        const auto& chartObj = chart.second;

        if (ImPlot::BeginPlot(chart.first.c_str(), ImVec2(350.f, 150.f)))
        {
            size_t samples = 0;
            if (!chartObj.Series.empty())
                samples = chartObj.Series[0].Data.size();

            if (samples != 0)
            {
                ImPlot::SetupAxes(nullptr, nullptr, ImPlotAxisFlags_NoLabel | ImPlotAxisFlags_NoTickLabels | ImPlotAxisFlags_Lock,
                    ImPlotAxisFlags_AutoFit);
                ImPlot::SetupAxesLimits(0, static_cast<double>(samples), 0, 5.0);

                for (const auto& serial : chartObj.Series)
                    ImPlot::PlotLine(serial.Name.c_str(), serial.Data.data(), static_cast<int>(serial.Data.size()), 1, 0, 0);
            }
            ImPlot::EndPlot();
        }
    }
}
