/**
 * @file
 * @date 2022/9/13
 * @author 9chu
 * 此文件为 LuaSTGPlus 项目的一部分，版权与许可声明详见 COPYRIGHT.txt。
 */
#pragma once
#include <system_error>

namespace lstg::Subsystem::Audio::detail
{
    /**
     * SDL_Sound 错误码
     */
    enum class SDLSoundErrorCodes
    {
        Ok = 0,
        Unknown,
        IsInitialized,
        NotInitialized,
        InvalidArgument,
        OutOfMemory,
        NotSupported,
        UnsupportedFormat,
        NotHandle,
        NoSuchFile,
        PastEOF,
        IOError,
        CompressionError,
        PrevError,
        PrevEOF,
        NotSeekable,
    };

    /**
     * SDL_Sound 错误代码分类
     */
    class SDLSoundErrorCategory :
        public std::error_category
    {
    public:
        static const SDLSoundErrorCategory& GetInstance() noexcept;

    public:
        const char* name() const noexcept override;
        std::string message(int ev) const override;
    };

    inline std::error_code make_error_code(SDLSoundErrorCodes ec) noexcept
    {
        return { static_cast<int>(ec), SDLSoundErrorCategory::GetInstance() };
    }

    /**
     * 从错误字符串构造错误码
     * @param str 字符串
     * @return 错误码
     */
    SDLSoundErrorCodes FromErrorString(const char* str) noexcept;
}

template <>
struct std::is_error_code_enum<lstg::Subsystem::Audio::detail::SDLSoundErrorCodes> : std::true_type {};
