/**
 * @file
 * @date 2022/7/9
 * @author 9chu
 * 此文件为 LuaSTGPlus 项目的一部分，版权与许可声明详见 COPYRIGHT.txt。
 */
#include <lstg/Core/Text/IniParsingError.hpp>

#include <cassert>

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
