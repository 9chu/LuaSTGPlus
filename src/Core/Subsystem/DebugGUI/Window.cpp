/**
 * @file
 * @date 2022/3/11
 * @author 9chu
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#include <lstg/Core/Subsystem/DebugGUI/Window.hpp>

#include <imgui.h>

using namespace std;
using namespace lstg;
using namespace lstg::Subsystem::DebugGUI;

Window::Window(const char* name, DebugWindowFlags flags)
    : m_stName(name), m_uFlags(flags)
{
}

void Window::Render() noexcept
{
    if (!m_bVisible)
        return;

    if (!ImGui::Begin(m_stName.c_str(), &m_bVisible, static_cast<int>(m_uFlags)))
        ImGui::End();
    OnDraw();
    ImGui::End();
}

void Window::OnDraw() noexcept
{
}
