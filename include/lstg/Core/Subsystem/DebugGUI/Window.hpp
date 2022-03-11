/**
 * @file
 * @date 2022/3/11
 * @author 9chu
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#pragma once
#include <cstdint>
#include <string>
#include <lstg/Core/Flag.hpp>

namespace lstg::Subsystem::DebugGUI
{
    LSTG_FLAG_BEGIN(DebugWindowFlags)  // ImGuiWindowFlags_
        None = 0,
        NoTitleBar = 1 << 0,
        NoResize = 1 << 1,
        NoMove = 1 << 2,
        NoScrollbar = 1 << 3,
        NoScrollWithMouse = 1 << 4,
        NoCollapse = 1 << 5,
        AlwaysAutoResize = 1 << 6,
        NoBackground = 1 << 7,
        NoSavedSettings = 1 << 8,
        NoMouseInputs = 1 << 9,
        MenuBar = 1 << 10,
        HorizontalScrollbar = 1 << 11,
        NoFocusOnAppearing = 1 << 12,
        NoBringToFrontOnFocus = 1 << 13,
        AlwaysVerticalScrollbar = 1 << 14,
        AlwaysHorizontalScrollbar = 1<< 15,
        AlwaysUseWindowPadding = 1 << 16,
        NoNavInputs = 1 << 18,
        NoNavFocus = 1 << 19,
        UnsavedDocument = 1 << 20,
        NoNav = NoNavInputs | NoNavFocus,
        NoDecoration = NoTitleBar | NoResize | NoScrollbar | NoCollapse,
        NoInputs = NoMouseInputs | NoNavInputs | NoNavFocus,
    LSTG_FLAG_END(DebugWindowFlags)

    /**
     * 调试 GUI 窗口基类
     */
    class Window
    {
    public:
        Window(const char* name, const char* title, DebugWindowFlags flags);
        virtual ~Window() = default;

    public:
        /**
         * 是否可见
         */
        bool IsVisible() const noexcept { return m_bVisible; }

        /**
         * 隐藏
         */
        void Hide() noexcept { m_bVisible = false; }

        /**
         * 显示
         */
        void Show() noexcept { m_bVisible = true; }

        /**
         * 获取标志位
         */
        DebugWindowFlags GetFlags() const noexcept { return m_uFlags; }

        /**
         * 设置标志位
         * @param flags 标志
         */
        void SetFlags(DebugWindowFlags flags) noexcept { m_uFlags = flags; }

        /**
         * 获取名称
         */
        const std::string& GetName() const noexcept { return m_stName; }

        /**
         * 获取标题
         */
        const std::string& GetTitle() const noexcept { return m_stTitle; }

        /**
         * 设置标题
         */
        void SetTitle(std::string title) noexcept { m_stTitle = std::move(title); }

        /**
         * 更新状态
         * @param elapsedTime 流逝时间
         */
        void Update(double elapsedTime) noexcept;

        /**
         * 绘制
         */
        void Render() noexcept;

    protected:  // 需要实现
        virtual void OnPrepareWindow() noexcept;
        virtual void OnUpdate(double elapsedTime) noexcept;
        virtual void OnRender() noexcept;

    private:
        std::string m_stName;
        std::string m_stTitle;
        bool m_bVisible = false;  // 默认不可见
        DebugWindowFlags m_uFlags;
    };
}
