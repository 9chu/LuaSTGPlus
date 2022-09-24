/**
 * @file
 * @date 2022/3/9
 * @author 9chu
 * 此文件为 LuaSTGPlus 项目的一部分，版权与许可声明详见 COPYRIGHT.txt。
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

namespace lstg::Subsystem::Render::detail::RenderDevice::OSX
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
        [[nodiscard]] CAMetalLayer* GetLayer() const noexcept { return m_pMetalLayer; }

    private:
        NSView* m_pMetalView = nullptr;  // 仅持有指针
        CAMetalLayer* m_pMetalLayer = nullptr;
    };
}
