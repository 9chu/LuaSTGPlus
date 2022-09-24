/**
 * @file
 * @date 2022/7/9
 * @author 9chu
 * 此文件为 LuaSTGPlus 项目的一部分，版权与许可声明详见 COPYRIGHT.txt。
 */
#pragma once
#include <system_error>

namespace lstg::Subsystem::Render::Font::detail
{
    /**
     * HGE 字体加载错误
     */
    enum class HgeFontLoadError
    {
        Ok = 0,
        DuplicatedFontSection = 1,
        DuplicatedBitmap = 2,
        UnexpectedCharacter = 3,
        Utf8DecodeError = 4,
        InvalidValue = 5,
        MissingBitmap = 6,
    };

    /**
     * HGE 字体加载错误代码分类
     */
    class HgeFontLoadErrorCategory : public std::error_category
    {
    public:
        static const HgeFontLoadErrorCategory& GetInstance() noexcept;

    public:
        const char* name() const noexcept override;
        std::string message(int ev) const override;
    };

    inline std::error_code make_error_code(HgeFontLoadError ec) noexcept
    {
        return { static_cast<int>(ec), HgeFontLoadErrorCategory::GetInstance() };
    }
}

template <>
struct std::is_error_code_enum<lstg::Subsystem::Render::Font::detail::HgeFontLoadError> : std::true_type {};
