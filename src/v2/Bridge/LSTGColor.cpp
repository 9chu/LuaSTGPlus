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

bool LSTGColor::Equals(LSTGColor* lhs, LSTGColor* rhs) noexcept
{
    if (lhs == nullptr || rhs == nullptr)
        return false;
    return *lhs == *rhs;
}

LSTGColor LSTGColor::Add(LSTGColor* lhs, LSTGColor* rhs) noexcept
{
    if (rhs == nullptr)
    {
        assert(lhs);
        return *lhs;
    }
    if (lhs == nullptr)
    {
        assert(rhs);
        return *rhs;
    }
    return *lhs + *rhs;
}

LSTGColor LSTGColor::Substract(LSTGColor* lhs, LSTGColor* rhs) noexcept
{
    if (rhs == nullptr)
    {
        assert(lhs);
        return *lhs;
    }
    if (lhs == nullptr)
    {
        assert(rhs);
        return *rhs;
    }
    return *lhs - *rhs;
}

LSTGColor LSTGColor::Multiply(std::variant<double, LSTGColor*> lhs, std::variant<double, LSTGColor*> rhs) noexcept
{
    if (lhs.index() == 0)
    {
        assert(rhs.index() == 1);
        auto f = std::get<0>(lhs);
        auto p = std::get<1>(rhs);
        assert(p);
        return f * *p;
    }
    if (rhs.index() == 0)
    {
        assert(lhs.index() == 1);
        auto f = std::get<0>(rhs);
        auto p = std::get<1>(lhs);
        assert(p);
        return *p * f;
    }
    return *std::get<1>(lhs) * *std::get<1>(rhs);
}

std::string LSTGColor::ToString(LSTGColor* self)
{
    return fmt::format("lstg.Color({},{},{},{})", self->RGBA32[3], self->RGBA32[0], self->RGBA32[1], self->RGBA32[2]);
}

bool lstg::v2::Bridge::operator==(LSTGColor a, LSTGColor b) noexcept
{
    return ::memcmp(a.RGBA32, b.RGBA32, sizeof(a.RGBA32)) == 0;
}

LSTGColor lstg::v2::Bridge::operator+(LSTGColor a, LSTGColor b) noexcept
{
    LSTGColor ret {};
    ret.RGBA32[0] = static_cast<uint8_t>(std::min<int>(255, a.RGBA32[0] + b.RGBA32[0]));
    ret.RGBA32[1] = static_cast<uint8_t>(std::min<int>(255, a.RGBA32[1] + b.RGBA32[1]));
    ret.RGBA32[2] = static_cast<uint8_t>(std::min<int>(255, a.RGBA32[2] + b.RGBA32[2]));
    ret.RGBA32[3] = static_cast<uint8_t>(std::min<int>(255, a.RGBA32[3] + b.RGBA32[3]));
    return ret;
}

LSTGColor lstg::v2::Bridge::operator-(LSTGColor a, LSTGColor b) noexcept
{
    LSTGColor ret {};
    ret.RGBA32[0] = static_cast<uint8_t>(std::max<int>(0, a.RGBA32[0] - b.RGBA32[0]));
    ret.RGBA32[1] = static_cast<uint8_t>(std::max<int>(0, a.RGBA32[1] - b.RGBA32[1]));
    ret.RGBA32[2] = static_cast<uint8_t>(std::max<int>(0, a.RGBA32[2] - b.RGBA32[2]));
    ret.RGBA32[3] = static_cast<uint8_t>(std::max<int>(0, a.RGBA32[3] - b.RGBA32[3]));
    return ret;
}

LSTGColor lstg::v2::Bridge::operator*(LSTGColor a, LSTGColor b) noexcept
{
    LSTGColor ret {};
    ret.RGBA32[0] = static_cast<uint8_t>(std::clamp<double>(a.RGBA32[0] * (b.RGBA32[0] / 255.), 0, 255));
    ret.RGBA32[1] = static_cast<uint8_t>(std::clamp<double>(a.RGBA32[1] * (b.RGBA32[1] / 255.), 0, 255));
    ret.RGBA32[2] = static_cast<uint8_t>(std::clamp<double>(a.RGBA32[2] * (b.RGBA32[2] / 255.), 0, 255));
    ret.RGBA32[3] = static_cast<uint8_t>(std::clamp<double>(a.RGBA32[3] * (b.RGBA32[3] / 255.), 0, 255));
    return ret;
}

LSTGColor lstg::v2::Bridge::operator*(double f, LSTGColor c) noexcept
{
    double r = f * c.RGBA32[0];
    double g = f * c.RGBA32[1];
    double b = f * c.RGBA32[2];
    double a = f * c.RGBA32[3];

    LSTGColor ret {};
    ret.RGBA32[0] = static_cast<uint8_t>(std::clamp<double>(r, 0, 255));
    ret.RGBA32[1] = static_cast<uint8_t>(std::clamp<double>(g, 0, 255));
    ret.RGBA32[2] = static_cast<uint8_t>(std::clamp<double>(b, 0, 255));
    ret.RGBA32[3] = static_cast<uint8_t>(std::clamp<double>(a, 0, 255));
    return ret;
}

LSTGColor lstg::v2::Bridge::operator*(LSTGColor c, double f) noexcept
{
    return f * c;
}

// </editor-fold>
// <editor-fold desc="LSTGColorModule">

Subsystem::Script::Unpack<int, int, int, int> LSTGColorModule::ToARGB(LSTGColor* color)
{
    assert(color);
    return { color->RGBA32[3], color->RGBA32[0], color->RGBA32[1], color->RGBA32[2] };
}

// </editor-fold>
