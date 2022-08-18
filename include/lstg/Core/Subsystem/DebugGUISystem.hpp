/**
 * @file
 * @date 2022/3/6
 * @author 9chu
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#pragma once
#include <map>
#include <string_view>
#include "RenderSystem.hpp"
#include "WindowSystem.hpp"
#include "DebugGUI/Window.hpp"
#include "DebugGUI/MiniStatusWindow.hpp"
#include "DebugGUI/FrameTimeMonitor.hpp"
#include "DebugGUI/ConsoleWindow.hpp"
#include "DebugGUI/ProgressWindow.hpp"

struct SDL_Cursor;
struct ImGuiIO;
struct ImGuiContext;
struct ImPlotContext;

namespace lstg::Subsystem
{
    namespace detail
    {
        LSTG_FLAG_BEGIN(MouseButtonState)
            None = 0,
            LeftDown = 1,
            RightDown = 2,
            MiddleDown = 4,
        LSTG_FLAG_END(MouseButtonState)
    }

    namespace DebugGUI::detail
    {
        class ImGuiRenderer;
    }

    /**
     * 调试 UI 系统
     * 基于 IMGUI 的调试用 UI 系统。
     */
    class DebugGUISystem :
        public ISubsystem
    {
    public:
        DebugGUISystem(SubsystemContainer& container);
        ~DebugGUISystem() override;

    public:
        /**
         * 增加窗体
         * @param window 窗口
         * @return 是否已经存在
         */
        Result<bool> AppendWindow(std::shared_ptr<DebugGUI::Window> window) noexcept;

        /**
         * 寻找窗口
         * @param name 窗口名称
         * @return 存在则返回对应的窗口对象
         */
        std::shared_ptr<DebugGUI::Window> FindWindow(std::string_view name) noexcept;

        /**
         * 获取迷你状态窗口
         */
        [[nodiscard]] const auto& GetMiniStatusWindow() const noexcept { return m_pMiniStatusWindow; }

        /**
         * 获取帧时间监视器窗口
         */
        [[nodiscard]] const auto& GetFrameTimeMonitor() const noexcept { return m_pFrameTimeMonitor; }

        /**
         * 获取控制台窗口
         */
        [[nodiscard]] const auto& GetConsoleWindow() const noexcept { return m_pConsoleWindow; }

        /**
         * 获取进度窗口
         */
        [[nodiscard]] const auto& GetProgressWindow() const noexcept { return m_pProgressWindow; }

    protected:  // ISubsystem
        void OnUpdate(double elapsedTime) noexcept override;
        void OnAfterRender(double elapsedTime) noexcept override;
        void OnEvent(SubsystemEvent& event) noexcept override;

    private:
        void AdjustViewSize(ImGuiIO& io) noexcept;
        const char* ImGuiGetClipboardText() noexcept;
        void ImGuiSetClipboardText(const char* text) noexcept;

    private:
        std::shared_ptr<WindowSystem> m_pWindowSystem;
        std::shared_ptr<RenderSystem> m_pRenderSystem;

        // ImGui 状态
        ImGuiContext* m_pImGuiContext = nullptr;
        ImPlotContext* m_pImPlotContext = nullptr;
        std::string m_stClipboardText;
        SDL_Cursor* m_stMouseCursors[9];
        detail::MouseButtonState m_iMouseButtonState = detail::MouseButtonState::None;
        int m_iLastMouseCursor = -2;

        // 渲染器
        std::shared_ptr<DebugGUI::detail::ImGuiRenderer> m_pRenderer;

        // 子窗口
        std::map<std::string, std::shared_ptr<DebugGUI::Window>, std::less<>> m_stWindows;

        // 内建窗口
        std::shared_ptr<DebugGUI::MiniStatusWindow> m_pMiniStatusWindow;
        std::shared_ptr<DebugGUI::FrameTimeMonitor> m_pFrameTimeMonitor;
        std::shared_ptr<DebugGUI::ConsoleWindow> m_pConsoleWindow;
        std::shared_ptr<DebugGUI::ProgressWindow> m_pProgressWindow;
    };
}
