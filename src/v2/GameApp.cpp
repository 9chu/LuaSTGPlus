/**
 * @file
 * @author 9chu
 * @date 2022/2/16
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#include <lstg/v2/GameApp.hpp>

#include <SDL.h>
#include <glm/ext.hpp>

#include <lstg/Core/Pal.hpp>
#include <lstg/Core/Logging.hpp>
#include <lstg/Core/Encoding/Unicode.hpp>
#include <lstg/Core/Encoding/Convert.hpp>
#include <lstg/Core/Subsystem/AssetSystem.hpp>
#include <lstg/Core/Subsystem/VirtualFileSystem.hpp>
#include <lstg/Core/Subsystem/ScriptSystem.hpp>
#include <lstg/Core/Subsystem/WindowSystem.hpp>
#include <lstg/Core/Subsystem/DebugGUISystem.hpp>
#include <lstg/Core/Subsystem/DebugGUI/MiniStatusWindow.hpp>
#include <lstg/Core/Subsystem/DebugGUI/ConsoleWindow.hpp>
#include <lstg/Core/Subsystem/VFS/FileStream.hpp>
#include <lstg/Core/Subsystem/VFS/LocalFileSystem.hpp>
#include <lstg/Core/Subsystem/VFS/ZipArchiveFileSystem.hpp>
#include <lstg/Core/Subsystem/Script/LuaStack.hpp>
#include <lstg/v2/DebugGUI/PerformanceMonitor.hpp>
#include "detail/KeyMapping.hpp"

// 资源类型
#include <lstg/v2/Asset/TextureAsset.hpp>
#include <lstg/v2/Asset/TextureAssetFactory.hpp>
#include <lstg/v2/Asset/SpriteAssetFactory.hpp>
#include <lstg/v2/Asset/SpriteSequenceAssetFactory.hpp>
#include <lstg/v2/Asset/TrueTypeFontAssetFactory.hpp>
#include <lstg/v2/Asset/HgeFontAssetFactory.hpp>
#include <lstg/v2/Asset/HgeParticleAssetFactory.hpp>
#include <lstg/v2/Asset/EffectAssetFactory.hpp>

// 脚本桥
#include <lstg/v2/Bridge/BuiltInModules.hpp>

using namespace std;
using namespace lstg;
using namespace lstg::v2;

namespace
{
    /**
     * 向 string 中追加 char32_t
     * @param out 输出 UTF-8 串
     * @param ch Unicode字符
     * @return 是否成功
     */
    bool AppendUtf8(std::string& out, char32_t ch) noexcept
    {
        uint32_t encodedCount = 0;
        array<char, Encoding::Utf8::Encoder::kMaxOutputCount> state {};
        Encoding::Utf8::Encoder encoder;
        if (Encoding::EncodingResult::Accept != encoder(ch, state, encodedCount))
            return false;
        assert(encodedCount > 0);
        // out 总是预留了足够空间，不会出现分配问题
        out.append(state.data(), encodedCount);
        return true;
    }
}

LSTG_DEF_LOG_CATEGORY(GameApp);

static const char* kScriptEntryFile = "launch";
static const char* kScriptCoreFile = "core.lua";

static const char* kEventOnGameInit = "GameInit";
static const char* kEventOnGainFocus = "FocusGainFunc";
static const char* kEventOnLostFocus = "FocusLoseFunc";
static const char* kEventOnUpdate = "FrameFunc";
static const char* kEventOnRender = "RenderFunc";

static size_t kMaxRenderTargetStackDepth = 8u;

extern "C" int luaopen_cjson(lua_State* L);

