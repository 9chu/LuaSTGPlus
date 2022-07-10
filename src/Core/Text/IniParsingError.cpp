/**
 * @file
 * @date 2022/7/9
 * @author 9chu
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#include <lstg/Core/Text/IniParsingError.hpp>

using namespace std;
using namespace lstg;
using namespace lstg::Text;

const IniParsingErrorCategory& IniParsingErrorCategory::GetInstance() noexcept
{
    static IniParsingErrorCategory kInstance;
    return kInstance;
}

const char* IniParsingErrorCategory::name() const noexcept
{
    return "IniParsingError";
}

std::string IniParsingErrorCategory::message(int ev) const
{
    switch (static_cast<IniParsingError>(ev))
    {
        case IniParsingError::Ok:
            return "ok";
        case IniParsingError::SectionNotClosed:
            return "section not closed";
        case IniParsingError::UnexpectedCharacter:
            return "unexpected character";
        default:
            assert(false);
            return "<unknown>";
    }
}
