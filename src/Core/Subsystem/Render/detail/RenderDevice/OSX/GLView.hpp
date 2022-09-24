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
@class NSOpenGLView;
@class NSOpenGLContext;
#else
typedef struct _NSView NSView;
typedef struct _NSWindow NSWindow;
typedef struct _NSOpenGLView NSOpenGLView;
typedef struct _NSOpenGLContext NSOpenGLContext;
#endif

namespace lstg::Subsystem::Render::detail::RenderDevice::OSX
{
    /**
     * GL 视图
     */
    class GLView
    {
    public:
        GLView(/* NSWindow* */ void* nsWindow, bool highDpi);
        ~GLView();

    public:
        [[nodiscard]] NSOpenGLView* GetView() const noexcept { return m_pGLView; }
        void Present() const noexcept;

    private:
        NSOpenGLView* m_pGLView = nullptr;
        NSOpenGLContext* m_pGLContext = nullptr;
    };
}
