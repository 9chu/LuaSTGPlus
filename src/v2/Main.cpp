/**
 * @file
 * @author 9chu
 * @date 2022/2/14
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#include <lstg/v2/GameApp.hpp>

#include <memory>
#include <SDL.h>
#include <lstg/Core/Pal.hpp>
#include <lstg/Core/Logging.hpp>

using namespace std;
using namespace lstg;
using namespace lstg::v2;

extern "C" int main(int argc, char** argv)
{
    // 强制日志系统初始化
    Logging::GetInstance();

    // 初始化 GameApp
#ifdef LSTG_PLATFORM_EMSCRIPTEN
    static std::unique_ptr<GameApp> app;  // 防止退出 main 后被析构
#else
    std::unique_ptr<GameApp> app;
#endif
    try
    {
        app = std::make_unique<GameApp>(argc, argv);

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
