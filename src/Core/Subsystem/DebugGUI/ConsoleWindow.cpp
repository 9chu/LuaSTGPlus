/**
 * @file
 * @date 2022/3/11
 * @author 9chu
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#include <lstg/Core/Subsystem/DebugGUI/ConsoleWindow.hpp>

#include <atomic>
#include <deque>
#include <string>
#include <mutex>
#include <imgui.h>
#include <lstg/Core/AppBase.hpp>
#include <lstg/Core/Subsystem/DebugGUISystem.hpp>
#include <lstg/Core/Subsystem/ScriptSystem.hpp>
#include <lstg/Core/Subsystem/Script/LuaState.hpp>
#include <lstg/Core/Subsystem/Script/LuaRead.hpp>

using namespace std;
using namespace lstg;
using namespace lstg::Subsystem::DebugGUI;
using namespace lstg::Subsystem::Script;

static const DebugWindowFlags kWindowStyle = DebugWindowFlags::NoSavedSettings;

#ifdef LSTG_DEVELOPMENT
static const unsigned kMaxLogItems = 200;
static const unsigned kMaxHistoryItems = 100;
#else
static const unsigned kMaxLogItems = 50;
static const unsigned kMaxHistoryItems = 10;
#endif

static const char* kScriptReturn = "return ";
static const unsigned kScriptReturnLen = strlen(kScriptReturn);

static const ImVec4 kColorTrace {180.f / 255.f, 180.f / 255.f, 180.f / 255.f, 1.0f};
static const ImVec4 kColorDebug {230.f / 255.f, 230.f / 255.f, 230.f / 255.f, 1.0f};
static const ImVec4 kColorInfo {0.f / 255.f, 160.f / 255.f, 255.f / 255.f, 1.0f};
static const ImVec4 kColorWarn {250.f / 255.f, 230.f / 255.f, 0.f / 255.f, 1.0f};
static const ImVec4 kColorError {255.f / 255.f, 100.f / 255.f, 100.f / 255.f, 1.0f};
static const ImVec4 kColorFatal {255.f / 255.f, 0.f / 255.f, 0.f / 255.f, 1.0f};
static const ImVec4 kColorUserInput {1.f, 1.f, 1.f, 1.f};

namespace lstg::Subsystem::DebugGUI::detail
{
    class ConsoleLogSink :
        public Logging::ICustomSink
    {
    public:
        struct LogItem
        {
            ImVec4 Color;
            string Content;
        };

        class LogItemAccessor
        {
        public:
            LogItemAccessor(mutex& lock, const deque<LogItem>& logs)
                : m_stLock(lock), m_stLogs(logs), m_stLockGuard(m_stLock)
            {
            }

            const deque<LogItem>& operator*() const noexcept { return m_stLogs; }

        private:
            mutex& m_stLock;
            const deque<LogItem>& m_stLogs;
            unique_lock<mutex> m_stLockGuard;
        };

    public:
        ConsoleLogSink()
        {
            SetErrorFlag(false);
        }

    public:
        LogItemAccessor AccessItems() const noexcept
        {
            return { m_stLock, m_stLogs };
        }

        void Clear() noexcept
        {
            unique_lock<mutex> lock(m_stLock);
            m_stLogs.clear();
        }

        void AppendRaw(ImVec4 color, std::string content)
        {
            unique_lock<mutex> lock(m_stLock);

            // 不超过 kMaxLogItems 条
            if (m_stLogs.size() >= kMaxLogItems)
                m_stLogs.pop_front();

            LogItem item;
            item.Color = color;
            item.Content = std::move(content);

            m_stLogs.emplace_back(std::move(item));
        }

        bool IsErrorFlagSet() const noexcept
        {
#ifdef LSTG_DEVELOPMENT
            return m_bErrorSet.load(std::memory_order_relaxed);
#endif
            return false;
        }

        void SetErrorFlag(bool f) noexcept
        {
#ifdef LSTG_DEVELOPMENT
            m_bErrorSet.store(f, std::memory_order_relaxed);
#endif
        }

    public:  // ICustomSink
        void Sink(const lstg::detail::LogMessage& message) noexcept override
        {
            try
            {
                ImVec4 color;
                switch (message.Level)
                {
                    case LogLevel::Trace:
                        color = kColorTrace;
                        break;
                    case LogLevel::Debug:
                        color = kColorDebug;
                        break;
                    case LogLevel::Info:
                        color = kColorInfo;
                        break;
                    case LogLevel::Warn:
                        color = kColorWarn;
                        break;
                    case LogLevel::Error:
                        color = kColorError;
                        SetErrorFlag(true);
                        break;
                    case LogLevel::Critical:
                        color = kColorFatal;
                        SetErrorFlag(true);
                        break;
                    default:
                        assert(false);
                        color = kColorTrace;
                        break;
                }

                AppendRaw(color, fmt::format("{} - {}", message.CategoryName, message.Payload));
            }
            catch (...)
            {
            }
        }

    private:
        mutable mutex m_stLock;
        deque<LogItem> m_stLogs;

#ifdef LSTG_DEVELOPMENT
        std::atomic<bool> m_bErrorSet;
#endif
    };
}

namespace
{
    /**
     * 字符串剔除
     * @param input 输入串
     * @return 剔除前后空白的输出串
     */
    string_view StrTrimRight(string_view input) noexcept
    {
        auto begin = input.begin();
        auto end = input.end();
        while (begin != end)
        {
            auto ch = *(end - 1);
            if (ch == ' ' || ch == '\t')
                --end;
            else
                break;
        }
        return {&*begin, static_cast<size_t>(end - begin)};
    }

    /**
     * 执行脚本
     * @param sourceCode 被执行脚本源码
     * @param errorOutput 错误输出
     * @return 结果或者错误
     */
    Result<std::string> ExecScript(std::string_view sourceCode, std::string& errorOutput) noexcept
    {
        // 由于我们并不知道传入的 Lua 脚本是一个 Expression 还是一个 Statement
        // 因此我们总是在 sourceCode 之前追加一个 "return " 前缀将 Expression 的返回值作为块的执行结果
        // 这一过程由外部保证
        // 如果这步编译失败，则按照 Statement 进行处理
        assert(::memcmp(sourceCode.data(), kScriptReturn, kScriptReturnLen) == 0);

        // 获取全局 Lua 状态
        auto& instance = AppBase::GetInstance();
        auto& L = instance.GetSubsystem<Subsystem::ScriptSystem>()->GetState();
        LuaStack::BalanceChecker stackChecker(L);

        // 先按照 Expression 对待
        bool isStatement = false;
        lua_checkstack(L, 2);
        {
            auto load = luaL_loadbuffer(L, sourceCode.data(), sourceCode.size(), "@console");
            if (load != 0)
            {
                // 编译失败，假定这是个 Statement
                lua_pop(L, 1);
                isStatement = true;
            }
        }
        if (isStatement)
        {
            // 减掉 "return " 前缀
            sourceCode = sourceCode.substr(kScriptReturnLen);

            auto load = luaL_loadbuffer(L, sourceCode.data(), sourceCode.size(), "@console");
            if (load != 0)
            {
                // 编译失败，可以报错了
                try
                {
                    errorOutput = L.ReadValue<string>(-1);
                }
                catch (...)
                {
                }
                lua_pop(L, 1);
                return make_error_code(static_cast<LuaError>(load));
            }
        }
        assert(lua_type(L, -1) == LUA_TFUNCTION);

        // 执行
        auto call = L.ProtectedCallWithTraceback(0, 1);
        if (!call)
        {
            // 执行失败，报错
            try
            {
                errorOutput = L.ReadValue<string>(-1);
            }
            catch (...)
            {
            }
            lua_pop(L, 1);
            return call.GetError();
        }

        // 将栈上的内容转成文本进行输出
        string ret;
        try
        {
            size_t len = 0;
            const char* what = lua_tolstring(L, -1, &len);
            ret = string{what, len};
        }
        catch (...)
        {
        }
        lua_pop(L, 1);
        return ret;
    }
}

