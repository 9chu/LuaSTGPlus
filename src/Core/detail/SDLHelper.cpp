/**
 * @file
 * @author 9chu
 * @date 2022/2/16
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#include "SDLHelper.hpp"

using namespace std;
using namespace lstg;

const detail::SDLErrorCategory& detail::SDLErrorCategory::GetInstance() noexcept
{
    static const SDLErrorCategory kInstance;
    return kInstance;
}

const char* detail::SDLErrorCategory::name() const noexcept
{
    return "SDLError";
}

std::string detail::SDLErrorCategory::message(int ev) const
{
    // SDL 并没有一个枚举类型的错误码，这里总是获取最近一次的错误
    return SDL_GetError();
}
