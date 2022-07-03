/**
 * @file
 * @date 2021/11/18
 * @author 9chu
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#pragma once
#include <cstdint>
#include <system_error>

namespace lstg::Encoding
{
    /**
     * 编码异常错误码
     */
    enum class EncodingError
    {
        DecodingFailure = 1,
        EncodingFailure = 2,
    };

    /**
     * 错误分类声明
     */
    class EncodingErrorCategory : public std::error_category
    {
    public:
        static const EncodingErrorCategory& GetInstance() noexcept;

    public:
        const char* name() const noexcept override;
        std::string message(int ev) const override;
    };

    inline std::error_code make_error_code(EncodingError err) noexcept
    {
        return {static_cast<int32_t>(err), EncodingErrorCategory::GetInstance()};
    }
}

template <>
struct std::is_error_code_enum<lstg::Encoding::EncodingError> : true_type {};
