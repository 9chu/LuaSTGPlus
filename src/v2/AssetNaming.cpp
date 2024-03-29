/**
 * @file
 * @date 2022/6/5
 * @author 9chu
 * 此文件为 LuaSTGPlus 项目的一部分，版权与许可声明详见 COPYRIGHT.txt。
 */
#include <lstg/v2/AssetNaming.hpp>

#include <cassert>
#include <fmt/format.h>

using namespace std;
using namespace lstg;
using namespace lstg::v2;

static const size_t kAssetPrefixLength = 4;

static const char* kAssetPrefix[] = {
    "???$",
    "tex$",  // Texture = 1
    "img$",  // Image = 2
    "ani$",  // Animation = 3
    "bgm$",  // Music = 4
    "snd$",  // Sound = 5
    "par$",  // Particle = 6
    "txf$",  // TexturedFont = 7
    "ttf$",  // TrueTypeFont = 8,
    "lfx$",  // Effect = 9,
};

const char* lstg::v2::AssetTypeToPrefix(AssetTypes t) noexcept
{
    auto idx = static_cast<uint32_t>(t);
    if (idx >= std::extent_v<decltype(kAssetPrefix)>)
        idx = 0;
    return kAssetPrefix[idx];
}

bool lstg::v2::IsAssetNameMatchType(string_view name, AssetTypes t) noexcept
{
    auto prefix = AssetTypeToPrefix(t);
    if (name.length() >= kAssetPrefixLength && name.substr(0, kAssetPrefixLength) == prefix)
        return true;
    return false;
}

string lstg::v2::MakeFullAssetName(AssetTypes t, string_view name)
{
    auto prefix = AssetTypeToPrefix(t);
    return fmt::format("{}{}", prefix, name);
}

Result<void> lstg::v2::MakeFullAssetName(std::string& out, AssetTypes t, std::string_view name) noexcept
{
    auto prefix = AssetTypeToPrefix(t);
    try
    {
        out.clear();
        fmt::format_to(std::back_inserter(out), "{}{}", prefix, name);
        return {};
    }
    catch (...)  // bad_alloc
    {
        return make_error_code(errc::not_enough_memory);
    }
}

string lstg::v2::ExtractAssetName(string_view name)
{
    assert(name.length() >= kAssetPrefixLength);
    return string { name.substr(kAssetPrefixLength) };
}
