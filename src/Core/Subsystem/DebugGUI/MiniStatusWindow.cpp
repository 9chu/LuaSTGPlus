/**
 * @file
 * @date 2022/3/11
 * @author 9chu
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#include <lstg/Core/Subsystem/DebugGUI/MiniStatusWindow.hpp>

#include <imgui.h>
#include <lstg/Core/AppBase.hpp>  // for Cmdline

using namespace std;
using namespace lstg;
using namespace lstg::Subsystem::DebugGUI;

static const DebugWindowFlags kWindowStyle =
    DebugWindowFlags::AlwaysAutoResize |
    DebugWindowFlags::NoFocusOnAppearing |
    DebugWindowFlags::NoInputs |
    DebugWindowFlags::NoNav |
    DebugWindowFlags::NoMove |
    DebugWindowFlags::NoSavedSettings |
    DebugWindowFlags::NoTitleBar;

MiniStatusWindow::MiniStatusWindow()
    : Window("MiniStatusWindow", "MiniStatusWindow", kWindowStyle)
{
    // 加入默认的监控项
#ifdef LSTG_PLATFORM_EMSCRIPTEN
    AddCounter("LogicFPS", PerformanceCounterTypes::RealTime, "LogicFps");
    AddCounter("RenderFPS", PerformanceCounterTypes::RealTime, "RenderFps");
#else
    AddCounter("FPS", PerformanceCounterTypes::RealTime, "LogicFps");

    // 允许从命令行设置跳帧
    auto cmdRenderFrameSkip = AppBase::GetInstance().GetCmdline().GetOption<int>("render-frame-skip", 0);
    if (cmdRenderFrameSkip != 0)
        AddCounter("RenderFPS", PerformanceCounterTypes::RealTime, "RenderFps");
#endif
}

Result<void> MiniStatusWindow::AddCounter(std::string label, PerformanceCounterTypes counterType, std::string counterName) noexcept
{
    try
    {
        DrawCounters counter;
        counter.Label = std::move(label);
        counter.CounterType = counterType;
        counter.CounterName = std::move(counterName);
        m_stCounters.emplace_back(std::move(counter));
    }
    catch (...)
    {
        return make_error_code(errc::not_enough_memory);
    }
    return {};
}

void MiniStatusWindow::OnPrepareWindow() noexcept
{
    ImGui::SetNextWindowPos(ImVec2(5.f, 5.f));
}

void MiniStatusWindow::OnRender() noexcept
{
    const auto& profiler = ProfileSystem::GetInstance();

    for (const auto& item : m_stCounters)
    {
        ImGui::Text("%s", item.Label.c_str()); ImGui::SameLine();
        ImGui::Text("%.2lf", profiler.GetPerformanceCounter(item.CounterType, item.CounterName));
    }
}
