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
@class NSOpenGLView;
@class NSOpenGLContext;
#else
typedef struct _NSView NSView;
typedef struct _NSWindow NSWindow;
typedef struct _NSOpenGLView NSOpenGLView;
typedef struct _NSOpenGLContext NSOpenGLContext;
#endif

namespace lstg::Subsystem::Render::detail::OSX
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

    private:
        NSOpenGLView* m_pGLView = nullptr;
        NSOpenGLContext* m_pGLContext = nullptr;
    };
}
