/**
 * @file
 * @author 9chu
 * @date 2022/3/5
 * 此文件为 LuaSTGPlus 项目的一部分，版权与许可声明详见 COPYRIGHT.txt。
 */
#include <lstg/v2/Bridge/LSTGRandomizer.hpp>

using namespace std;
using namespace lstg;
using namespace lstg::v2::Bridge;

void LSTGRandomizer::Seed(uint32_t v) noexcept
{
    Randomizer::SetSeed(v);
}

uint32_t LSTGRandomizer::GetSeed() const noexcept
{
    return Randomizer::GetSeed();
}

int32_t LSTGRandomizer::Int(int32_t low, int32_t upper) noexcept
{
    if (low >= upper)
        return upper;

    auto range = static_cast<float>(upper - low + 1);
    // m_stRandomizer.Next(range)  // Next采用丢弃采样策略，可能会比较慢
    // 使用 float 会造成取值范围不能覆盖 int32_t，各有利弊
    auto ret = static_cast<int32_t>(::floor(Randomizer::NextFloat() * range + static_cast<float>(low)));
    assert(low <= ret && ret <= upper);
    return ret;
}

double LSTGRandomizer::Float(double low, double upper) noexcept
{
    if (low >= upper)
        return upper;

    auto range = static_cast<float>(upper - low);
    auto ret = (Randomizer::NextFloat() * range) + low;
    assert(low <= ret && ret < upper);
    return ret;
}

int32_t LSTGRandomizer::Sign() noexcept
{
    auto r = Randomizer::Next();
    return static_cast<int32_t>((r & 1u) * 2u) - 1;
}

std::string LSTGRandomizer::ToString() const
{
    return "lstgRandomizer";
}
