/**
 * @file
 * @date 2022/3/6
 * @author 9chu
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#pragma once
#include <system_error>

namespace lstg::Subsystem::Render
{
    /**
     * 渲染错误代码
     */
    enum class RenderError
    {
        Ok = 0,
    };

    /**
     * Lua 错误代码分类
     */
    class RenderErrorCategory : public std::error_category
    {
    public:
        static const RenderErrorCategory& GetInstance() noexcept;

    public:
        const char* name() const noexcept override;
        std::string message(int ev) const override;
    };

    inline std::error_code make_error_code(RenderError ec) noexcept
    {
        return { static_cast<int>(ec), RenderErrorCategory::GetInstance() };
    }
}

namespace std
{
    template <>
    struct is_error_code_enum<lstg::Subsystem::Render::RenderError> : true_type {};
}
