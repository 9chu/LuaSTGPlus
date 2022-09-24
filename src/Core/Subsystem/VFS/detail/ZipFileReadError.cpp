/**
 * @file
 * @author 9chu
 * @date 2022/2/26
 * 此文件为 LuaSTGPlus 项目的一部分，版权与许可声明详见 COPYRIGHT.txt。
 */
#include "ZipFileReadError.hpp"

using namespace std;
using namespace lstg;
using namespace lstg::Subsystem::VFS::detail;

const ZipFileReadErrorCategory& ZipFileReadErrorCategory::GetInstance() noexcept
{
    static const ZipFileReadErrorCategory kInstance;
    return kInstance;
}

const char* ZipFileReadErrorCategory::name() const noexcept
{
    return "ZipFileReadError";
}

std::string ZipFileReadErrorCategory::message(int ev) const
{
    switch (static_cast<ZipFileReadError>(ev))
    {
        case ZipFileReadError::Ok:
            return "ok";
        case ZipFileReadError::UnexpectedEndOfStream:
            return "unexpected end of stream";
        case ZipFileReadError::BadEOCDSignature:
            return "bad end of central directory signature";
        case ZipFileReadError::BadEOCD64Signature:
            return "bad zip64 end of central directory signature";
        case ZipFileReadError::BadEOCD64LocatorSignature:
            return "bad zip64 end of central directory locator signature";
        case ZipFileReadError::BadEOCDSize:
            return "bad end of central directory size";
        case ZipFileReadError::MultiDiskFormatNotSupported:
            return "multi part zip file is not supported yet";
        case ZipFileReadError::CDOffsetTooBig:
            return "the offset of central directory is too big";
        case ZipFileReadError::CDSizeTooBig:
            return "the size of central directory is too big";
        case ZipFileReadError::BadCDFileHeaderSignature:
            return "bad central directory file header signature";
        case ZipFileReadError::CDLocationInvalid:
            return "central directory location is invalid";
        case ZipFileReadError::BadLocalFileHeaderSignature:
            return "bad local file header signature";
        case ZipFileReadError::DataDescriptorNotSupported:
            return "data descriptor is not supported yet";
        case ZipFileReadError::CompressedPatchedDataNotSupported:
            return "compressed patched data is not supported yet";
        case ZipFileReadError::StrongEncryptionNotSupported:
            return "strong encryption method is not supported yet";
        case ZipFileReadError::CentralDirectoryEncryptionNotSupported:
            return "central directory encryption is not supported yet";
        case ZipFileReadError::CompressionMethodNotSupported:
            return "specific compression method is not supported yet";
        case ZipFileReadError::BadCDEntrySize:
            return "bad entry size in central directory";
        case ZipFileReadError::MissingZip64ExtraField:
            return "zip64 extra field is not found";
        case ZipFileReadError::BadPassword:
            return "bad password or corrupt file";
        default:
            return "<unknown>";
    }
}
