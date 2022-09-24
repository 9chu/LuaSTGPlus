/**
 * @file
 * @author 9chu
 * @date 2022/3/4
 * 此文件为 LuaSTGPlus 项目的一部分，版权与许可声明详见 COPYRIGHT.txt。
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
    return {lhs + rhs};
}

LSTGColor LSTGColor::Substract(const LSTGColor& lhs, const LSTGColor& rhs) noexcept
{
    return {lhs - rhs};
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

Subsystem::Script::Unpack<int, int, int, int> LSTGColor::ToARGB() const noexcept
{
    return { ColorRGBA32::a(), ColorRGBA32::r(), ColorRGBA32::g(), ColorRGBA32::b() };
}

std::string LSTGColor::ToString() const
{
    return fmt::format("lstg.Color({},{},{},{})", ColorRGBA32::a(), ColorRGBA32::r(), ColorRGBA32::g(), ColorRGBA32::b());
}

// </editor-fold>
