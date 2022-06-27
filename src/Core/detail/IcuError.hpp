/**
 * @file
 * @date 2022/6/11
 * @author 9chu
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#pragma once
#include <system_error>
#include <unicode/utypes.h>

namespace lstg::detail
{
    /**
     * ICU 错误代码分类
     */
    class IcuErrorCategory :
        public std::error_category
    {
    public:
        static const IcuErrorCategory& GetInstance() noexcept;

    public:
        const char* name() const noexcept override;
        std::string message(int ev) const override;
    };
}

inline std::error_code make_error_code(UErrorCode ec) noexcept
{
    return { static_cast<int>(ec), lstg::detail::IcuErrorCategory::GetInstance() };
}

template <>
struct std::is_error_code_enum<UErrorCode> : std::true_type {};
