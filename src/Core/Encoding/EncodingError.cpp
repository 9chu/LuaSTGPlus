/**
 * @file
 * @date 2022/7/3
 * @author 9chu
 * 此文件为 LuaSTGPlus 项目的一部分，版权与许可声明详见 COPYRIGHT.txt。
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
