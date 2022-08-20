/**
 * @file
 * @date 2022/8/20
 * @author 9chu
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#include "GLContext.hpp"

#if defined(LSTG_PLATFORM_LINUX) && defined(LSTG_X11_ENABLE)

#include <cassert>
#include <X11/Xlib.h>
#include <GL/glx.h>
#include <GL/gl.h>
#include <lstg/Core/Subsystem/Render/RenderDevice.hpp>  // for RenderDeviceInitializeFailedException

using namespace std;
using namespace lstg;
using namespace lstg::Subsystem::Render::detail::RenderDevice::Linux;

typedef GLXContext (*glXCreateContextAttribsARBProc)(Display*, GLXFBConfig, GLXContext, int, const int*);

GLContext::GLContext(void* display, uint32_t windowId)
    : m_pDisplay(display)
{
    auto xDisplay = static_cast<Display*>(display);

    static const int kVisualAttribs[] = {
        GLX_RENDER_TYPE, GLX_RGBA_BIT,
        GLX_DRAWABLE_TYPE, GLX_WINDOW_BIT,
        GLX_DOUBLEBUFFER, true,

        // largest channel size
        GLX_RED_SIZE, 8,
        GLX_GREEN_SIZE, 8,
        GLX_BLUE_SIZE, 8,
        GLX_ALPHA_SIZE, 8,
        GLX_DEPTH_SIZE, 24,

        GLX_SAMPLES,  1,
        None
    };

    int frameBufferCount = 0;
    GLXFBConfig* frameBufferConfig = ::glXChooseFBConfig(xDisplay, DefaultScreen(xDisplay), kVisualAttribs, &frameBufferCount);
    if (!frameBufferCount)
        LSTG_THROW(RenderDeviceInitializeFailedException, "glXChooseFBConfig fail to retrieve a framebuffer config");

    XVisualInfo* visualInfo = ::glXGetVisualFromFBConfig(xDisplay, frameBufferConfig[0]);

    glXCreateContextAttribsARBProc glXCreateContextAttribsARB = nullptr;
    {
        // Create an oldstyle context first, to get the correct function pointer for glXCreateContextAttribsARB
        GLXContext contextOldStyle = ::glXCreateContext(xDisplay, visualInfo, 0, GL_TRUE);
        glXCreateContextAttribsARB = reinterpret_cast<glXCreateContextAttribsARBProc>(
            ::glXGetProcAddress(reinterpret_cast<const GLubyte*>("glXCreateContextAttribsARB")));
        ::glXMakeCurrent(xDisplay, None, nullptr);
        ::glXDestroyContext(xDisplay, contextOldStyle);
    }

    if (glXCreateContextAttribsARB == nullptr)
    {
        ::XFree(visualInfo);
        ::XFree(frameBufferConfig);
        LSTG_THROW(RenderDeviceInitializeFailedException, "glXCreateContextAttribsARB entry point not found");
    }

    static const int kContextAttribs[] =
    {
        GLX_CONTEXT_MAJOR_VERSION_ARB, 4,
        GLX_CONTEXT_MINOR_VERSION_ARB, 3,
#ifdef _DEBUG
        GLX_CONTEXT_FLAGS_ARB, GLX_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB | GLX_CONTEXT_DEBUG_BIT_ARB,
#else
        GLX_CONTEXT_FLAGS_ARB, GLX_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB,
#endif
        None
    };

    GLXContext context = glXCreateContextAttribsARB(xDisplay, frameBufferConfig[0], nullptr, /* True */ 1, kContextAttribs);
    ::XFree(visualInfo);
    ::XFree(frameBufferConfig);
    if (!context)
        LSTG_THROW(RenderDeviceInitializeFailedException, "glXCreateContextAttribsARB failed to create context");

    ::glXMakeCurrent(xDisplay, static_cast<XID>(windowId), context);
}

GLContext::~GLContext()
{
    auto xDisplay = static_cast<Display*>(m_pDisplay);

    auto context = ::glXGetCurrentContext();
    assert(context);
    ::glXMakeCurrent(xDisplay, None, nullptr);
    ::glXDestroyContext(xDisplay, context);
}

#endif
