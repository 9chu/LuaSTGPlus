/**
 * @file
 * @date 2022/6/11
 * @author 9chu
 * 此文件为 LuaSTGPlus 项目的一部分，版权与许可声明详见 COPYRIGHT.txt。
 */
#include "IcuError.hpp"

using namespace std;
using namespace lstg;
using namespace lstg::detail;

const IcuErrorCategory& IcuErrorCategory::GetInstance() noexcept
{
    static const IcuErrorCategory kInstance;
    return kInstance;
}

const char* IcuErrorCategory::name() const noexcept
{
    return "IcuError";
}

std::string IcuErrorCategory::message(int ev) const
{
    return ::u_errorName(static_cast<UErrorCode>(ev));
}
