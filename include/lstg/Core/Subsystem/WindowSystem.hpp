/**
 * @file
 * @author 9chu
 * @date 2022/2/15
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#pragma once
#include <tuple>
#include <string_view>
#include "../Flag.hpp"
#include "../Result.hpp"
#include "../Exception.hpp"
#include "ISubsystem.hpp"
#include "EventBusSystem.hpp"

struct SDL_Window;

namespace lstg::Subsystem
{
    LSTG_DEFINE_EXCEPTION(WindowInitializeFailedException);

    /**
     * 窗口特性
     *
     * 跨平台窗口系统具有如下特点：
     *  - 部分平台仅有全屏模式，无法切换到视窗模式
     *  - 即便是全屏模式也可能存在 Resize 消息（我们不确定 SDL 是否支持诸如安卓下的分辨率切换，但是假定存在这种情况）
     *  - 部分平台允许用户调整渲染窗口大小，而不允许代码进行调整（HTML5 平台我们不希望在 C++ 侧调整 Canvas，而应该自适应 Canvas 调整）
     */
    LSTG_FLAG_BEGIN(WindowFeatures)
        /**
         * 指示是否支持窗体模式
         */
        SupportWindowMode = 1,

        /**
         * 指示是否支持代码侧的大小调整
         */
        ProgrammingResizable = 2,

        /**
         * 指示是否启用 HighDPI
         */
        HighDPISupport = 4,
    LSTG_FLAG_END(WindowFeatures)

    /**
     * 窗口系统
     */
    class WindowSystem :
        public ISubsystem
    {
    public:
        WindowSystem(SubsystemContainer& container);
        WindowSystem(const WindowSystem&) = delete;
        WindowSystem(WindowSystem&&)noexcept = delete;
        ~WindowSystem() override;

    public:
        /**
         * 获取系统底层的窗体句柄
         */
        [[nodiscard]] SDL_Window* GetNativeHandle() const noexcept { return m_pWindow; }

        /**
         * 获取窗体特性
         * @see WindowFeatures
         * @return 窗体特性标志位
         */
        [[nodiscard]] WindowFeatures GetFeatures() const noexcept { return m_iFeatures; }

        /**
         * 获取窗口标题
         */
        const char* GetTitle() const noexcept;

        /**
         * 设置窗体标题
         * @param title 标题
         */
        void SetTitle(const char* title) noexcept;

        /**
         * 获取窗体大小
         */
        [[nodiscard]] std::tuple<int, int> GetSize() const noexcept;

        /**
         * 设置窗体大小
         * @param width 宽度
         * @param height 高度
         */
        void SetSize(int width, int height) noexcept;

        /**
         * 获取渲染大小
         */
        [[nodiscard]] std::tuple<int, int> GetRenderSize() const noexcept;

        /**
         * 窗口移到最顶上
         */
        void Raise() noexcept;

        /**
         * 显示窗口
         */
        void Show() noexcept;

        /**
         * 隐藏窗口
         */
        void Hide() noexcept;

        /**
         * 是否全屏
         */
        bool IsFullScreen() const noexcept;

        /**
         * 是否最小化
         */
        bool IsMinimized() const noexcept;

        /**
         * 检查是否具备焦点
         */
        bool HasFocus() const noexcept;

        /**
         * 切换全屏/窗口模式
         * @param fullscreen 是否全屏
         * @return 是否成功
         */
        Result<void> ToggleFullScreen(bool fullscreen) noexcept;

        /**
         * 鼠标是否可见
         */
        Result<bool> IsMouseCursorVisible() noexcept;

        /**
         * 设置鼠标光标是否可见
         * @param shown 可见
         */
        Result<void> SetMouseCursorVisible(bool shown) noexcept;

    private:
        EventBusSystemPtr m_pEventBusSystem;
        SDL_Window* m_pWindow = nullptr;
        WindowFeatures m_iFeatures = static_cast<WindowFeatures>(0);
    };

    using WindowSystemPtr = std::shared_ptr<WindowSystem>;
} // namespace lstg::Subsystem
