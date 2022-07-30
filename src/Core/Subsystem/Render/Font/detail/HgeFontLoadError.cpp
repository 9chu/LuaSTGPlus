/**
 * @file
 * @date 2022/7/9
 * @author 9chu
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#include "HgeFontLoadError.hpp"

#include <cassert>

using namespace std;
using namespace lstg;
using namespace lstg::Subsystem::Render::Font::detail;

const HgeFontLoadErrorCategory& HgeFontLoadErrorCategory::GetInstance() noexcept
{
    static HgeFontLoadErrorCategory kInstance;
    return kInstance;
}

const char* HgeFontLoadErrorCategory::name() const noexcept
{
    return "HgeFontLoadError";
}

std::string HgeFontLoadErrorCategory::message(int ev) const
{
    switch (static_cast<HgeFontLoadError>(ev))
    {
        case HgeFontLoadError::Ok:
            return "ok";
        case HgeFontLoadError::DuplicatedFontSection:
            return "duplicated font section";
        case HgeFontLoadError::DuplicatedBitmap:
            return "duplicated bitmap";
        case HgeFontLoadError::UnexpectedCharacter:
            return "unexpected character";
        case HgeFontLoadError::Utf8DecodeError:
            return "unicode decoding error";
        case HgeFontLoadError::InvalidValue:
            return "invalid value";
        case HgeFontLoadError::MissingBitmap:
            return "missing bitmap";
        default:
            assert(false);
            return "<unknown>";
    }
}
