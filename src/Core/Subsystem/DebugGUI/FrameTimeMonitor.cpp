/**
 * @file
 * @date 2022/3/11
 * @author 9chu
 * 此文件为 LuaSTGPlus 项目的一部分，版权与许可声明详见 COPYRIGHT.txt。
 */
#include <lstg/Core/Subsystem/DebugGUI/FrameTimeMonitor.hpp>

#include <imgui.h>
#include <implot.h>
#include <lstg/Core/Subsystem/ProfileSystem.hpp>
#include <lstg/Core/Subsystem/Audio/BusChannel.hpp>

using namespace std;
using namespace lstg;
using namespace lstg::Subsystem::DebugGUI;

static const DebugWindowFlags kWindowStyle = DebugWindowFlags::NoSavedSettings | DebugWindowFlags::AlwaysAutoResize;

FrameTimeMonitor::FrameTimeMonitor()
    : Window("FrameTimeMonitor", "Frame Time Monitor", kWindowStyle)
{
    // 收集 200 个采样点
    m_stFrames.resize(200);
}

void FrameTimeMonitor::OnPrepareWindow() noexcept
{
    ImGui::SetNextWindowPos(ImVec2(5.f, 60.f), ImGuiCond_FirstUseEver);
}

void FrameTimeMonitor::OnUpdate(double elapsedTime) noexcept
{
    const auto& profiler = ProfileSystem::GetInstance();

    // 收集帧信息
    double total = profiler.GetLastFrameElapsedTime();
#ifdef LSTG_DEVELOPMENT
    double eventDispatchTime = profiler.GetPerformanceCounter(PerformanceCounterTypes::PerFrame, "EventDispatchTime");
    double updateTime = profiler.GetPerformanceCounter(PerformanceCounterTypes::PerFrame, "UpdateTime");
    double renderTime = profiler.GetPerformanceCounter(PerformanceCounterTypes::PerFrame, "RenderTime");
    double audioUpdateTime = profiler.GetPerformanceCounter(PerformanceCounterTypes::PerFrame, "AudioUpdateTime");
#endif

    FrameTime ft;
    ft.Total = total;
#ifdef LSTG_DEVELOPMENT
#ifdef LSTG_AUDIO_SINGLE_THREADED
    ft.EventDispatchTimeStack = eventDispatchTime + updateTime + renderTime + audioUpdateTime;
    ft.UpdateTimeStack = updateTime + renderTime + audioUpdateTime;
    ft.RenderTimeStack = renderTime + audioUpdateTime;
    ft.AudioUpdateTime = audioUpdateTime;
#else
    ft.EventDispatchTimeStack = eventDispatchTime + updateTime + renderTime;
    ft.UpdateTimeStack = updateTime + renderTime;
    ft.RenderTimeStack = renderTime;
    ft.AudioUpdateTime = audioUpdateTime;  // Multi-thread，no-stack
#endif
#endif

    m_stFrames.erase(m_stFrames.begin());
    m_stFrames.push_back(ft);
}

void FrameTimeMonitor::OnRender() noexcept
{
    if (ImPlot::BeginPlot("##NoTitle", ImVec2(350.f, 150.f)))
    {
        auto totalSamples = static_cast<int>(m_stFrames.size());
        auto beginOfTotalTime = reinterpret_cast<double*>(reinterpret_cast<uint8_t*>(m_stFrames.data()) + offsetof(FrameTime, Total));
#ifdef LSTG_DEVELOPMENT
        auto beginOfEventDispatchTime = reinterpret_cast<double*>(reinterpret_cast<uint8_t*>(m_stFrames.data()) +
            offsetof(FrameTime, EventDispatchTimeStack));
        auto beginOfEventUpdateTime = reinterpret_cast<double*>(reinterpret_cast<uint8_t*>(m_stFrames.data()) +
            offsetof(FrameTime, UpdateTimeStack));
        auto beginOfEventRenderTime = reinterpret_cast<double*>(reinterpret_cast<uint8_t*>(m_stFrames.data()) +
            offsetof(FrameTime, RenderTimeStack));
        auto beginOfEventAudioTime = reinterpret_cast<double*>(reinterpret_cast<uint8_t*>(m_stFrames.data()) +
            offsetof(FrameTime, AudioUpdateTime));
#endif

        ImPlot::SetupAxes(nullptr, nullptr, ImPlotAxisFlags_NoLabel | ImPlotAxisFlags_NoTickLabels | ImPlotAxisFlags_Lock,
            ImPlotAxisFlags_AutoFit);
        ImPlot::SetupAxesLimits(0, totalSamples, 0, 5.0);
        ImPlot::PlotLine("Frame", beginOfTotalTime, totalSamples, 1, 0, 0, sizeof(FrameTime));
#ifdef LSTG_DEVELOPMENT
        ImPlot::PlotLine("Event (stack)", beginOfEventDispatchTime, totalSamples, 1, 0, 0, sizeof(FrameTime));
        ImPlot::PlotLine("Update (stack)", beginOfEventUpdateTime, totalSamples, 1, 0, 0, sizeof(FrameTime));
        ImPlot::PlotLine("Render (stack)", beginOfEventRenderTime, totalSamples, 1, 0, 0, sizeof(FrameTime));
#ifdef LSTG_AUDIO_SINGLE_THREADED
        ImPlot::PlotLine("Audio (stack)", beginOfEventAudioTime, totalSamples, 1, 0, 0, sizeof(FrameTime));
#else
        ImPlot::PlotLine("Audio", beginOfEventAudioTime, totalSamples, 1, 0, 0, sizeof(FrameTime));
#endif
#endif
        ImPlot::EndPlot();
    }
}
