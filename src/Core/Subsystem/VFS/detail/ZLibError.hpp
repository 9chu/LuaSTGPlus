/**
 * @file
 * @date 2022/2/28
 * @author 9chu
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#pragma once
#include <system_error>

namespace lstg::Subsystem::VFS::detail
{
    /**
     * ZLIB 错误
     */
    enum class ZLibError
    {
        Ok = 0,
        StreamEnd = 1,  // Z_STREAM_END
        NeedDict = 2,  // Z_NEED_DICT
        GeneralError = -1,  // Z_ERRNO
        StreamError = -2,  // Z_STREAM_ERROR
        DataError = -3,  // Z_DATA_ERROR
        MemoryError = -4,  // Z_MEM_ERROR
        BufferError = -5,  // Z_BUF_ERROR
        VersionError = -6,  // Z_VERSION_ERROR
    };

    /**
     * ZLIB 错误分类
     */
    class ZLibErrorCategory :
        public std::error_category
    {
    public:
        static const ZLibErrorCategory& GetInstance() noexcept;

    public:
        const char* name() const noexcept override;
        std::string message(int ev) const override;
    };

    inline std::error_code make_error_code(ZLibError ec) noexcept
    {
        return { static_cast<int>(ec), ZLibErrorCategory::GetInstance() };
    }
}

namespace std
{
    template <>
    struct is_error_code_enum<lstg::Subsystem::VFS::detail::ZLibError> : true_type {};
}
