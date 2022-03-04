/**
 * @file
 * @author 9chu
 * @date 2022/2/16
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#include <lstg/v2/GameApp.hpp>

#include <lstg/Core/Subsystem/VFS/LocalFileSystem.hpp>
#include <lstg/Core/Subsystem/VFS/ZipArchiveFileSystem.hpp>

using namespace std;
using namespace lstg;
using namespace lstg::v2;

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

        // 准备 LocalFileSystem
#ifdef LSTG_SHIPPING
        auto base = filesystem::path(argv[0]);
        auto localFileSystem = make_shared<Subsystem::VFS::LocalFileSystem>(base.parent_path());  // 发布环境下总是与应用程序一个目录
#else
        auto localFileSystem = make_shared<Subsystem::VFS::LocalFileSystem>(".");  // DEV 模式下任意
#endif
        m_pAssetsFileSystem->PushFileSystem(std::move(localFileSystem));

        // 挂载
        auto ret = GetVirtualFileSystem().Mount("assets", m_pAssetsFileSystem);
        ret.ThrowIfError();
    }

    // 初始化函数库
    {
        auto& state = GetScriptSystem().GetState();
        lua_gc(state, LUA_GCSTOP, 0);  // 初始化时关闭GC

        // 调用自动生成的注册方法
        LuaModuleAutoBridge(state);

        lua_gc(state, LUA_GCRESTART, -1);  // 重启GC
    }

    // 设置命令行参数
//    regex tDebuggerPattern("\\/debugger:(\\d+)");
//    lua_getglobal(L, "lstg");  // t
//    lua_newtable(L);  // t t
//    for (int i = 0, c = 1; i < __argc; ++i)
//    {
//        cmatch tMatch;
//        if (regex_match(__argv[i], tMatch, tDebuggerPattern))
//        {
//#if (defined LDEVVERSION) || (defined LDEBUG)
//            // 创建调试器
//			if (!m_DebuggerClient)
//			{
//				fuShort tPort = atoi(tMatch[1].first);
//
//				try
//				{
//					m_DebuggerClient = make_unique<RemoteDebuggerClient>(tPort);
//					LINFO("调试器已创建，于端口：%d", (fuInt)tPort);
//				}
//				catch (const fcyException& e)
//				{
//					LERROR("创建调试器失败 (详细信息: %m)", e.GetDesc());
//				}
//			}
//			else
//				LWARNING("命令行参数中带有多个/debugger项，忽略。");
//#endif
//            // 不将debugger项传入用户命令行参数中
//            continue;
//        }
//        lua_pushinteger(L, c++);  // t t i
//        lua_pushstring(L, __argv[i]);  // t t i s
//        lua_settable(L, -3);  // t t
//    }
//    lua_setfield(L, -2, "args");  // t
//    lua_pop(L, 1);

    // 分配对象池
    // TODO

    // 执行 launch 脚本
    // TODO

    // PostInit，根据 launch 配置初始化框架
    // TODO

    // 执行 core.lua 脚本
    // TODO
}
