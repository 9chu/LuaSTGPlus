/**
 * @file
 * @date 2022/3/6
 * @author 9chu
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#pragma once
#include "RenderSystem.hpp"
#include "WindowSystem.hpp"

struct SDL_Cursor;
struct ImGuiIO;
struct ImGuiContext;

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

        ImGuiContext* m_pImGuiContext = nullptr;
        std::string m_stClipboardText;
        SDL_Cursor* m_stMouseCursors[9];
        detail::MouseButtonState m_iMouseButtonState = detail::MouseButtonState::None;
        int m_iLastMouseCursor = -2;

        std::shared_ptr<DebugGUI::detail::ImGuiRenderer> m_pRenderer;
    };
}