ConsoleWindow::ConsoleWindow()
    : Window("ConsoleWindow", "Console", kWindowStyle)
{
    // 设置一个 Sink 接受全局日志输出
    m_pConsoleLogSink = make_shared<detail::ConsoleLogSink>();
    Logging::GetInstance().AddCustomSink(m_pConsoleLogSink);

    strcpy(m_stInputBuffer, kScriptReturn);

    // 内建窗口开关
    AddContextMenuItem("Toggle MiniStatusWindow", []() {
        auto& instance = AppBase::GetInstance();
        auto& debug = *instance.GetSubsystem<Subsystem::DebugGUISystem>();
        auto window = debug.GetMiniStatusWindow();
        window->IsVisible() ? window->Hide() : window->Show();
    });
    AddContextMenuItem("Toggle FrameTimeMonitor", []() {
        auto& instance = AppBase::GetInstance();
        auto& debug = *instance.GetSubsystem<Subsystem::DebugGUISystem>();
        auto window = debug.GetFrameTimeMonitor();
        window->IsVisible() ? window->Hide() : window->Show();
    });

    // 初始化历史数
    m_stHistory.reserve(kMaxHistoryItems);
}

ConsoleWindow::~ConsoleWindow()
{
    bool ok = Logging::GetInstance().RemoveCustomSink(m_pConsoleLogSink.get());
    static_cast<void>(ok);
    assert(ok);
}

