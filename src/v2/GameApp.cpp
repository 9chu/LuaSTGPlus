/**
 * @file
 * @author 9chu
 * @date 2022/2/16
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#include <lstg/v2/GameApp.hpp>

#include <lstg/Core/Logging.hpp>
#include <lstg/Core/Subsystem/VirtualFileSystem.hpp>
#include <lstg/Core/Subsystem/ScriptSystem.hpp>
#include <lstg/Core/Subsystem/VFS/LocalFileSystem.hpp>
#include <lstg/Core/Subsystem/VFS/ZipArchiveFileSystem.hpp>
#include <lstg/Core/Subsystem/VFS/WebFileSystem.hpp>
#include <lstg/Core/Subsystem/Script/LuaStack.hpp>

using namespace std;
using namespace lstg;
using namespace lstg::v2;

LSTG_DEF_LOG_CATEGORY(GameApp);

extern void LuaModuleAutoBridge(Subsystem::Script::LuaStack&);

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

#ifdef __EMSCRIPTEN__
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

        // 设置脚本系统基准目录
        GetSubsystem<Subsystem::ScriptSystem>()->GetSandBox().SetBaseDirectory("assets");
    }

    // 初始化函数库
    {
        auto& state = GetSubsystem<Subsystem::ScriptSystem>()->GetState();
        Subsystem::Script::LuaStack::BalanceChecker stackChecker(state);

        lua_gc(state, LUA_GCSTOP, 0);  // 初始化时关闭GC

        // 调用自动生成的注册方法
        LuaModuleAutoBridge(state);

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
        LSTG_LOG_INFO_CAT(GameApp, "Execute launch script");
        auto ret = GetSubsystem<Subsystem::ScriptSystem>()->LoadScript("launch");
        if (!ret)
            LSTG_THROW(AppInitializeFailedException, "Fail to execute \"launch\" script: {}", ret.GetError());
    }

    // PostInit，根据 launch 配置初始化框架
    // TODO

    // 执行 core.lua 脚本
    // TODO
}
