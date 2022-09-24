/**
 * @file
 * @date 2022/8/16
 * @author 9chu
 * 此文件为 LuaSTGPlus 项目的一部分，版权与许可声明详见 COPYRIGHT.txt。
 */
#include <lstg/Core/Subsystem/DebugGUI/ProgressWindow.hpp>

#include <imgui.h>

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
    DebugWindowFlags::NoTitleBar |
    DebugWindowFlags::NoBackground;

ProgressWindow::ProgressWindow()
    : Window("ProgressWindow", "ProgressWindow", kWindowStyle)
{
}

Result<void> ProgressWindow::SetHintText(std::string_view text) noexcept
{
    try
    {
        m_stHintText = text;
    }
    catch (...)  // bad_alloc
    {
        return make_error_code(errc::not_enough_memory);
    }
    return {};
}

void ProgressWindow::OnPrepareWindow() noexcept
{
    auto sz = ImGui::GetIO().DisplaySize;
    ImGui::SetNextWindowPos(ImVec2{sz.x * 0.5f, sz.y * 0.75f}, 0, ImVec2{0.5f, 0.f});
}

void ProgressWindow::OnRender() noexcept
{
    ImGui::Text("%s", m_stHintText.c_str());
    ImGui::Spacing();
    ImGui::ProgressBar(m_fPercent);
}