GameApp::GameApp(int argc, const char* argv[])
    : AppBase(argc, argv), m_stDesiredSolution(640, 480), m_stCommandExecutor(*GetSubsystem<Subsystem::RenderSystem>()),
    m_stDefaultWorld(*this)
{
    // 初始化文件系统
    //  - assets
    //    - ...
    //    - core.lua
    //    - launch
    // 为了保持与老 API 的兼容性，默认资源寻找均从 assets 开始。
    // 可以通过 ../ 来访问其他路径。
    {
        auto& virtualFileSystem = *GetSubsystem<Subsystem::VirtualFileSystem>();

        // assets 目录通过 OverlayFileSystem 实现
        // 最下面是 LocalFileSystem，这使得在其他 FileSystem 上搜索不到时会到本地文件系统进行搜寻
        m_pAssetsFileSystem = make_shared<Subsystem::VFS::OverlayFileSystem>();

        // 准备 LocalFileSystem
#ifdef LSTG_SHIPPING
        auto base = filesystem::path(argv[0]);
        auto localFileSystem = make_shared<Subsystem::VFS::LocalFileSystem>(base.parent_path());  // 发布环境下总是与应用程序一个目录
#else
        auto localFileSystem = make_shared<Subsystem::VFS::LocalFileSystem>(".");  // DEV 模式下任意
#endif
        m_pAssetsFileSystem->PushFileSystem(std::move(localFileSystem));

        // 挂载
        auto ret = virtualFileSystem.Mount("assets", m_pAssetsFileSystem);
        if (!ret)
            LSTG_THROW(AppInitializeFailedException, "Fail to mount \"assets\" virtual directory: {}", ret.GetError());

        // 设置资源系统基准目录
        virtualFileSystem.SetAssetBaseDirectory("assets");

        // 准备数据存储目录
        auto localStorageFileSystem = make_shared<Subsystem::VFS::LocalFileSystem>(Pal::GetUserStorageDirectory());

        // 挂载
        ret = virtualFileSystem.Mount("storage", localStorageFileSystem);
        if (!ret)
            LSTG_THROW(AppInitializeFailedException, "Fail to mount \"storage\" virtual directory: {}", ret.GetError());
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
        assetSystem->RegisterAssetFactory(make_shared<Asset::EffectAssetFactory>());

        // TODO: Music Sound

        // 创建资源池
        m_pAssetPools = make_unique<AssetPools>();
        m_pInternalAssetPool = make_shared<Subsystem::Asset::AssetPool>();

        // 绑定资产解析器
        assetSystem->SetDependencyResolver(m_pAssetPools.get());
    }

    // 初始化渲染系统
    {
        auto& renderSystem = *GetSubsystem<Subsystem::RenderSystem>();

        // 初始化渲染参数
        auto renderDevice = renderSystem.GetRenderDevice();
        assert(renderDevice);
        m_stNativeSolution = { renderDevice->GetRenderOutputWidth(), renderDevice->GetRenderOutputHeight() };
        AdjustViewport();

        // 刷初始渲染状态
        m_stCommandBuffer.Begin();
        m_stCommandBuffer.SetNoDepth(true);  // 默认关闭深度测试
        m_stCommandBuffer.SetView(glm::identity<glm::mat4x4>());
        m_stCommandBuffer.SetProjection(glm::ortho<float>(0.f, static_cast<float>(m_stViewportBound.Width()),
                                                          0.f, static_cast<float>(m_stViewportBound.Height()),
                                                          0.f, 100.0f));
        m_stCommandBuffer.SetViewport(static_cast<float>(m_stViewportBound.Left()), static_cast<float>(m_stViewportBound.Top()),
                                      static_cast<float>(m_stViewportBound.Width()), static_cast<float>(m_stViewportBound.Height()));
        m_stCommandBuffer.End();

        // 初始化文字渲染组件
        m_pTextShaper = Subsystem::Render::Font::CreateHarfBuzzTextShaper();
        m_pFontGlyphAtlas = make_shared<Subsystem::Render::Font::DynamicFontGlyphAtlas>(renderSystem);

        // 初始化 RT Stack
        m_stRenderTargetStack.reserve(kMaxRenderTargetStackDepth);
    }

    // 初始化输入系统
    {
        m_uInputWindowID = ::SDL_GetWindowID(GetSubsystem<Subsystem::WindowSystem>()->GetNativeHandle());
        m_stLastInputChar.reserve(16);
        m_stKeyStateMap.resize(SDL_NUM_SCANCODES);
        ::memset(m_stMouseButtonStateMap, 0, sizeof(m_stMouseButtonStateMap));
    }

    // 初始化性能分析系统
    {
        auto& debugGUI = *GetSubsystem<Subsystem::DebugGUISystem>();

        // 增加对象数计数显示
        // GameWorld.cpp:Update
        debugGUI.GetMiniStatusWindow()->AddCounter("OBJ", Subsystem::PerformanceCounterTypes::PerFrame, "GameWorld_EntityCount");

#ifdef LSTG_DEVELOPMENT
        // 增加 PerformanceMonitor
        auto pm = make_shared<v2::DebugGUI::PerformanceMonitor>();
        debugGUI.AppendWindow(pm).ThrowIfError();
        debugGUI.GetConsoleWindow()->AddContextMenuItem("Toggle PerformanceMonitor", [pm]() {
            pm->IsVisible() ? pm->Hide() : pm->Show();
        }).ThrowIfError();
#endif
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
#ifdef LSTG_PARSE_CMDLINE
        {
            int c = 1;
            const auto& cmdline = GetCmdline();

            // 兼容性处理，argv[1] 总是 executable path
            state.PushValue(c++);  // t t i
            state.PushValue(cmdline.GetExecutablePath());  // t t i s
            lua_settable(state, -3);  // t t

            for (size_t i = 0; i < cmdline.GetTransparentArgumentCount(); ++i)
            {
                state.PushValue(c++);  // t t i
                state.PushValue(cmdline.GetTransparentArgument(i));  // t t i s
                lua_settable(state, -3);  // t t
            }
        }
#else
        for (int i = 0, c = 1; i < argc; ++i)
        {
            lua_pushinteger(state, c++);  // t t i
            lua_pushstring(state, argv[i]);  // t t i s
            lua_settable(state, -3);  // t t
        }
#endif
        lua_setfield(state, -2, "args");  // t
        lua_pop(state, 1);

        // 修补高版本没有 math.mod
        lua_getglobal(state, "math");  // t
        assert(!lua_isnil(state, -1));
        lua_getfield(state, -1, "mod");  // t f|n
        if (lua_isnil(state, -1))
        {
            lua_pop(state, 1);  // t
            lua_getfield(state, -1, "fmod");  // t f
            lua_setfield(state, -2, "mod");  // t
            lua_pop(state, 1);
        }
        else
        {
            lua_pop(state, 2);
        }

        lua_gc(state, LUA_GCRESTART, -1);  // 重启GC
    }
}

