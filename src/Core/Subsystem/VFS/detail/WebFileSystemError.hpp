/**
 * @file
 * @date 2022/3/5
 * @author 9chu
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#pragma once
#include <system_error>

namespace lstg::Subsystem::VFS::detail
{
    /**
     * 网络文件系统错误
     */
    enum class WebFileSystemError
    {
        Ok = 0,
        ApiError = 1,
        HttpError = 2,
        HttpHeaderFieldTooLong = 3,
        InvalidHttpHeader = 4,
        InvalidHttpDateTime = 5,
    };

    class WebFileSystemErrorCategory : public std::error_category
    {
    public:
        static const WebFileSystemErrorCategory& GetInstance() noexcept;

    public:
        const char* name() const noexcept override;
        std::string message(int ev) const override;
    };

    inline std::error_code make_error_code(WebFileSystemError ec) noexcept
    {
        return { static_cast<int>(ec), detail::WebFileSystemErrorCategory::GetInstance() };
    }
}

namespace std
{
    template <>
    struct is_error_code_enum<lstg::Subsystem::VFS::detail::WebFileSystemError> : true_type {};
}
