/**
 * @file
 * @date 2022/3/6
 * @author 9chu
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#include <lstg/Core/Subsystem/DebugGUISystem.hpp>

#include <SDL.h>
#include <SDL_syswm.h>
#include <imgui.h>
#include <implot.h>
#include <lstg/Core/Logging.hpp>
#include <lstg/Core/Subsystem/SubsystemContainer.hpp>
#include <lstg/Core/Subsystem/ProfileSystem.hpp>
#include "DebugGUI/detail/ImGuiRenderer.hpp"

#ifdef FindWindow
#undef FindWindow
#endif

using namespace std;
using namespace lstg;
using namespace lstg::Subsystem;

LSTG_DEF_LOG_CATEGORY(DebugGUISystem);

namespace
{
    void UpdateKeyModifiers(ImGuiIO& io, SDL_Keymod modifier) noexcept
    {
        io.AddKeyEvent(ImGuiKey_ModCtrl, (modifier & KMOD_CTRL) != 0);
        io.AddKeyEvent(ImGuiKey_ModShift, (modifier & KMOD_SHIFT) != 0);
        io.AddKeyEvent(ImGuiKey_ModAlt, (modifier & KMOD_ALT) != 0);
        io.AddKeyEvent(ImGuiKey_ModSuper, (modifier & KMOD_GUI) != 0);
    }

    ImGuiKey SDLKeyCodeToImGui(int keycode) noexcept
    {
        switch (keycode)
        {
            case SDLK_TAB: return ImGuiKey_Tab;
            case SDLK_LEFT: return ImGuiKey_LeftArrow;
            case SDLK_RIGHT: return ImGuiKey_RightArrow;
            case SDLK_UP: return ImGuiKey_UpArrow;
            case SDLK_DOWN: return ImGuiKey_DownArrow;
            case SDLK_PAGEUP: return ImGuiKey_PageUp;
            case SDLK_PAGEDOWN: return ImGuiKey_PageDown;
            case SDLK_HOME: return ImGuiKey_Home;
            case SDLK_END: return ImGuiKey_End;
            case SDLK_INSERT: return ImGuiKey_Insert;
            case SDLK_DELETE: return ImGuiKey_Delete;
            case SDLK_BACKSPACE: return ImGuiKey_Backspace;
            case SDLK_SPACE: return ImGuiKey_Space;
            case SDLK_RETURN: return ImGuiKey_Enter;
            case SDLK_ESCAPE: return ImGuiKey_Escape;
            case SDLK_QUOTE: return ImGuiKey_Apostrophe;
            case SDLK_COMMA: return ImGuiKey_Comma;
            case SDLK_MINUS: return ImGuiKey_Minus;
            case SDLK_PERIOD: return ImGuiKey_Period;
            case SDLK_SLASH: return ImGuiKey_Slash;
            case SDLK_SEMICOLON: return ImGuiKey_Semicolon;
            case SDLK_EQUALS: return ImGuiKey_Equal;
            case SDLK_LEFTBRACKET: return ImGuiKey_LeftBracket;
            case SDLK_BACKSLASH: return ImGuiKey_Backslash;
            case SDLK_RIGHTBRACKET: return ImGuiKey_RightBracket;
            case SDLK_BACKQUOTE: return ImGuiKey_GraveAccent;
            case SDLK_CAPSLOCK: return ImGuiKey_CapsLock;
            case SDLK_SCROLLLOCK: return ImGuiKey_ScrollLock;
            case SDLK_NUMLOCKCLEAR: return ImGuiKey_NumLock;
            case SDLK_PRINTSCREEN: return ImGuiKey_PrintScreen;
            case SDLK_PAUSE: return ImGuiKey_Pause;
            case SDLK_KP_0: return ImGuiKey_Keypad0;
            case SDLK_KP_1: return ImGuiKey_Keypad1;
            case SDLK_KP_2: return ImGuiKey_Keypad2;
            case SDLK_KP_3: return ImGuiKey_Keypad3;
            case SDLK_KP_4: return ImGuiKey_Keypad4;
            case SDLK_KP_5: return ImGuiKey_Keypad5;
            case SDLK_KP_6: return ImGuiKey_Keypad6;
            case SDLK_KP_7: return ImGuiKey_Keypad7;
            case SDLK_KP_8: return ImGuiKey_Keypad8;
            case SDLK_KP_9: return ImGuiKey_Keypad9;
            case SDLK_KP_PERIOD: return ImGuiKey_KeypadDecimal;
            case SDLK_KP_DIVIDE: return ImGuiKey_KeypadDivide;
            case SDLK_KP_MULTIPLY: return ImGuiKey_KeypadMultiply;
            case SDLK_KP_MINUS: return ImGuiKey_KeypadSubtract;
            case SDLK_KP_PLUS: return ImGuiKey_KeypadAdd;
            case SDLK_KP_ENTER: return ImGuiKey_KeypadEnter;
            case SDLK_KP_EQUALS: return ImGuiKey_KeypadEqual;
            case SDLK_LCTRL: return ImGuiKey_LeftCtrl;
            case SDLK_LSHIFT: return ImGuiKey_LeftShift;
            case SDLK_LALT: return ImGuiKey_LeftAlt;
            case SDLK_LGUI: return ImGuiKey_LeftSuper;
            case SDLK_RCTRL: return ImGuiKey_RightCtrl;
            case SDLK_RSHIFT: return ImGuiKey_RightShift;
            case SDLK_RALT: return ImGuiKey_RightAlt;
            case SDLK_RGUI: return ImGuiKey_RightSuper;
            case SDLK_APPLICATION: return ImGuiKey_Menu;
            case SDLK_0: return ImGuiKey_0;
            case SDLK_1: return ImGuiKey_1;
            case SDLK_2: return ImGuiKey_2;
            case SDLK_3: return ImGuiKey_3;
            case SDLK_4: return ImGuiKey_4;
            case SDLK_5: return ImGuiKey_5;
            case SDLK_6: return ImGuiKey_6;
            case SDLK_7: return ImGuiKey_7;
            case SDLK_8: return ImGuiKey_8;
            case SDLK_9: return ImGuiKey_9;
            case SDLK_a: return ImGuiKey_A;
            case SDLK_b: return ImGuiKey_B;
            case SDLK_c: return ImGuiKey_C;
            case SDLK_d: return ImGuiKey_D;
            case SDLK_e: return ImGuiKey_E;
            case SDLK_f: return ImGuiKey_F;
            case SDLK_g: return ImGuiKey_G;
            case SDLK_h: return ImGuiKey_H;
            case SDLK_i: return ImGuiKey_I;
            case SDLK_j: return ImGuiKey_J;
            case SDLK_k: return ImGuiKey_K;
            case SDLK_l: return ImGuiKey_L;
            case SDLK_m: return ImGuiKey_M;
            case SDLK_n: return ImGuiKey_N;
            case SDLK_o: return ImGuiKey_O;
            case SDLK_p: return ImGuiKey_P;
            case SDLK_q: return ImGuiKey_Q;
            case SDLK_r: return ImGuiKey_R;
            case SDLK_s: return ImGuiKey_S;
            case SDLK_t: return ImGuiKey_T;
            case SDLK_u: return ImGuiKey_U;
            case SDLK_v: return ImGuiKey_V;
            case SDLK_w: return ImGuiKey_W;
            case SDLK_x: return ImGuiKey_X;
            case SDLK_y: return ImGuiKey_Y;
            case SDLK_z: return ImGuiKey_Z;
            case SDLK_F1: return ImGuiKey_F1;
            case SDLK_F2: return ImGuiKey_F2;
            case SDLK_F3: return ImGuiKey_F3;
            case SDLK_F4: return ImGuiKey_F4;
            case SDLK_F5: return ImGuiKey_F5;
            case SDLK_F6: return ImGuiKey_F6;
            case SDLK_F7: return ImGuiKey_F7;
            case SDLK_F8: return ImGuiKey_F8;
            case SDLK_F9: return ImGuiKey_F9;
            case SDLK_F10: return ImGuiKey_F10;
            case SDLK_F11: return ImGuiKey_F11;
            case SDLK_F12: return ImGuiKey_F12;
            default: return ImGuiKey_None;
        }
    }
}

DebugGUISystem::DebugGUISystem(SubsystemContainer& container)
    : m_pWindowSystem(container.Get<WindowSystem>()), m_pRenderSystem(container.Get<RenderSystem>())
{
    assert(m_pWindowSystem && m_pRenderSystem);
    container.Get<ProfileSystem>();  // 强制构造 ProfileSystem

    // 初始化 imgui
    LSTG_LOG_TRACE_CAT(DebugGUISystem, "Initialize ImGui");
    m_pImGuiContext = ImGui::CreateContext();
    assert(m_pImGuiContext);

    // 初始化 implot
    m_pImPlotContext = ImPlot::CreateContext();
    assert(m_pImPlotContext);

    // 配置 IO
    ImGuiIO& io = ImGui::GetIO();
    io.BackendPlatformUserData = this;
    io.BackendPlatformName = "LSTGPlus/SDL";
    io.BackendFlags |= ImGuiBackendFlags_HasMouseCursors;
    io.BackendFlags |= ImGuiBackendFlags_HasSetMousePos;
    io.IniFilename = nullptr;  // 关闭存储

    // 构造渲染器
    m_pRenderer = make_shared<lstg::Subsystem::DebugGUI::detail::ImGuiRenderer>(m_pRenderSystem->GetRenderDevice(), io);

    // 注册剪贴板方法
    io.ClipboardUserData = this;
    io.GetClipboardTextFn = [](void* ud) {
        auto self = static_cast<DebugGUISystem*>(ud);
        return self->ImGuiGetClipboardText();
    };
    io.SetClipboardTextFn = [](void* ud, const char* text) {
        auto self = static_cast<DebugGUISystem*>(ud);
        self->ImGuiSetClipboardText(text);
    };

    // 加载系统鼠标指针
    static_assert(ImGuiMouseCursor_COUNT == 9);
    m_stMouseCursors[ImGuiMouseCursor_Arrow] = ::SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_ARROW);
    m_stMouseCursors[ImGuiMouseCursor_TextInput] = ::SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_IBEAM);
    m_stMouseCursors[ImGuiMouseCursor_ResizeAll] = ::SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZEALL);
    m_stMouseCursors[ImGuiMouseCursor_ResizeNS] = ::SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZENS);
    m_stMouseCursors[ImGuiMouseCursor_ResizeEW] = ::SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZEWE);
    m_stMouseCursors[ImGuiMouseCursor_ResizeNESW] = ::SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZENESW);
    m_stMouseCursors[ImGuiMouseCursor_ResizeNWSE] = ::SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZENWSE);
    m_stMouseCursors[ImGuiMouseCursor_Hand] = ::SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_HAND);
    m_stMouseCursors[ImGuiMouseCursor_NotAllowed] = ::SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_NO);

    // 使得 Windows 环境 IME 可以正常跟踪光标位置
    // https://github.com/ocornut/imgui/blob/master/docs/FAQ.md
#ifdef LSTG_PLATFORM_WIN32
    SDL_SysWMinfo systemWindowInfo;
    SDL_VERSION(&systemWindowInfo.version);
    if (::SDL_GetWindowWMInfo(m_pWindowSystem->GetNativeHandle(), &systemWindowInfo))
        ImGui::GetMainViewport()->PlatformHandleRaw = reinterpret_cast<void*>(systemWindowInfo.info.win.window);
#endif

    // 告知 SDL 在获取焦点的瞬间也要发出一个鼠标点击事件
    ::SDL_SetHint(SDL_HINT_MOUSE_FOCUS_CLICKTHROUGH, "1");

    // 默认工具窗口
    {
        m_pMiniStatusWindow = make_shared<DebugGUI::MiniStatusWindow>();
        AppendWindow(m_pMiniStatusWindow);

        m_pFrameTimeMonitor = make_shared<DebugGUI::FrameTimeMonitor>();
        AppendWindow(m_pFrameTimeMonitor);

        m_pConsoleWindow = make_shared<DebugGUI::ConsoleWindow>();
        AppendWindow(m_pConsoleWindow);

#ifdef LSTG_DEVELOPMENT
        m_pMiniStatusWindow->Show();
#endif
    }
}

DebugGUISystem::~DebugGUISystem()
{
    ImGuiIO& io = ImGui::GetIO();

    // 销毁所有鼠标指针
    for (auto p : m_stMouseCursors)
    {
        if (p)
            ::SDL_FreeCursor(p);
    }

    io.BackendPlatformUserData = nullptr;
    io.BackendPlatformName = nullptr;
    ImPlot::DestroyContext(m_pImPlotContext);
    ImGui::DestroyContext(m_pImGuiContext);
}

Result<bool> DebugGUISystem::AppendWindow(std::shared_ptr<DebugGUI::Window> window) noexcept
{
    assert(window);
    if (m_stWindows.find(window->GetName()) != m_stWindows.end())
        return false;
    const auto& name = window->GetName();
    try
    {
        m_stWindows.emplace(name, std::move(window));
    }
    catch (...)
    {
        return make_error_code(errc::not_enough_memory);
    }
    return true;
}

std::shared_ptr<DebugGUI::Window> DebugGUISystem::FindWindow(std::string_view name) noexcept
{
    auto it = m_stWindows.find(name);
    if (it == m_stWindows.end())
        return {};
    return it->second;
}

void DebugGUISystem::OnUpdate(double elapsedTime) noexcept
{
    ImGuiIO& io = ImGui::GetIO();

    // 更新间隔时间
    io.DeltaTime = static_cast<float>(elapsedTime);

    // 每帧刷新下显示区域（是否有必要？）
    AdjustViewSize(io);

    // 检查是否具备焦点
    if (m_pWindowSystem->HasFocus())
    {
        if (io.WantSetMousePos)
            ::SDL_WarpMouseInWindow(m_pWindowSystem->GetNativeHandle(), static_cast<int>(io.MousePos.x), static_cast<int>(io.MousePos.y));
    }

    // 更新鼠标指针
    if ((io.ConfigFlags & ImGuiConfigFlags_NoMouseCursorChange) != ImGuiConfigFlags_NoMouseCursorChange)
    {
        ImGuiMouseCursor imGuiCursor = ImGui::GetMouseCursor();
        if (m_iLastMouseCursor != imGuiCursor)
        {
            assert(imGuiCursor < static_cast<int>(std::extent_v<decltype(m_stMouseCursors)>));
            m_iLastMouseCursor = imGuiCursor;

            if (io.MouseDrawCursor || imGuiCursor == ImGuiMouseCursor_None)
            {
                // IMGUI 自绘鼠标或者没有指针时告知系统隐藏鼠标指针
                ::SDL_ShowCursor(SDL_FALSE);
            }
            else
            {
                auto cursor = m_stMouseCursors[imGuiCursor] ? m_stMouseCursors[imGuiCursor] : m_stMouseCursors[ImGuiMouseCursor_Arrow];
                if (cursor)
                    ::SDL_SetCursor(cursor);
                ::SDL_ShowCursor(SDL_TRUE);
            }
        }
    }

    // 更新子窗口
    for (const auto& w : m_stWindows)
        w.second->Update(elapsedTime);
}

void DebugGUISystem::OnAfterRender(double elapsedTime) noexcept
{
    ImGui::NewFrame();

    // 绘制所有窗口
    for (const auto& w : m_stWindows)
        w.second->Render();

    ImGui::Render();
    m_pRenderer->RenderDrawData(ImGui::GetDrawData());
}

void DebugGUISystem::OnEvent(SubsystemEvent& event) noexcept
{
    const auto& underlay = event.GetEvent();
    if (underlay.index() != 0)
        return;

    ImGuiIO& io = ImGui::GetIO();
    const SDL_Event* platformEvent = std::get<0>(underlay);
    int t = 0;

    // 发送事件给 ImGui
    switch (platformEvent->type)
    {
        case SDL_MOUSEMOTION:
            io.AddMousePosEvent(static_cast<float>(platformEvent->motion.x), static_cast<float>(platformEvent->motion.y));
            break;
        case SDL_MOUSEWHEEL:
            io.AddMouseWheelEvent(
                (platformEvent->wheel.x > 0) ? 1.f : ((platformEvent->wheel.x < 0) ? -1.f : 0.f),
                (platformEvent->wheel.y > 0) ? 1.f : ((platformEvent->wheel.y < 0) ? -1.f : 0.f));
            break;
        case SDL_MOUSEBUTTONDOWN:
            switch (platformEvent->button.button)
            {
                case SDL_BUTTON_LEFT:
                    t = 0;
                    m_iMouseButtonState |= detail::MouseButtonState::LeftDown;
                    break;
                case SDL_BUTTON_MIDDLE:
                    t = 2;
                    m_iMouseButtonState |= detail::MouseButtonState::MiddleDown;
                    break;
                case SDL_BUTTON_RIGHT:
                    t = 1;
                    m_iMouseButtonState |= detail::MouseButtonState::RightDown;
                    break;
            }
            io.AddMouseButtonEvent(t, true);
            break;
        case SDL_MOUSEBUTTONUP:
            switch (platformEvent->button.button)
            {
                case SDL_BUTTON_LEFT:
                    t = 0;
                    m_iMouseButtonState ^= detail::MouseButtonState::LeftDown;
                    break;
                case SDL_BUTTON_MIDDLE:
                    t = 2;
                    m_iMouseButtonState ^= detail::MouseButtonState::MiddleDown;
                    break;
                case SDL_BUTTON_RIGHT:
                    t = 1;
                    m_iMouseButtonState ^= detail::MouseButtonState::RightDown;
                    break;
            }
            io.AddMouseButtonEvent(t, false);
            break;
        case SDL_TEXTINPUT:
            io.AddInputCharactersUTF8(platformEvent->text.text);
            break;
        case SDL_KEYDOWN:
        case SDL_KEYUP:
            UpdateKeyModifiers(io, static_cast<SDL_Keymod>(platformEvent->key.keysym.mod));
            t = static_cast<int>(SDLKeyCodeToImGui(platformEvent->key.keysym.sym));
            io.AddKeyEvent(t, (platformEvent->type == SDL_KEYDOWN));
            io.SetKeyEventNativeData(t, platformEvent->key.keysym.sym, platformEvent->key.keysym.scancode,
                platformEvent->key.keysym.scancode);
            break;
        case SDL_WINDOWEVENT:
            if (platformEvent->window.event == SDL_WINDOWEVENT_LEAVE)
                io.AddMousePosEvent(-FLT_MAX, -FLT_MAX);
            if (platformEvent->window.event == SDL_WINDOWEVENT_FOCUS_GAINED)
                io.AddFocusEvent(true);
            else if (platformEvent->window.event == SDL_WINDOWEVENT_FOCUS_LOST)
                io.AddFocusEvent(false);
            break;
    }

    // 检查 ImGui 是否需要独占这些事件
    if (io.WantCaptureMouse)
    {
        switch (platformEvent->type)
        {
            case SDL_MOUSEMOTION:
            case SDL_MOUSEWHEEL:
            case SDL_MOUSEBUTTONDOWN:
            case SDL_MOUSEBUTTONUP:
                event.StopPropagation();
                break;
            default:
                break;
        }
    }
    if (io.WantCaptureKeyboard)
    {
        switch (platformEvent->type)
        {
            case SDL_TEXTINPUT:
            case SDL_KEYDOWN:
            case SDL_KEYUP:
                event.StopPropagation();
                break;
            default:
                break;
        }
    }

    // 如果鼠标按下，则告知 OS 不要激活其他窗口
    if (platformEvent->type == SDL_MOUSEBUTTONDOWN || platformEvent->type == SDL_MOUSEBUTTONUP)
        SDL_CaptureMouse(m_iMouseButtonState != detail::MouseButtonState::None ? SDL_TRUE : SDL_FALSE);

#ifdef LSTG_DEVELOPMENT
    // 弹出 ConsoleWindow
    if (platformEvent->type == SDL_KEYDOWN && platformEvent->key.keysym.sym == SDLK_BACKQUOTE)
    {
        if (m_pConsoleWindow->IsVisible())
            m_pConsoleWindow->Hide();
        else
            m_pConsoleWindow->Show();
    }
#endif
}

void DebugGUISystem::AdjustViewSize(ImGuiIO& io) noexcept
{
    auto deviceBridge = m_pRenderSystem->GetRenderDevice();
    auto windowSize = m_pWindowSystem->GetSize();
    auto renderSizeWidth = deviceBridge->GetRenderOutputWidth();
    auto renderSizeHeight = deviceBridge->GetRenderOutputHeight();
    if (m_pWindowSystem->IsMinimized())
        windowSize = {0, 0};

    io.DisplaySize = ImVec2(static_cast<float>(std::get<0>(windowSize)), static_cast<float>(std::get<1>(windowSize)));
    if (std::get<0>(windowSize) > 0 && std::get<1>(windowSize) > 0)
    {
        io.DisplayFramebufferScale = ImVec2(renderSizeWidth / static_cast<float>(std::get<0>(windowSize)),
            renderSizeHeight / static_cast<float>(std::get<1>(windowSize)));
    }
}

const char* DebugGUISystem::ImGuiGetClipboardText() noexcept
{
    auto data = ::SDL_GetClipboardText();
    try
    {
        // 拷贝到当前对象上存储
        m_stClipboardText = data;
    }
    catch (...)  // bad_alloc
    {
        LSTG_LOG_ERROR_CAT(DebugGUISystem, "Allocate clipboard data fail");
        m_stClipboardText.clear();
    }
    ::SDL_free(data);
    return m_stClipboardText.c_str();
}

void DebugGUISystem::ImGuiSetClipboardText(const char* text) noexcept
{
    if (0 != ::SDL_SetClipboardText(text))
        LSTG_LOG_ERROR_CAT(DebugGUISystem, "SDL_SetClipboardText fail: {}", ::SDL_GetError());
}