Result<void> ConsoleWindow::AddContextMenuItem(std::string_view item, ConsoleContextMenuCallback callback) noexcept
{
    try
    {
        m_stContextMenuItems.emplace_back(std::make_pair<string, ConsoleContextMenuCallback>(string{item}, std::move(callback)));
    }
    catch (...)
    {
        return make_error_code(errc::not_enough_memory);
    }
    return {};
}

void ConsoleWindow::ClearLog() noexcept
{
    m_pConsoleLogSink->Clear();
}

void ConsoleWindow::OnPrepareWindow() noexcept
{
    ImGui::SetNextWindowPos(ImVec2(330, 10), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(300, 400), ImGuiCond_FirstUseEver);
}

void ConsoleWindow::OnUpdate(double elapsedTime) noexcept
{
    static_cast<void>(elapsedTime);

#ifdef LSTG_DEVELOPMENT
    // 发生错误自动弹框
    if (m_pConsoleLogSink->IsErrorFlagSet())
    {
        m_pConsoleLogSink->SetErrorFlag(false);

        if (!IsVisible())
            Show();
    }
#endif
}

void ConsoleWindow::OnRender() noexcept
{
    {
        const float footerHeightToReserve = ImGui::GetStyle().ItemSpacing.y + ImGui::GetFrameHeightWithSpacing();
        ImGui::BeginChild("ScrollingRegion", ImVec2(0, -footerHeightToReserve), false, ImGuiWindowFlags_None);
        if (ImGui::BeginPopupContextWindow())
        {
            if (!m_stContextMenuItems.empty())
            {
                for (auto& e : m_stContextMenuItems)
                {
                    if (ImGui::Selectable(e.first.c_str()))
                    {
                        if (e.second)
                        {
                            try
                            {
                                e.second();
                            }
                            catch (...)
                            {
                                assert(false);
                            }
                        }
                    }
                }
                ImGui::Separator();
            }
            if (ImGui::Selectable("Clear"))
                ClearLog();
            ImGui::EndPopup();
        }

        // FIXME: 用 ImGuiListClipper 进行裁切可以提高性能
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4, 1));  // 更小的间距
        {
            auto accessor = m_pConsoleLogSink->AccessItems();
            for (const auto& e : *accessor)
            {
                ImGui::PushStyleColor(ImGuiCol_Text, e.Color);
                ImGui::TextWrapped("%s", e.Content.c_str());
                ImGui::PopStyleColor();
            }
        }
        if (m_bScrollToBottomNextTime || (ImGui::GetScrollY() >= ImGui::GetScrollMaxY()))
            ImGui::SetScrollHereY(1.0f);
        m_bScrollToBottomNextTime = false;
        ImGui::PopStyleVar();
        ImGui::EndChild();
    }

    ImGui::Separator();

    bool reclaimFocus = false;
    ImGuiInputTextFlags inputTextFlags = ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_CallbackCompletion |
        ImGuiInputTextFlags_CallbackHistory | ImGuiInputTextFlags_CallbackEdit;
    if (ImGui::InputText("##EnterCmd", m_stInputBuffer + kScriptReturnLen, IM_ARRAYSIZE(m_stInputBuffer), inputTextFlags,
        [](ImGuiInputTextCallbackData* data) {
            auto self = static_cast<ConsoleWindow*>(data->UserData);
            return self->OnTextEdit(data);
        }, static_cast<void*>(this)))
    {
        ExecCommand();
        reclaimFocus = true;
    }
    ImGui::SetItemDefaultFocus();
    ImGui::SameLine();
    if (ImGui::Button("OK"))
    {
        ExecCommand();
        reclaimFocus = true;
    }

    if (reclaimFocus)
        ImGui::SetKeyboardFocusHere(-1);  // 自动聚焦到上一个控件
}

