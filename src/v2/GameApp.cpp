/**
 * @file
 * @author 9chu
 * @date 2022/2/16
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#include <lstg/v2/GameApp.hpp>

#include <SDL.h>
#include <glm/ext.hpp>

#include <lstg/Core/Logging.hpp>
#include <lstg/Core/Subsystem/AssetSystem.hpp>
#include <lstg/Core/Subsystem/VirtualFileSystem.hpp>
#include <lstg/Core/Subsystem/ScriptSystem.hpp>
#include <lstg/Core/Subsystem/WindowSystem.hpp>
#include <lstg/Core/Subsystem/VFS/LocalFileSystem.hpp>
#include <lstg/Core/Subsystem/VFS/ZipArchiveFileSystem.hpp>
#include <lstg/Core/Subsystem/VFS/WebFileSystem.hpp>
#include <lstg/Core/Subsystem/Script/LuaStack.hpp>

// 资源类型
#include <lstg/v2/Asset/TextureAssetFactory.hpp>
#include <lstg/v2/Asset/SpriteAssetFactory.hpp>
#include <lstg/v2/Asset/SpriteSequenceAssetFactory.hpp>
#include <lstg/v2/Asset/TrueTypeFontAssetFactory.hpp>
#include <lstg/v2/Asset/HgeFontAssetFactory.hpp>
#include <lstg/v2/Asset/HgeParticleAssetFactory.hpp>

#include <lstg/v2/Bridge/BuiltInModules.hpp>

using namespace std;
using namespace lstg;
using namespace lstg::v2;

LSTG_DEF_LOG_CATEGORY(GameApp);

static const char* kScriptEntryFile = "launch";
static const char* kScriptCoreFile = "core.lua";

static const char* kEventOnGameInit = "GameInit";
static const char* kEventOnGainFocus = "FocusGainFunc";
static const char* kEventOnLostFocus = "FocusLoseFunc";
static const char* kEventOnUpdate = "FrameFunc";
static const char* kEventOnRender = "RenderFunc";

extern "C" int luaopen_cjson(lua_State* L);

GameApp::GameApp(int argc, char** argv)
    : m_stDesiredSolution(640, 480), m_stCommandExecutor(*GetSubsystem<Subsystem::RenderSystem>())
{
    // 初始化文件系统
    //  - assets
    //    - ...
    //    - core.lua
    //    - launch
    // 为了保持与老 API 的兼容性，默认资源寻找均从 assets 开始。
    // 可以通过 ../ 来访问其他路径。
    {
        // assets 目录通过 OverlayFileSystem 实现
        // 最下面是 LocalFileSystem，这使得在其他 FileSystem 上搜索不到时会到本地文件系统进行搜寻
        m_pAssetsFileSystem = make_shared<Subsystem::VFS::OverlayFileSystem>();

#ifdef LSTG_PLATFORM_EMSCRIPTEN
        // WEB 下创建 WebFileSystem
        // 在 LocalFileSystem (memfs) 中找不到才去 WebFileSystem 搜索
        auto webFileSystem = make_shared<Subsystem::VFS::WebFileSystem>("");
        m_pAssetsFileSystem->PushFileSystem(std::move(webFileSystem));
#endif

        // 准备 LocalFileSystem
#ifdef LSTG_SHIPPING
        auto base = filesystem::path(argv[0]);
        auto localFileSystem = make_shared<Subsystem::VFS::LocalFileSystem>(base.parent_path());  // 发布环境下总是与应用程序一个目录
#else
        auto localFileSystem = make_shared<Subsystem::VFS::LocalFileSystem>(".");  // DEV 模式下任意
#endif
        m_pAssetsFileSystem->PushFileSystem(std::move(localFileSystem));

        // 挂载
        auto ret = GetSubsystem<Subsystem::VirtualFileSystem>()->Mount("assets", m_pAssetsFileSystem);
        if (!ret)
            LSTG_THROW(AppInitializeFailedException, "Fail to mount \"assets\" virtual directory: {}", ret.GetError());

        // 设置资源系统基准目录
        GetSubsystem<Subsystem::VirtualFileSystem>()->SetAssetBaseDirectory("assets");
    }

    // 初始化资源系统
    {
        // 注册内置资源类型
        auto assetSystem = GetSubsystem<Subsystem::AssetSystem>();
        assetSystem->RegisterAssetFactory(make_shared<Asset::TextureAssetFactory>());
        assetSystem->RegisterAssetFactory(make_shared<Asset::SpriteAssetFactory>());
        assetSystem->RegisterAssetFactory(make_shared<Asset::SpriteSequenceAssetFactory>());
        assetSystem->RegisterAssetFactory(make_shared<Asset::TrueTypeFontAssetFactory>());
        assetSystem->RegisterAssetFactory(make_shared<Asset::HgeFontAssetFactory>());
        assetSystem->RegisterAssetFactory(make_shared<Asset::HgeParticleAssetFactory>());

        // TODO: Music Sound FX

        // 创建资源池
        m_pAssetPools = make_unique<AssetPools>();

        // 绑定资产解析器
        assetSystem->SetDependencyResolver(m_pAssetPools.get());
    }

    // 初始化渲染参数
    {
        auto renderDevice = GetSubsystem<Subsystem::RenderSystem>()->GetRenderDevice();
        assert(renderDevice);
        m_stNativeSolution = { renderDevice->GetRenderOutputWidth(), renderDevice->GetRenderOutputHeight() };
        AdjustViewport();

        // 刷初始渲染状态
        m_stCommandBuffer.Begin();
        m_stCommandBuffer.SetView(glm::identity<glm::mat4x4>());
        m_stCommandBuffer.SetProjection(glm::ortho<float>(0.f, static_cast<float>(m_stViewportBound.Width()),
                                                          0.f, static_cast<float>(m_stViewportBound.Height()),
                                                          0.f, 100.0f));
        m_stCommandBuffer.SetViewport(static_cast<float>(m_stViewportBound.Left()), static_cast<float>(m_stViewportBound.Top()),
                                      static_cast<float>(m_stViewportBound.Width()), static_cast<float>(m_stViewportBound.Height()));
        m_stCommandBuffer.End();
    }

    // 初始化函数库
    {
        auto& state = GetSubsystem<Subsystem::ScriptSystem>()->GetState();
        Subsystem::Script::LuaStack::BalanceChecker stackChecker(state);

        lua_gc(state, LUA_GCSTOP, 0);  // 初始化时关闭GC

        // 注册 cjson
        luaopen_cjson(state);
        lua_pop(state, 1);

        // 调用自动生成的注册方法
        Bridge::InitBuiltInModule(state);

        // 设置命令行参数
        lua_getglobal(state, "lstg");  // t(lstg)
        assert(lua_istable(state, -1));
        lua_newtable(state);  // t(lstg) t
        for (int i = 0, c = 1; i < argc; ++i)
        {
            lua_pushinteger(state, c++);  // t t i
            lua_pushstring(state, argv[i]);  // t t i s
            lua_settable(state, -3);  // t t
        }
        lua_setfield(state, -2, "args");  // t
        lua_pop(state, 1);

        lua_gc(state, LUA_GCRESTART, -1);  // 重启GC
    }

    // 分配对象池
    // TODO

    // 执行 launch 脚本
    {
        LSTG_LOG_TRACE_CAT(GameApp, "Execute \"{}\"", kScriptEntryFile);
        auto ret = GetSubsystem<Subsystem::ScriptSystem>()->LoadScript(kScriptEntryFile);
        if (!ret)
            LSTG_THROW(AppInitializeFailedException, "Fail to execute \"{}\": {}", kScriptEntryFile, ret.GetError());
    }

    // PostInit，根据 launch 配置初始化框架
    // TODO

    // 执行 core.lua 脚本
    {
        LSTG_LOG_TRACE_CAT(GameApp, "Execute \"{}\"", kScriptCoreFile);
        auto ret = GetSubsystem<Subsystem::ScriptSystem>()->LoadScript(kScriptCoreFile);
        if (!ret)
            LSTG_THROW(AppInitializeFailedException, "Fail to execute \"{}\": {}", kScriptCoreFile, ret.GetError());
    }

    // 执行框架 GameInit 方法
    {
        LSTG_LOG_TRACE_CAT(GameApp, "Call \"{}\"", kEventOnGameInit);
        auto ret = GetSubsystem<Subsystem::ScriptSystem>()->CallGlobal<void>(kEventOnGameInit);
        if (!ret)
            LSTG_THROW(AppInitializeFailedException, "Fail to call \"{}\": {}", kEventOnGameInit, ret.GetError());
    }

    // TODO
    GetSubsystem<Subsystem::WindowSystem>()->Show();
}

Result<void> GameApp::MountAssetPack(const char* path, std::optional<std::string_view> password) noexcept
{
    if (!path)
        return make_error_code(errc::invalid_argument);

    // 检查是否已经加载
    for (size_t i = 0; i < m_pAssetsFileSystem->GetFileSystemCount(); ++i)
    {
        auto fs = m_pAssetsFileSystem->GetFileSystem(i);
        assert(fs);
        if (fs->GetUserData() == path)
        {
            LSTG_LOG_WARN_CAT(GameApp, "Asset pack is already loaded, path={}", path);
            return {};
        }
    }

    // 创建 ZipArchiveFileSystem
    try
    {
        // 尝试加载流
        auto fullPath = fmt::format("{}/{}", GetSubsystem<Subsystem::VirtualFileSystem>()->GetAssetBaseDirectory(), path);
        auto stream = GetSubsystem<Subsystem::VirtualFileSystem>()->OpenFile(fullPath, Subsystem::VFS::FileAccessMode::Read);
        if (!stream)
        {
            LSTG_LOG_ERROR_CAT(GameApp, "Open asset pack from \"{}\" fail: {}", path, stream.GetError());
            return stream.GetError();
        }

        auto fs = make_shared<Subsystem::VFS::ZipArchiveFileSystem>(std::move(*stream), password ? string{*password} : "");
        fs->SetUserData(path);
        m_pAssetsFileSystem->PushFileSystem(std::move(fs));
        return {};
    }
    catch (const std::system_error& ex)
    {
        return ex.code();
    }
    catch (...)  // bad_alloc
    {
        return make_error_code(errc::not_enough_memory);
    }
}

Result<void> GameApp::UnmountAssetPack(const char* path) noexcept
{
    if (!path || ::strlen(path) == 0)
        return make_error_code(errc::invalid_argument);

    for (size_t i = 0; i < m_pAssetsFileSystem->GetFileSystemCount(); ++i)
    {
        auto fs = m_pAssetsFileSystem->GetFileSystem(i);
        assert(fs);
        if (fs->GetUserData() == path)
        {
            m_pAssetsFileSystem->RemoveFileSystemAt(i);
            return {};
        }
    }

    LSTG_LOG_ERROR_CAT(GameApp, "Asset pack not found, path={}", path);
    return make_error_code(errc::no_such_file_or_directory);
}

glm::vec2 GameApp::GetNativeResolution() const noexcept
{
    return m_stNativeSolution;
}

glm::vec2 GameApp::GetDesiredResolution() const noexcept
{
    return m_stDesiredSolution;
}

Math::ImageRectangleFloat GameApp::GetViewportBound() const noexcept
{
    return m_stViewportBound;
}

void GameApp::ChangeDesiredResolution(uint32_t width, uint32_t height) noexcept
{
    auto windowSystem = GetSubsystem<Subsystem::WindowSystem>();
    auto currentFullscreen = windowSystem->IsFullScreen();

    m_stDesiredSolution = { std::max(1u, width), std::max(1u, height) };

    // 如果当前环境支持切换分辨率
    if ((windowSystem->GetFeatures() & Subsystem::WindowFeatures::ProgrammingResizable) && !currentFullscreen)
    {
        // TODO 防止窗口超过屏幕大小
        // 这里会触发窗口大小变化事件
        windowSystem->SetSize(static_cast<int>(width), static_cast<int>(height));
    }
    else
    {
        AdjustViewport();
    }
}

void GameApp::ToggleFullScreen(bool fullscreen) noexcept
{
    auto windowSystem = GetSubsystem<Subsystem::WindowSystem>();
    auto currentFullscreen = windowSystem->IsFullScreen();

    // 切换到窗口模式
    if (currentFullscreen && !fullscreen)
    {
        if (windowSystem->GetFeatures() & Subsystem::WindowFeatures::SupportWindowMode)
            windowSystem->ToggleFullScreen(false);
        else
            LSTG_LOG_WARN_CAT(GameApp, "Toggle to window mode is not supported");
    }

    // 切换到全屏模式
    if (!currentFullscreen && fullscreen)
        windowSystem->ToggleFullScreen(true);
}

Subsystem::Render::Drawing2D::CommandBuffer& GameApp::GetCommandBuffer() noexcept
{
    return m_stCommandBuffer;
}

void GameApp::OnEvent(Subsystem::SubsystemEvent& event) noexcept
{
    AppBase::OnEvent(event);

    if (event.IsBubbles())
    {
        const auto& underlay = event.GetEvent();
        if (underlay.index() == 0)
        {
            auto sdlEvent = std::get<0>(underlay);
            assert(sdlEvent);

            if (sdlEvent->type == SDL_WINDOWEVENT)
            {
                if (sdlEvent->window.event == SDL_WINDOWEVENT_RESIZED)
                {
                    // 获取渲染系统的分辨率
                    auto renderDevice = GetSubsystem<Subsystem::RenderSystem>()->GetRenderDevice();
                    assert(renderDevice);

                    auto renderWidth = renderDevice->GetRenderOutputWidth();
                    auto renderHeight = renderDevice->GetRenderOutputHeight();
                    m_stNativeSolution = { renderWidth, renderHeight };

                    AdjustViewport();
                }
                else if (sdlEvent->window.event == SDL_WINDOWEVENT_FOCUS_GAINED)
                {
                    // 执行框架 FocusGainFunc 方法
                    LSTG_LOG_TRACE_CAT(GameApp, "Call \"{}\"", kEventOnGainFocus);
                    auto ret = GetSubsystem<Subsystem::ScriptSystem>()->CallGlobal<void>(kEventOnGainFocus);
                    if (!ret)
                        LSTG_LOG_ERROR_CAT(GameApp, "Fail to call \"{}\": {}", kEventOnGainFocus, ret.GetError());
                }
                else if (sdlEvent->window.event == SDL_WINDOWEVENT_FOCUS_LOST)
                {
                    // 执行框架 FocusLoseFunc 方法
                    LSTG_LOG_TRACE_CAT(GameApp, "Call \"{}\"", kEventOnLostFocus);
                    auto ret = GetSubsystem<Subsystem::ScriptSystem>()->CallGlobal<void>(kEventOnLostFocus);
                    if (!ret)
                        LSTG_LOG_ERROR_CAT(GameApp, "Fail to call \"{}\": {}", kEventOnLostFocus, ret.GetError());
                }
            }
        }
    }
}

void GameApp::OnUpdate(double elapsed) noexcept
{
    // 执行框架 FrameFunc 方法
    auto ret = GetSubsystem<Subsystem::ScriptSystem>()->CallGlobal<bool>(kEventOnUpdate);
    if (!ret)
    {
        LSTG_LOG_ERROR_CAT(GameApp, "Fail to call \"{}\": {}", kEventOnUpdate, ret.GetError());
    }
    else if (*ret)
    {
        LSTG_LOG_TRACE_CAT(GameApp, "{} -> true, exit main loop", kEventOnUpdate);
        Stop();
    }
}

void GameApp::OnRender(double elapsed) noexcept
{
    // 启动场景
    m_stCommandBuffer.Begin();

    // 执行框架 RenderFunc 方法
    auto ret = GetSubsystem<Subsystem::ScriptSystem>()->CallGlobal<void>(kEventOnRender);
    if (!ret)
        LSTG_LOG_ERROR_CAT(GameApp, "Fail to call \"{}\": {}", kEventOnRender, ret.GetError());

    // 结束场景
    auto drawData = m_stCommandBuffer.End();

    // 渲染
    m_stCommandExecutor.Execute(drawData);
}

void GameApp::AdjustViewport() noexcept
{
    // 计算 Scale
    // 例如 NativeSolution = 1280x720，DesiredSolution = 640x480
    // 则 1280/640 = 2, 720/480 = 1.5
    // 取 scale = 1.5，则 vp = 1.5 * 640 x 1.5 * 480 = 960x720
    // 例如 NativeSolution = 480x320，DesiredSolution = 640x480
    // 则 480/640 = 0.75, 320/480 = 0.667
    // 取 scale = 0.667，则 vp = 0.667 * 640 x 0.667 * 480 = 427x320
    auto scale = std::min(m_stNativeSolution.x / m_stDesiredSolution.x, m_stNativeSolution.y / m_stDesiredSolution.y);
    auto halfWidth = m_stDesiredSolution.x * scale / 2.f;
    auto halfHeight = m_stDesiredSolution.y * scale / 2.f;
    auto centerX = m_stNativeSolution.x / 2.f;
    auto centerY = m_stNativeSolution.y / 2.f;
    m_stViewportBound = {
        std::max(0.f, std::ceil(centerX - halfWidth)),
        std::max(0.f, std::ceil(centerY - halfHeight)),
        std::ceil(halfWidth * 2.f),
        std::ceil(halfHeight * 2.f)
    };
    LSTG_LOG_DEBUG_CAT(GameApp, "Adjust viewport to {}x{}-{}x{}", m_stViewportBound.Left(), m_stViewportBound.Top(),
        m_stViewportBound.GetBottomRight().x, m_stViewportBound.GetBottomRight().y);
}
