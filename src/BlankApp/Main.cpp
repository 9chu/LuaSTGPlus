/**
 * @file
 * @author 9chu
 * @date 2022/3/1
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#include <memory>
#include <SDL.h>
#include <lstg/Core/Pal.hpp>
#include <lstg/Core/Logging.hpp>

using namespace std;
using namespace lstg;

extern "C" int main(int argc, char** argv)
{
    // 强制日志系统初始化
    Logging::GetInstance();

    // NOTE: 在这里编写测试代码

    return 0;
}