void ConsoleWindow::ExecCommand() noexcept
{
    try
    {
        auto cmd = StrTrimRight(m_stInputBuffer);
        if (cmd.length() > kScriptReturnLen)
        {
            auto inputCmd = string_view {cmd.data() + kScriptReturnLen, cmd.size() - kScriptReturnLen};

            // 追加输入
            m_pConsoleLogSink->AppendRaw(kColorUserInput, fmt::format("] {}", inputCmd));

            // 写入历史
            try
            {
                m_stCurrentSelectHistoryIndex = {};
                if (m_stHistory.size() >= kMaxHistoryItems)
                    m_stHistory.erase(m_stHistory.begin());
                m_stHistory.emplace_back(inputCmd);
            }
            catch (...)  // bad_alloc
            {
            }

            // 执行脚本
            string errorOutput;
            auto ret = ExecScript(cmd, errorOutput);
            if (!ret)
                m_pConsoleLogSink->AppendRaw(kColorError, std::move(errorOutput));
            else
                m_pConsoleLogSink->AppendRaw(kColorUserInput, std::move(*ret));
        }
        strcpy(m_stInputBuffer, kScriptReturn);
    }
    catch (...)  // bad_alloc
    {
    }
}

int ConsoleWindow::OnTextEdit(ImGuiInputTextCallbackData* data) noexcept
{
    switch (data->EventFlag)
    {
        case ImGuiInputTextFlags_CallbackCompletion:
            // FIXME: 实现自动完成
            break;
        case ImGuiInputTextFlags_CallbackHistory:
            if (m_stHistory.empty())
            {
                m_stCurrentSelectHistoryIndex = {};
            }
            else
            {
                if (m_stCurrentSelectHistoryIndex && *m_stCurrentSelectHistoryIndex >= m_stHistory.size())
                    m_stCurrentSelectHistoryIndex = {};

                if (data->EventKey == ImGuiKey_UpArrow)
                {
                    if (!m_stCurrentSelectHistoryIndex)
                    {
                        m_stCurrentSelectHistoryIndex = m_stHistory.size() - 1;
                    }
                    else
                    {
                        assert(!m_stHistory.empty());
                        if (*m_stCurrentSelectHistoryIndex == 0)
                        {
                            m_stCurrentSelectHistoryIndex = m_stHistory.size() - 1;
                        }
                        else
                        {
                            m_stCurrentSelectHistoryIndex = *m_stCurrentSelectHistoryIndex - 1;
                        }
                    }
                }
                else if (data->EventKey == ImGuiKey_DownArrow)
                {
                    if (!m_stCurrentSelectHistoryIndex)
                    {
                        m_stCurrentSelectHistoryIndex = 0;
                    }
                    else
                    {
                        assert(!m_stHistory.empty());
                        if (*m_stCurrentSelectHistoryIndex >= m_stHistory.size() - 1)
                        {
                            m_stCurrentSelectHistoryIndex = 0;
                        }
                        else
                        {
                            m_stCurrentSelectHistoryIndex = *m_stCurrentSelectHistoryIndex + 1;
                        }
                    }
                }

                assert(m_stCurrentSelectHistoryIndex);
                string& selectedHistory = m_stHistory[*m_stCurrentSelectHistoryIndex];
                data->DeleteChars(0, data->BufTextLen);
                data->InsertChars(0, selectedHistory.c_str());
                data->SelectionStart = data->SelectionEnd = data->BufTextLen;
            }
            break;
        case ImGuiInputTextFlags_CallbackEdit:
            m_stCurrentSelectHistoryIndex = {};
            break;
    }
    return 0;
}
