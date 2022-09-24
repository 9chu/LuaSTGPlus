/**
 * @file
 * @author 9chu
 * @date 2022/4/17
 * 此文件为 LuaSTGPlus 项目的一部分，版权与许可声明详见 COPYRIGHT.txt。
 */
#include <lstg/v2/Bridge/MathModule.hpp>

#include <cmath>

using namespace std;
using namespace lstg;
using namespace lstg::v2::Bridge;

double MathModule::Sin(double v) noexcept
{
    return ::sin(v * kDegree2Radian);
}

double MathModule::Cos(double v) noexcept
{
    return ::cos(v * kDegree2Radian);
}

double MathModule::ASin(double v) noexcept
{
    return ::asin(v) * kRadian2Degree;
}

double MathModule::ACos(double v) noexcept
{
    return ::acos(v) * kRadian2Degree;
}

double MathModule::Tan(double v) noexcept
{
    return ::tan(v * kDegree2Radian);
}

double MathModule::ATan(double v) noexcept
{
    return ::atan(v) * kRadian2Degree;
}

double MathModule::ATan2(double y, double x) noexcept
{
    return ::atan2(y, x) * kRadian2Degree;
}
