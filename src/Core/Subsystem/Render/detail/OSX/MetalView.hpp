/**
 * @file
 * @date 2022/3/9
 * @author 9chu
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#pragma once

#ifdef __OBJC__
@class NSView;
@class NSWindow;
@class CAMetalLayer;
#else
typedef struct _NSView NSView;
typedef struct _NSWindow NSWindow;
typedef struct _CAMetalLayer CAMetalLayer;
#endif

namespace lstg::Subsystem::Render::detail::OSX
{
    /**
     * GL 视图
     */
    class MetalView
    {
    public:
        MetalView(/* NSWindow* */ void* nsWindow, bool highDpi);
        ~MetalView();

    public:
        [[nodiscard]] NSView* GetView() const noexcept { return m_pMetalView; }

    private:
        NSView* m_pMetalView = nullptr;  // 仅持有指针
        CAMetalLayer* m_pMetalLayer = nullptr;
    };
}
