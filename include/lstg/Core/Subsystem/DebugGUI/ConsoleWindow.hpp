/**
 * @file
 * @date 2022/3/11
 * @author 9chu
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#pragma once
#include <optional>
#include <vector>
#include <functional>
#include "Window.hpp"
#include "../../Logging.hpp"

struct ImGuiInputTextCallbackData;

namespace lstg::Subsystem::DebugGUI
{
    namespace detail
    {
        class ConsoleLogSink;
    }

    using ConsoleContextMenuCallback = std::function<void() /* noexcept */>;

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
         * 增加上下文菜单项
         * @param item 菜单名
         * @param callback 回调
         */
        Result<void> AddContextMenuItem(std::string_view item, ConsoleContextMenuCallback callback) noexcept;

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

        // 上下文菜单
        std::vector<std::pair<std::string, ConsoleContextMenuCallback>> m_stContextMenuItems;

        // 历史
        std::optional<size_t> m_stCurrentSelectHistoryIndex;
        std::vector<std::string> m_stHistory;
    };
}
