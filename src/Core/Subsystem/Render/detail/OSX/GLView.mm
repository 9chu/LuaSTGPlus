/**
 * @file
 * @date 2022/3/9
 * @author 9chu
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#include "GLView.hpp"

#include <Cocoa/Cocoa.h>
#include <lstg/Core/Subsystem/Render/RenderDevice.hpp>  // for RenderDeviceInitializeFailedException

using namespace std;
using namespace lstg;
using namespace lstg::Subsystem::Render::detail::OSX;

GLView::GLView(void* nsWindow, bool highDpi)
{
    assert([(NSObject*)nsWindow isKindOfClass:[NSWindow class]]);

    // --- 创建 GLView 和 GLContext
    auto window = (NSWindow*)nsWindow;
    auto contentView = [window contentView];
    assert(contentView);  // SDL总是会创建一个 ContentView

    // 初始化参数参考：DiligentTools/NativeApp/Apple/Source/Classes/OSX/GLView.mm
    NSOpenGLPixelFormatAttribute attrs[] = {
        NSOpenGLPFADoubleBuffer,
        NSOpenGLPFADepthSize, 24,
        NSOpenGLPFAOpenGLProfile,
        NSOpenGLProfileVersion4_1Core,
        0
    };
    NSOpenGLPixelFormat* pixelFormat = [[NSOpenGLPixelFormat alloc] initWithAttributes:attrs];
    if (!pixelFormat)
        LSTG_THROW(RenderDeviceInitializeFailedException, "NSOpenGLPixelFormat::initWithAttributes fail");

    NSRect glViewRect = [contentView bounds];
    NSOpenGLView* glView = [[NSOpenGLView alloc] initWithFrame:glViewRect pixelFormat:pixelFormat];
    if (!glView)
    {
        [pixelFormat release];
        LSTG_THROW(RenderDeviceInitializeFailedException, "NSOpenGLView::initWithFrame fail");
    }

    // 我们挂到 contentView 上，而不是对 contentView 进行操作
    [glView setAutoresizingMask:( NSViewHeightSizable | NSViewWidthSizable | NSViewMinXMargin | NSViewMaxXMargin | NSViewMinYMargin |
        NSViewMaxYMargin )];
    [contentView addSubview:glView];

    NSOpenGLContext* glContext = [[NSOpenGLContext alloc] initWithFormat:pixelFormat shareContext:nil];
    if (!glContext)
    {
        [glView removeFromSuperview];
        [glView release];
        [pixelFormat release];
        LSTG_THROW(RenderDeviceInitializeFailedException, "NSOpenGLContext::initWithFormat fail");
    }

    assert([NSThread isMainThread]);
    [glView setPixelFormat:pixelFormat];
    [glView setOpenGLContext:glContext];
    [glView setWantsBestResolutionOpenGLSurface:highDpi];
    [glContext setView:glView];

    [pixelFormat release];

    // --- 设置 CurrentContext
    [glContext makeCurrentContext];

    GLint swapInt = 0;
    [glContext setValues:&swapInt forParameter:NSOpenGLContextParameterSwapInterval];

    m_pGLView = glView;
    m_pGLContext = glContext;
}

GLView::~GLView()
{
    [m_pGLContext release];
    [m_pGLView release];
}
