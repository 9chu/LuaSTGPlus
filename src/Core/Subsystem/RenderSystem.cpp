/**
 * @file
 * @date 2022/3/6
 * @author 9chu
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#include <lstg/Core/Subsystem/RenderSystem.hpp>

#include <SDL_syswm.h>
#include <lstg/Core/Pal.hpp>
#include <lstg/Core/Logging.hpp>
#include <lstg/Core/Subsystem/SubsystemContainer.hpp>
#include <lstg/Core/Subsystem/Render/RenderError.hpp>
#include "../detail/SDLHelper.hpp"
#include "detail/BgfxHelper.hpp"

using namespace std;
using namespace lstg;
using namespace lstg::Subsystem;

using RenderError = Render::RenderError;

LSTG_DEF_LOG_CATEGORY(RenderSystem);

namespace lstg::Subsystem::detail
{
    class BgfxCallback :
        public bgfx::CallbackI
    {
    public:  // CallbackI
        void fatal(const char* filePath, uint16_t line, bgfx::Fatal::Enum code, const char* str) override
        {
            LSTG_LOG_CRITICAL_CAT(RenderSystem, "BGFX FATAL ERROR: {} @ ({}:{})", str, filePath, line);
            Pal::FatalError(str);
        }

        void traceVargs(const char* filePath, uint16_t line, const char* format, va_list argList) override
        {
            char buffer[128];
            memset(buffer, 0, sizeof(buffer));
            vsnprintf(buffer, sizeof(buffer), format, argList);

            // 去掉末尾的换行符
            auto sz = strlen(buffer);
            while (sz)
            {
                if (buffer[sz - 1] == '\r' || buffer[sz - 1] == '\n')
                    buffer[(sz--) - 1] = '\0';
                else
                    break;
            }

            lstg::Logging::GetInstance().Log(kLogCategoryRenderSystem.Name, LogLevel::Trace, lstg::detail::GetLogCurrentTime(),
                {filePath, "", line}, "{}", buffer);
        }

        void profilerBegin(const char* name, uint32_t abgr, const char* filePath, uint16_t line) override
        {
            static_cast<void>(name);
            static_cast<void>(abgr);
            static_cast<void>(filePath);
            static_cast<void>(line);
            LSTG_LOG_WARN_CAT(RenderSystem, "profilerBegin is not implemented");
        }

        void profilerBeginLiteral(const char* name, uint32_t abgr, const char* filePath, uint16_t line) override
        {
            static_cast<void>(name);
            static_cast<void>(abgr);
            static_cast<void>(filePath);
            static_cast<void>(line);
            LSTG_LOG_WARN_CAT(RenderSystem, "profilerBeginLiteral is not implemented");
        }

        void profilerEnd() override
        {
            LSTG_LOG_WARN_CAT(RenderSystem, "profilerEnd is not implemented");
        }

        uint32_t cacheReadSize(uint64_t id) override
        {
            static_cast<void>(id);
            LSTG_LOG_WARN_CAT(RenderSystem, "cacheReadSize is not implemented");
            return 0;
        }

        bool cacheRead(uint64_t id, void* data, uint32_t size) override
        {
            static_cast<void>(id);
            static_cast<void>(data);
            LSTG_LOG_WARN_CAT(RenderSystem, "cacheRead is not implemented");
            return false;
        }

        void cacheWrite(uint64_t id, const void* data, uint32_t size) override
        {
            static_cast<void>(id);
            static_cast<void>(data);
            static_cast<void>(size);
            LSTG_LOG_WARN_CAT(RenderSystem, "cacheWrite is not implemented");
        }

        void screenShot(const char* filePath, uint32_t width, uint32_t height, uint32_t pitch, const void* data, uint32_t size,
            bool yflip) override
        {
            static_cast<void>(filePath);
            static_cast<void>(width);
            static_cast<void>(height);
            static_cast<void>(pitch);
            static_cast<void>(data);
            static_cast<void>(size);
            static_cast<void>(yflip);
            LSTG_LOG_WARN_CAT(RenderSystem, "screenShot is not implemented");
        }

        void captureBegin(uint32_t width, uint32_t height, uint32_t pitch, bgfx::TextureFormat::Enum format, bool yflip) override
        {
            static_cast<void>(width);
            static_cast<void>(height);
            static_cast<void>(pitch);
            static_cast<void>(format);
            static_cast<void>(yflip);
            LSTG_LOG_WARN_CAT(RenderSystem, "captureBegin is not implemented");
        }

        void captureEnd() override
        {
            LSTG_LOG_WARN_CAT(RenderSystem, "captureEnd is not implemented");
        }

        void captureFrame(const void* data, uint32_t size) override
        {
            LSTG_LOG_WARN_CAT(RenderSystem, "captureFrame is not implemented");
        }
    };
}

RenderSystem::RenderSystem(SubsystemContainer& container)
    : m_pWindowSystem(container.Get<WindowSystem>())
{
    assert(m_pWindowSystem);

#if BX_PLATFORM_OSX
    auto windowHandle = m_pWindowSystem->GetNativeHandle();
    assert(windowHandle);

    // MacOS 需要创建一个 Metal View 贴到 SDL 窗口上
    // see: https://github.com/bkaradzic/bgfx/issues/1773
    m_pMetalView = ::SDL_Metal_CreateView(windowHandle);
    if (!m_pMetalView)
        LSTG_THROW(RendererInitializeFailedException, "SDL_Metal_CreateView returns null");
    m_pMetalViewLayer = ::SDL_Metal_GetLayer(m_pMetalView);
    if (!m_pMetalViewLayer)
        LSTG_THROW(RendererInitializeFailedException, "SDL_Metal_GetLayer returns null");
#endif

    // 初始化 BGFX 回调
    m_pBgfxCallback = make_shared<detail::BgfxCallback>();

    // 初始化 BGFX
    Initialize();
}

RenderSystem::~RenderSystem()
{
    LSTG_LOG_TRACE_CAT(RenderSystem, "Calling bgfx::shutdown");
    bgfx::shutdown();
}

void RenderSystem::Initialize()
{
    auto windowHandle = m_pWindowSystem->GetNativeHandle();
    auto windowSize = m_pWindowSystem->GetSize();

    // 填充平台相关数据
    bgfx::PlatformData bgfxPlatformData;
#ifndef __EMSCRIPTEN__
    SDL_SysWMinfo windowInfo;
    SDL_VERSION(&windowInfo.version);
    if (!::SDL_GetWindowWMInfo(windowHandle, &windowInfo))
    {
        LSTG_LOG_ERROR_CAT(RenderSystem, "SDL_GetWindowWMInfo fail: {}", ::SDL_GetError());
        LSTG_THROW(RendererInitializeFailedException, "SDL_GetWindowWMInfo fail: {}", ::SDL_GetError());
    }
#if BX_PLATFORM_WINDOWS
    bgfxPlatformData.nwh = windowInfo.info.win.window;
#elif BX_PLATFORM_OSX
    bgfxPlatformData.nwh = m_pMetalViewLayer;
    // bgfxPlatformData.nwh = windowInfo.info.cocoa.window;
#elif BX_PLATFORM_LINUX
    bgfxPlatformData.ndt = windowInfo.info.x11.display;
    bgfxPlatformData.nwh = reinterpret_cast<void*>(static_cast<uintptr_t>(windowInfo.info.x11.window));
#else
#error "Unsupported platform"
#endif
#else
    static const char* kCanvasId = "#canvas";
    bgfxPlatformData.nwh = reinterpret_cast<void*>(kCanvasId);
#endif

    // 填充初始化参数
    bgfx::Init bgfxInit;
#ifdef __ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__
    bgfxInit.type = bgfx::RendererType::Metal;
#else
    bgfxInit.type = bgfx::RendererType::Count;
#endif
#ifndef NDEBUG
    bgfxInit.debug = true;
#endif
    bgfxInit.platformData = bgfxPlatformData;
    bgfxInit.resolution.width = std::get<0>(windowSize);
    bgfxInit.resolution.height = std::get<1>(windowSize);
    bgfxInit.resolution.reset = BGFX_RESET_NONE;
    bgfxInit.resolution.numBackBuffers = 1;
    bgfxInit.callback = m_pBgfxCallback.get();

    LSTG_LOG_TRACE_CAT(RenderSystem, "Prepare to call bgfx::init, resolution {}x{}", bgfxInit.resolution.width,
        bgfxInit.resolution.height);
    if (!bgfx::init(bgfxInit))
    {
        LSTG_LOG_ERROR_CAT(RenderSystem, "Fail to initialize bgfx");
        LSTG_THROW(RendererInitializeFailedException, "Fail to initialize bgfx");
    }

    LSTG_LOG_INFO_CAT(RenderSystem, "Render backend: {}", detail::ToString(bgfx::getRendererType()));
}
