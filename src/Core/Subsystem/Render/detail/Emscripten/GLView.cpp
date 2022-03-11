/**
 * @file
 * @date 2022/3/10
 * @author 9chu
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#include "GLView.hpp"

#include <lstg/Core/Logging.hpp>
#include <lstg/Core/Subsystem/Render/RenderDevice.hpp>  // for RenderDeviceInitializeFailedException

#ifdef LSTG_PLATFORM_EMSCRIPTEN
#include <emscripten.h>
#include <emscripten/html5_webgl.h>
#endif

using namespace std;
using namespace lstg;
using namespace lstg::Subsystem::Render::detail::Emscripten;

#ifdef LSTG_PLATFORM_EMSCRIPTEN

LSTG_DEF_LOG_CATEGORY(EmscriptenGLView);

GLView::GLView(const char* canvasId)
    : m_pCanvasId(canvasId)
{
    EmscriptenWebGLContextAttributes attrs;
    ::emscripten_webgl_init_context_attributes(&attrs);
    attrs.alpha = false;
    attrs.premultipliedAlpha = false;
    attrs.depth = true;
    attrs.stencil = true;
    attrs.enableExtensionsByDefault = true;
    attrs.antialias = false;
    attrs.minorVersion = 0;
    // attrs.explicitSwapControl = true;  // 需要 OFFSCREEN_FRAMEBUFFER 支持

    for (int version = 3; version >= 1; --version)
    {
        attrs.majorVersion = version;
        LSTG_LOG_INFO_CAT(EmscriptenGLView, "Try create GLContext with version {}.{}", attrs.majorVersion, attrs.minorVersion);
        m_hContextHandle = ::emscripten_webgl_create_context(m_pCanvasId, &attrs);
        if (m_hContextHandle > 0)
        {
            int ret = 0;
            if (EMSCRIPTEN_RESULT_SUCCESS != (ret = ::emscripten_webgl_make_context_current(m_hContextHandle)))
                LSTG_LOG_ERROR_CAT(EmscriptenGLView, "make_context_current fail: {}", ret);
            break;
        }
    }

    if (m_hContextHandle <= 0)
        LSTG_THROW(RenderDeviceInitializeFailedException, "Fail to create WebGL, last error: {}", m_hContextHandle);
}

GLView::~GLView()
{
    if (m_hContextHandle > 0)
        ::emscripten_webgl_destroy_context(m_hContextHandle);
}

void GLView::Present() noexcept
{
    // int ret = 0;
    // if (EMSCRIPTEN_RESULT_SUCCESS != (ret = ::emscripten_webgl_commit_frame()))
    //     LSTG_LOG_ERROR_CAT(EmscriptenGLView, "commit_frame fail: {}", ret);
}

#endif
