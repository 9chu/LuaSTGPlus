/**
 * @file
 * @date 2022/7/9
 * @author 9chu
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#pragma once
#include <system_error>

namespace lstg::Text
{
    /**
     * Ini 解析错误
     */
    enum class IniParsingError
    {
        Ok = 0,
        SectionNotClosed = 1,
        UnexpectedCharacter = 2,
    };

    /**
     * Ini 解析错误代码分类
     */
    class IniParsingErrorCategory : public std::error_category
    {
    public:
        static const IniParsingErrorCategory& GetInstance() noexcept;

    public:
        const char* name() const noexcept override;
        std::string message(int ev) const override;
    };

    inline std::error_code make_error_code(IniParsingError ec) noexcept
    {
        return { static_cast<int>(ec), IniParsingErrorCategory::GetInstance() };
    }
}

template <>
struct std::is_error_code_enum<lstg::Text::IniParsingError> : true_type {};
