/**
 * @file
 * @author 9chu
 * @date 2022/3/4
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#include <lstg/v2/Bridge/LSTGColor.hpp>

using namespace std;
using namespace lstg;
using namespace lstg::v2::Bridge;

// <editor-fold desc="LSTGColor">

bool LSTGColor::Equals(const LSTGColor& lhs, const LSTGColor& rhs) noexcept
{
    return lhs == rhs;
}

LSTGColor LSTGColor::Add(const LSTGColor& lhs, const LSTGColor& rhs) noexcept
{
    return LSTGColor {lhs + rhs};
}

LSTGColor LSTGColor::Substract(const LSTGColor& lhs, const LSTGColor& rhs) noexcept
{
    return LSTGColor {lhs - rhs};
}

LSTGColor LSTGColor::Multiply(std::variant<double, const LSTGColor*> lhs, std::variant<double, const LSTGColor*> rhs) noexcept
{
    if (lhs.index() == 0)
    {
        assert(rhs.index() == 1);
        auto f = std::get<0>(lhs);
        auto p = std::get<1>(rhs);
        assert(p);
        return LSTGColor{f * *p};
    }
    if (rhs.index() == 0)
    {
        assert(lhs.index() == 1);
        auto f = std::get<0>(rhs);
        auto p = std::get<1>(lhs);
        assert(p);
        return LSTGColor{*p * f};
    }
    return LSTGColor {*std::get<1>(lhs) * *std::get<1>(rhs)};
}

std::string LSTGColor::ToString(Subsystem::Script::LuaStack st)
{
    return fmt::format("lstg.Color({},{},{},{})", RGBA32Color::a(), RGBA32Color::r(), RGBA32Color::g(), RGBA32Color::b());
}

// </editor-fold>
// <editor-fold desc="LSTGColorModule">

Subsystem::Script::Unpack<int, int, int, int> LSTGColorModule::ToARGB(const LSTGColor& color)
{
    return { color.a(), color.r(), color.g(), color.b() };
}

// </editor-fold>
