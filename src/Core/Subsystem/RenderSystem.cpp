/**
 * @file
 * @date 2022/3/6
 * @author 9chu
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#include <lstg/Core/Subsystem/RenderSystem.hpp>

#include <vector>
#include <lstg/Core/Pal.hpp>
#include <lstg/Core/Logging.hpp>
#include <lstg/Core/Subsystem/SubsystemContainer.hpp>
#include "Render/detail/RenderDeviceGL.hpp"
#include "Render/detail/RenderDeviceVulkan.hpp"

using namespace std;
using namespace lstg;
using namespace lstg::Subsystem;

LSTG_DEF_LOG_CATEGORY(RenderSystem);

namespace
{
    /**
     * Diligent 调试日志转发
     */
    void DiligentDebugOutput(Diligent::DEBUG_MESSAGE_SEVERITY severity, const char* message, const char* function, const char* file,
        int line) noexcept
    {
        LogLevel level;
        switch (severity)
        {
            case Diligent::DEBUG_MESSAGE_SEVERITY_INFO:
                level = LogLevel::Info;
                break;
            case Diligent::DEBUG_MESSAGE_SEVERITY_WARNING:
                level = LogLevel::Warn;
                break;
            case Diligent::DEBUG_MESSAGE_SEVERITY_ERROR:
                level = LogLevel::Error;
                break;
            case Diligent::DEBUG_MESSAGE_SEVERITY_FATAL_ERROR:
                level = LogLevel::Critical;
                break;
            default:
                level = LogLevel::Debug;
                break;
        }

        Logging::GetInstance().Log(kLogCategoryRenderSystem.Name, level, lstg::detail::GetLogCurrentTime(), {file, function, line}, "{}",
            message);
    }

    using RenderDeviceConstructor = std::function<Render::RenderDevicePtr(WindowSystem*)>;

    /**
     * 获取支持的渲染设备
     * @param out 输出设备列表
     */
    void GetSupportedRenderDevice(std::vector<std::tuple<const char*, RenderDeviceConstructor>>& out)
    {
#if VULKAN_SUPPORTED
        out.emplace_back("Vulkan", [](WindowSystem* windowSystem) -> Render::RenderDevicePtr {
            return make_shared<Render::detail::RenderDeviceVulkan>(windowSystem);
        });
#endif
#if GL_SUPPORTED
        out.emplace_back("OpenGL", [](WindowSystem* windowSystem) -> Render::RenderDevicePtr {
            return make_shared<Render::detail::RenderDeviceGL>(windowSystem);
        });
#endif
    }
}

RenderSystem::RenderSystem(SubsystemContainer& container)
    : m_pWindowSystem(container.Get<WindowSystem>())
{
    assert(m_pWindowSystem);

    // 设置 Debug Output
    Diligent::SetDebugMessageCallback(DiligentDebugOutput);

    // 初始化渲染设备
    vector<tuple<const char*, RenderDeviceConstructor>> supportedDevices;
    GetSupportedRenderDevice(supportedDevices);
    if (!supportedDevices.empty())
    {
        for (const auto& builder : supportedDevices)
        {
            try
            {
                LSTG_LOG_INFO_CAT(RenderSystem, "Try initializing render backend {}", std::get<0>(builder));
                m_pRenderDevice = std::get<1>(builder)(m_pWindowSystem.get());
                break;
            }
            catch (const std::exception& ex)
            {
                LSTG_LOG_ERROR_CAT(RenderSystem, "Initializing render backend {} fail: {}", std::get<0>(builder), ex.what());
            }
        }
    }
    if (!m_pRenderDevice)
        LSTG_THROW(Render::RenderDeviceInitializeFailedException, "No available render device");
}
