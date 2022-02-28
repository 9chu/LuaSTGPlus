/**
 * @file
 * @author 9chu
 * @date 2022/2/26
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#pragma once
#include <system_error>

namespace lstg::Subsystem::VFS::detail
{
    /**
     * ZIP 读错误
     */
    enum class ZipFileReadError
    {
        Ok = 0,
        EOCDNotFound,
        UnexpectedEndOfStream,
        BadEOCDSignature,
        BadEOCD64Signature,
        BadEOCD64LocatorSignature,
        BadEOCDSize,
        MultiDiskFormatNotSupported,
        CDOffsetTooBig,
        CDSizeTooBig,
        BadCDFileHeaderSignature,
        CDLocationInvalid,
        BadLocalFileHeaderSignature,
        DataDescriptorNotSupported,
        CompressedPatchedDataNotSupported,
        StrongEncryptionNotSupported,
        CentralDirectoryEncryptionNotSupported,
        CompressionMethodNotSupported,
        BadCDEntrySize,
        MissingZip64ExtraField,
        BadPassword,
        DuplicatedFile,
    };

    /**
     * ZIP 读错误分类
     */
    class ZipFileReadErrorCategory :
        public std::error_category
    {
    public:
        static const ZipFileReadErrorCategory& GetInstance() noexcept;

    public:
        const char* name() const noexcept override;
        std::string message(int ev) const override;
    };

    inline std::error_code make_error_code(ZipFileReadError ec) noexcept
    {
        return { static_cast<int>(ec), ZipFileReadErrorCategory::GetInstance() };
    }
}

namespace std
{
    template <>
    struct is_error_code_enum<lstg::Subsystem::VFS::detail::ZipFileReadError> : true_type {};
}
