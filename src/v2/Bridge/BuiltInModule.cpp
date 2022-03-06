/**
 * @file
 * @author 9chu
 * @date 2022/3/5
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#include <lstg/v2/Bridge/BuiltInModule.hpp>

#include <cmath>

using namespace std;
using namespace lstg;
using namespace lstg::v2::Bridge;

// <editor-fold desc="内置数学库">

double BuiltInModule::Sin(double v) noexcept
{
    return ::sin(v * kDegree2Radian);
}

double BuiltInModule::Cos(double v) noexcept
{
    return ::cos(v * kDegree2Radian);
}

double BuiltInModule::ASin(double v) noexcept
{
    return ::asin(v) * kRadian2Degree;
}

double BuiltInModule::ACos(double v) noexcept
{
    return ::acos(v) * kRadian2Degree;
}

double BuiltInModule::Tan(double v) noexcept
{
    return ::tan(v * kDegree2Radian);
}

double BuiltInModule::ATan(double v) noexcept
{
    return ::atan(v) * kRadian2Degree;
}

double BuiltInModule::ATan2(double y, double x) noexcept
{
    return ::atan2(y, x) * kRadian2Degree;
}

// </editor-fold>
// <editor-fold desc="对象构造函数">

LSTGColor BuiltInModule::NewColor(Subsystem::Script::LuaStack stack) noexcept
{
    LSTGColor ret {};
    if (stack.GetTop() == 1)
    {
        auto argb = luaL_checkinteger(stack, 1);
        ret.r((argb & 0x00FF0000) >> 16);
        ret.g((argb & 0x0000FF00) >> 8);
        ret.b(argb & 0x000000FF);
        ret.a((argb & 0xFF000000) >> 24);
        return ret;
    }

    auto a = luaL_checkinteger(stack, 1);
    auto r = luaL_checkinteger(stack, 2);
    auto g = luaL_checkinteger(stack, 3);
    auto b = luaL_checkinteger(stack, 4);
    ret.r(std::clamp<int>(r, 0, 255));
    ret.g(std::clamp<int>(g, 0, 255));
    ret.b(std::clamp<int>(b, 0, 255));
    ret.a(std::clamp<int>(a, 0, 255));
    return ret;
}

// </editor-fold>
