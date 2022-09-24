/**
 * @file
 * @date 2022/3/11
 * @author 9chu
 * 此文件为 LuaSTGPlus 项目的一部分，版权与许可声明详见 COPYRIGHT.txt。
 */
#include <lstg/Core/Subsystem/DebugGUI/Window.hpp>

#include <imgui.h>

using namespace std;
using namespace lstg;
using namespace lstg::Subsystem::DebugGUI;

Window::Window(const char* name, const char* title, DebugWindowFlags flags)
    : m_stName(name), m_stTitle(title), m_uFlags(flags)
{
}

void Window::Update(double elapsedTime) noexcept
{
    OnUpdate(elapsedTime);
}

void Window::Render() noexcept
{
    if (!m_bVisible)
        return;

    OnPrepareWindow();

    if (!ImGui::Begin(m_stTitle.c_str(), &m_bVisible, static_cast<int>(m_uFlags)))
    {
        ImGui::End();
        return;
    }
    OnRender();
    ImGui::End();
}

void Window::OnPrepareWindow() noexcept
{
}

void Window::OnUpdate(double elapsedTime) noexcept
{
    static_cast<void>(elapsedTime);
}

void Window::OnRender() noexcept
{
}
