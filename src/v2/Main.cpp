/**
 * @file
 * @author 9chu
 * @date 2022/2/14
 * 此文件为 LuaSTGPlus 项目的一部分，版权与许可声明详见 COPYRIGHT.txt。
 */
#include <lstg/v2/GameApp.hpp>

#include <memory>
#include <SDL.h>
#include <lstg/Core/Pal.hpp>
#include <lstg/Core/Logging.hpp>

// 版本信息
#include <Version.hpp>

using namespace std;
using namespace lstg;
using namespace lstg::v2;

SDLMAIN_DECLSPEC int main(int argc, char* argv[])
{
    // 初始化命令行参数
    auto cargv = const_cast<char const ** const>(argv);
    AppBase::ParseCmdline(argc, cargv);

    // 强制日志系统初始化
    Logging::GetInstance();
#ifdef LSTG_DEVELOPMENT
    LSTG_LOG_INFO("Version: {} (Development mode)", LSTG_VERSION);
#else
    LSTG_LOG_INFO("Version: {} (Shipping mode)", LSTG_VERSION);
#endif

    // 初始化 GameApp
#ifdef LSTG_PLATFORM_EMSCRIPTEN
    static std::unique_ptr<GameApp> app;  // 防止退出 main 后被析构
#else
    std::unique_ptr<GameApp> app;
#endif
    try
    {
        app = std::make_unique<GameApp>(argc, cargv);

        // 启动
        app->Run();
    }
    catch (const std::exception& ex)
    {
        Pal::FatalError(ex.what(), false);
        return 1;
    }

    return 0;
}
