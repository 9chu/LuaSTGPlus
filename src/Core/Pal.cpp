/**
 * @file
 * @date 2022/2/15
 * @author 9chu
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#include <lstg/Core/Pal.hpp>

#include "detail/SDLHelper.hpp"

using namespace std;
using namespace lstg;

std::filesystem::path Pal::GetUserStorageDirectory() noexcept
{
    try
    {
        detail::SDLMemoryPtr<char> appDirectory(SDL_GetPrefPath("lstgplus", LSTG_APP_NAME));
        if (appDirectory)
            return appDirectory.get();
        else
            return filesystem::current_path();
    }
    catch (...)
    {
        return {};
    }
}

void Pal::FatalError(const char* msg, bool abort) noexcept
{
    // 在终端显示错误
    ::fprintf(stderr, "FATAL ERROR: %s", msg);

    // 弹出对话框，忽略返回值
    // FIXME: 设置 parent window
    ::SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "LuaSTGPlus: Fatal Error", msg, nullptr);

    // 终止
    if (abort)
        ::abort();
}

uint64_t Pal::GetCurrentTick() noexcept
{
    return ::SDL_GetPerformanceCounter();
}

uint64_t Pal::GetTickFrequency() noexcept
{
    return ::SDL_GetPerformanceFrequency();
}
