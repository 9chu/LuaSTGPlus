/**
 * @file
 * @date 2022/3/11
 * @author 9chu
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#pragma once
#include "Window.hpp"
#include "../../Logging.hpp"

struct ImGuiInputTextCallbackData;

namespace lstg::Subsystem::DebugGUI
{
    namespace detail
    {
        class ConsoleLogSink;
    }

    /**
     * 控制台窗口
     */
    class ConsoleWindow :
        public Window
    {
    public:
        ConsoleWindow();
        ~ConsoleWindow();

    public:
        /**
         * 清空日志
         */
        void ClearLog() noexcept;

    protected:  // Window
        void OnPrepareWindow() noexcept override;
        void OnUpdate(double elapsedTime) noexcept override;
        void OnRender() noexcept override;

    private:
        void ExecCommand() noexcept;
        int OnTextEdit(ImGuiInputTextCallbackData* data) noexcept;

    private:
        bool m_bScrollToBottomNextTime = true;
        char m_stInputBuffer[1024];
        std::shared_ptr<detail::ConsoleLogSink> m_pConsoleLogSink;
    };
}
