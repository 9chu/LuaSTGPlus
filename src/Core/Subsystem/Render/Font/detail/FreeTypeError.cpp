/**
 * @file
 * @date 2022/6/27
 * @author 9chu
 * 此文件为 LuaSTGPlus 项目的一部分，版权与许可声明详见 COPYRIGHT.txt。
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