// <editor-fold desc="资源系统">

Result<void> GameApp::MountAssetPack(std::string_view path, std::optional<std::string_view> password, bool vfsBypass) noexcept
{
    if (path.empty())
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
        Subsystem::VFS::StreamPtr packageStream;

        if (vfsBypass)
        {
            try
            {
                auto fsStream = make_shared<Subsystem::VFS::FileStream>(path, Subsystem::VFS::FileAccessMode::Read,
                    Subsystem::VFS::FileOpenFlags::None);
                packageStream = std::move(fsStream);
            }
            catch (const std::system_error& ex)
            {
                LSTG_LOG_ERROR_CAT(GameApp, "Open asset pack from \"{}\" fail: {}", path, ex.code());
                return ex.code();
            }
            catch (...)
            {
                LSTG_LOG_ERROR_CAT(GameApp, "Open asset pack from \"{}\" fail: <unknown>", path);
                return make_error_code(errc::io_error);
            }
        }
        else
        {
            // 尝试加载流
            auto fullPath = fmt::format("{}/{}", GetSubsystem<Subsystem::VirtualFileSystem>()->GetAssetBaseDirectory(), path);
            auto stream = GetSubsystem<Subsystem::VirtualFileSystem>()->OpenFile(fullPath, Subsystem::VFS::FileAccessMode::Read);
            if (!stream)
            {
                LSTG_LOG_ERROR_CAT(GameApp, "Open asset pack from \"{}\" fail: {}", path, stream.GetError());
                return stream.GetError();
            }
            packageStream = std::move(*stream);
        }
        auto fs = make_shared<Subsystem::VFS::ZipArchiveFileSystem>(packageStream, password ? string{*password} : "");
        fs->SetUserData(string{path});
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

Result<void> GameApp::UnmountAssetPack(std::string_view path) noexcept
{
    if (path.empty())
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

// </editor-fold>
// <editor-fold desc="渲染系统">

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

glm::vec2 GameApp::WindowCoordToDesiredCoord(glm::vec2 pos) noexcept
{
    auto windowSystem = GetSubsystem<Subsystem::WindowSystem>();
    auto windowSize = windowSystem->GetSize();

    // 相对窗口坐标系的 scale
    auto sx = pos.x / static_cast<float>(std::max(1, std::get<0>(windowSize)));
    auto sy = pos.y / static_cast<float>(std::max(1, std::get<1>(windowSize)));

    // 转换到原生坐标系下
    auto nx = m_stNativeSolution.x * sx;
    auto ny = m_stNativeSolution.y * sy;

    // 计算到视口左上角的偏移量
    auto topLeft = m_stViewportBound.GetTopLeft();
    auto ox = (nx - topLeft.x) / m_stViewportBound.Width();
    auto oy = (ny - topLeft.y) / m_stViewportBound.Height();

    // 计算到设计分辨率的坐标（以左上角为原点，Y向下）
    auto dx = ox * m_stDesiredSolution.x;
    auto dy = oy * m_stDesiredSolution.y;

    // 需要转换到以左下角为原点
    dy = m_stDesiredSolution.y - dy;
    return { dx, dy };
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

Result<void> GameApp::PushRenderTarget(Subsystem::Render::TexturePtr rt) noexcept
{
    if (m_stRenderTargetStack.size() >= kMaxRenderTargetStackDepth)
    {
        LSTG_LOG_ERROR_CAT(GameApp, "RT stack reaches limitation");
        return make_error_code(errc::invalid_argument);
    }

    // 检查是否已经在栈上
    if (IsRenderTargetInStack(rt.get()))
    {
        LSTG_LOG_ERROR_CAT(GameApp, "RT is already in stack");
        return make_error_code(errc::invalid_argument);
    }

    // 设置到 CommandBuffer
    m_stCommandBuffer.SetOutputViews(rt, nullptr);

    m_stRenderTargetStack.emplace_back(std::move(rt));  // never throws
    return {};
}

Result<Subsystem::Render::TexturePtr> GameApp::PopRenderTarget() noexcept
{
    if (m_stRenderTargetStack.empty())
        return make_error_code(errc::invalid_argument);
    auto ret = std::move(m_stRenderTargetStack.back());
    m_stRenderTargetStack.pop_back();

    // 恢复栈顶的 RT
    if (!m_stRenderTargetStack.empty())
        m_stCommandBuffer.SetOutputViews(m_stRenderTargetStack.back(), nullptr);
    else
        m_stCommandBuffer.SetOutputViews(nullptr, nullptr);

    return ret;
}

bool GameApp::IsRenderTargetInStack(Subsystem::Render::Texture* rt) noexcept
{
    for (const auto& e : m_stRenderTargetStack)
    {
        if (rt == e.get())
            return true;
    }
    return false;
}

Result<Subsystem::Render::TexturePtr> GameApp::GetDefaultRenderTarget() noexcept
{
    // 延迟构造 DefaultRT
    if (!m_pDefaultRenderTargetAsset)
    {
        auto assetSystem = GetSubsystem<Subsystem::AssetSystem>();
        assert(assetSystem);
        auto rtAsset = assetSystem->CreateAsset<Asset::TextureAsset>(m_pInternalAssetPool, "DefaultRT", {
            { "rt", true }
        });
        if (!rtAsset)
            return rtAsset.GetError();
        m_pDefaultRenderTargetAsset = std::move(*rtAsset);
    }

    return m_pDefaultRenderTargetAsset->GetDrawingTexture().GetUnderlayTexture();
}

// </editor-fold>
// <editor-fold desc="输入系统">

int32_t GameApp::GetLastInputKeyCode() const noexcept
{
    return detail::SDLScancodeToVKCode(static_cast<SDL_Scancode>(m_iLastInputKeyCode));
}

bool GameApp::IsKeyDown(int32_t keyCode) const noexcept
{
    auto scanCode = detail::VKCodeToSDLScanCode(keyCode);
    if (0 <= scanCode && scanCode < m_stKeyStateMap.size())
        return m_stKeyStateMap[scanCode];
    return false;
}

glm::vec2 GameApp::GetMousePosition() const noexcept
{
    return m_stMousePosition;
}

bool GameApp::IsMouseButtonDown(MouseButtons button) const noexcept
{
    assert(button < MouseButtons::MAX);
    return m_stMouseButtonStateMap[static_cast<uint32_t>(button)];
}

// </editor-fold>
// <editor-fold desc="框架事件">

void GameApp::OnStartup()
{
    // 预加载资源包
    {
        auto preloadPackPath = GetCmdline().GetOption<string_view>("preload-pack", "");
        if (!preloadPackPath.empty())
        {
            // 预加载资源包，失败直接退出
            MountAssetPack(preloadPackPath, {}, true).ThrowIfError();
        }
    }

    LSTG_LOG_TRACE_CAT(GameApp, "Startup script layer");

    // 执行 launch 脚本
    {
        LSTG_LOG_TRACE_CAT(GameApp, "Execute \"{}\"", kScriptEntryFile);
        auto ret = GetSubsystem<Subsystem::ScriptSystem>()->LoadScript(kScriptEntryFile);
        if (!ret)
            LSTG_THROW(AppInitializeFailedException, "Fail to execute \"{}\": {}", kScriptEntryFile, ret.GetError());
    }

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

    // 显示主窗口
    GetSubsystem<Subsystem::WindowSystem>()->Show();
    GetSubsystem<Subsystem::WindowSystem>()->Raise();
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
                if (sdlEvent->window.windowID == m_uInputWindowID)  // 过滤非主窗口事件
                {
                    if (sdlEvent->window.event == SDL_WINDOWEVENT_RESIZED)
                    {
                        LSTG_LOG_INFO_CAT(GameApp, "Window resize to {}x{}", sdlEvent->window.data1, sdlEvent->window.data2);

                        // 获取渲染系统的分辨率
                        auto renderDevice = GetSubsystem<Subsystem::RenderSystem>()->GetRenderDevice();
                        assert(renderDevice);

                        auto renderWidth = renderDevice->GetRenderOutputWidth();
                        auto renderHeight = renderDevice->GetRenderOutputHeight();
                        m_stNativeSolution = { renderWidth, renderHeight };

                        // 调整视口
                        AdjustViewport();

                        // 调整所有 RenderTarget 的大小
                        auto textureAssetFactory = static_pointer_cast<Asset::TextureAssetFactory>(
                            GetSubsystem<Subsystem::AssetSystem>()->FindAssetFactory<Asset::TextureAsset>());
                        assert(textureAssetFactory);
                        textureAssetFactory->ResizeRenderTarget(renderWidth, renderHeight);
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
            else if (sdlEvent->type == SDL_TEXTINPUT)
            {
                if (sdlEvent->text.windowID == m_uInputWindowID)  // 过滤非主窗口事件
                {
                    char32_t lastChar = '\0';
                    auto inputText = Span<const char>(sdlEvent->text.text, ::strlen(sdlEvent->text.text));

                    // 兼容性处理，取出输入的最后一个字符
                    Encoding::EncodingView<Encoding::Utf8::Decoder> view(inputText);
                    auto it = view.begin(), jt = view.end();
                    while (it != jt)
                    {
                        auto ret = *it;
                        if (!ret)
                        {
                            LSTG_LOG_ERROR_CAT(GameApp, "Decode input text error: {}", ret.GetError());
                            break;
                        }
                        assert(ret->GetSize() == 1);
                        lastChar = (*ret)[0];
                        ++it;
                    }

                    if (lastChar)
                    {
                        m_stLastInputChar.clear();
                        if (!AppendUtf8(m_stLastInputChar, lastChar))
                            LSTG_LOG_ERROR_CAT(GameApp, "Encode input text error");
                    }
                }
            }
            else if (sdlEvent->type == SDL_KEYDOWN || sdlEvent->type == SDL_KEYUP)
            {
                if (sdlEvent->key.windowID == m_uInputWindowID)  // 过滤非主窗口事件
                {
                    auto scanCode = sdlEvent->key.keysym.scancode;
                    assert(m_stKeyStateMap.size() == SDL_NUM_SCANCODES);
                    if (0 <= scanCode && scanCode < m_stKeyStateMap.size())
                        m_stKeyStateMap[scanCode] = (sdlEvent->key.state == SDL_PRESSED);

                    // 更新最后一次键入 Key
                    if (sdlEvent->key.state == SDL_PRESSED)
                        m_iLastInputKeyCode = sdlEvent->key.keysym.scancode;
                    else if (sdlEvent->key.keysym.scancode == m_iLastInputKeyCode)
                        m_iLastInputKeyCode = '\0';
                }
            }
            else if (sdlEvent->type == SDL_MOUSEMOTION)
            {
                if (sdlEvent->motion.windowID == m_uInputWindowID)  // 过滤非主窗口事件
                {
                    m_stMousePosition = WindowCoordToDesiredCoord({
                        static_cast<float>(sdlEvent->motion.x),
                        static_cast<float>(sdlEvent->motion.y)
                    });
                }
            }
            else if (sdlEvent->type == SDL_MOUSEBUTTONDOWN || sdlEvent->type == SDL_MOUSEBUTTONUP)
            {
                if (sdlEvent->button.windowID == m_uInputWindowID)  // 过滤非主窗口事件
                {
                    MouseButtons button = MouseButtons::MAX;
                    switch (sdlEvent->button.button)
                    {
                        case SDL_BUTTON_LEFT:
                            button = MouseButtons::Left;
                            break;
                        case SDL_BUTTON_RIGHT:
                            button = MouseButtons::Right;
                            break;
                        case SDL_BUTTON_MIDDLE:
                            button = MouseButtons::Middle;
                            break;
                    }
                    if (button != MouseButtons::MAX)
                        m_stMouseButtonStateMap[static_cast<uint32_t>(button)] = (sdlEvent->button.state == SDL_PRESSED);
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

    // 更新 GameWorld 的内部状态
    m_stDefaultWorld.Update(elapsed);

    // 帧末清理单帧输入状态
    {
        m_stLastInputChar.clear();
        m_iLastInputKeyCode = '\0';
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

    // 检查所有的 RT 是否从栈上弹出
    if (!m_stRenderTargetStack.empty())
    {
        LSTG_LOG_ERROR_CAT(GameApp, "Render target stack is not balanced");
        while (!m_stRenderTargetStack.empty())
            PopRenderTarget();
    }

    // 结束场景
    auto drawData = m_stCommandBuffer.End();

    // 上传字体图集
    auto atlasRet = m_pFontGlyphAtlas->Commit();
    if (!atlasRet)
        LSTG_LOG_ERROR_CAT(GameApp, "Fail to commit glyph atlas: {}", atlasRet.GetError());

    // 渲染
    {
#ifdef LSTG_DEVELOPMENT
        LSTG_PER_FRAME_PROFILE(Draw_ExecutionTime);
#endif

        m_stCommandExecutor.Execute(drawData);

#ifdef LSTG_DEVELOPMENT
#define ADD_COUNTER(NAME, WHAT) \
        Subsystem::ProfileSystem::GetInstance().IncrementPerformanceCounter(Subsystem::PerformanceCounterTypes::PerFrame, #NAME, WHAT)

        // 绘图统计
        ADD_COUNTER(Draw_VertexCount, static_cast<double>(drawData.VertexBuffer.size()));
        ADD_COUNTER(Draw_PrimitiveCount, static_cast<double>(drawData.IndexBuffer.size() / 6));
        ADD_COUNTER(Draw_DrawCallCount, static_cast<double>(m_stCommandExecutor.GetLastExecutedDrawCalls()));
#undef ADD_COUNTER
#endif
    }
}

// </editor-fold>

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
