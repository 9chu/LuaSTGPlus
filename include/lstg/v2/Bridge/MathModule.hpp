/**
 * @file
 * @author 9chu
 * @date 2022/4/17
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#pragma once
#include <lstg/Core/Subsystem/Script/LuaRead.hpp>
#include <lstg/Core/Subsystem/Script/AutoBridgeHint.hpp>

namespace lstg::v2::Bridge
{
    /**
     * 数学模块
     */
    LSTG_MODULE(lstg, GLOBAL)
    class MathModule
    {
    public:
        static constexpr double kPI = 3.14159265358979323846;
        static constexpr double kRadian2Degree = (180.0 / kPI);
        static constexpr double kDegree2Radian = (1.0 / kRadian2Degree);

        LSTG_METHOD(sin)
        static double Sin(double v) noexcept;

        LSTG_METHOD(cos)
        static double Cos(double v) noexcept;

        LSTG_METHOD(asin)
        static double ASin(double v) noexcept;

        LSTG_METHOD(acos)
        static double ACos(double v) noexcept;

        LSTG_METHOD(tan)
        static double Tan(double v) noexcept;

        LSTG_METHOD(atan)
        static double ATan(double v) noexcept;

        LSTG_METHOD(atan2)
        static double ATan2(double y, double x) noexcept;
    };
}
