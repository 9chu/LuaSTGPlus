/**
 * @file
 * @date 2022/7/3
 * @author 9chu
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#include <lstg/Core/Encoding/EncodingError.hpp>

using namespace std;
using namespace lstg;
using namespace lstg::Encoding;

const EncodingErrorCategory& EncodingErrorCategory::GetInstance() noexcept
{
    static EncodingErrorCategory kInstance;
    return kInstance;
}

const char* EncodingErrorCategory::name() const noexcept
{
    return "EncodingError";
}

std::string EncodingErrorCategory::message(int ev) const
{
    switch (static_cast<EncodingError>(ev))
    {
        case EncodingError::DecodingFailure:
            return "Decoding failure";
        case EncodingError::EncodingFailure:
            return "Encoding failure";
        default:
            return "<unknown>";
    }
}
