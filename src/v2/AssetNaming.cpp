/**
 * @file
 * @date 2022/6/5
 * @author 9chu
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#include <lstg/v2/AssetNaming.hpp>

#include <cassert>
#include <fmt/format.h>

using namespace std;
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

string lstg::v2::ExtractAssetName(string_view name)
{
    assert(name.length() >= kAssetPrefixLength);
    return string { name.substr(kAssetPrefixLength) };
}
