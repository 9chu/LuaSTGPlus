/**
 * @file
 * @date 2022/8/16
 * @author 9chu
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
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
