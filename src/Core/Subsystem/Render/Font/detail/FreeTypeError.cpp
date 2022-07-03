/**
 * @file
 * @date 2022/6/27
 * @author 9chu
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#include "FreeTypeError.hpp"

using namespace std;
using namespace lstg;
using namespace lstg::Subsystem::Render::Font::detail;

const FreeTypeErrorCategory& FreeTypeErrorCategory::GetInstance() noexcept
{
    static const FreeTypeErrorCategory kInstance;
    return kInstance;
}

const char* FreeTypeErrorCategory::name() const noexcept
{
    return "FreeTypeError";
}

std::string FreeTypeErrorCategory::message(int ev) const
{
    auto p = ::FT_Error_String(static_cast<FT_Error>(ev));
    return p ? p : "<unknown>";
}
