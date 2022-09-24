/**
 * @file
 * @date 2022/6/27
 * @author 9chu
 * 此文件为 LuaSTGPlus 项目的一部分，版权与许可声明详见 COPYRIGHT.txt。
 */
#pragma once
#include <system_error>
#include <freetype/freetype.h>

namespace lstg::Subsystem::Render::Font::detail
{
    // 见 FT_Error
    enum class FreeTypeError : int32_t {};

    /**
     * FreeType 错误代码分类
     */
    class FreeTypeErrorCategory :
        public std::error_category
    {
    public:
        static const FreeTypeErrorCategory& GetInstance() noexcept;

    public:
        const char* name() const noexcept override;
        std::string message(int ev) const override;
    };

    inline std::error_code make_error_code(FreeTypeError ec) noexcept
    {
        return { static_cast<int>(ec), lstg::Subsystem::Render::Font::detail::FreeTypeErrorCategory::GetInstance() };
    }
}

template <>
struct std::is_error_code_enum<lstg::Subsystem::Render::Font::detail::FreeTypeError> : std::true_type {};
