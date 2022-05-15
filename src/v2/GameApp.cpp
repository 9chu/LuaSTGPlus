/**
 * @file
 * @author 9chu
 * @date 2022/2/16
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#include <lstg/v2/GameApp.hpp>

#include <SDL.h>
#include <lstg/Core/Logging.hpp>
#include <lstg/Core/Subsystem/VirtualFileSystem.hpp>
#include <lstg/Core/Subsystem/ScriptSystem.hpp>
#include <lstg/Core/Subsystem/WindowSystem.hpp>
#include <lstg/Core/Subsystem/VFS/LocalFileSystem.hpp>
#include <lstg/Core/Subsystem/VFS/ZipArchiveFileSystem.hpp>
#include <lstg/Core/Subsystem/VFS/WebFileSystem.hpp>
#include <lstg/Core/Subsystem/Script/LuaStack.hpp>
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

    // 初始化资源池
    m_pGlobalAssetPool = make_shared<Subsystem::Asset::AssetPool>();
    m_pStageAssetPool = make_shared<Subsystem::Asset::AssetPool>();
    m_pCurrentAssetPool = m_pGlobalAssetPool;  // 默认挂载在全局池上

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

Subsystem::Asset::AssetPtr GameApp::FindAsset(std::string_view name) const noexcept
{
    auto ret = m_pStageAssetPool->GetAsset(name);
    if (!ret)
        ret = m_pGlobalAssetPool->GetAsset(name);
    return ret;
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
                if (sdlEvent->window.event == SDL_WINDOWEVENT_FOCUS_GAINED)
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
    // 执行框架 RenderFunc 方法
    auto ret = GetSubsystem<Subsystem::ScriptSystem>()->CallGlobal<void>(kEventOnRender);
    if (!ret)
        LSTG_LOG_ERROR_CAT(GameApp, "Fail to call \"{}\": {}", kEventOnRender, ret.GetError());
}
