/**
 * @file
 * @author 9chu
 * @date 2022/2/26
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#pragma once
#include <cstdint>
#include <lstg/Core/Subsystem/VFS/IStream.hpp>
#include "ZipFileReadError.hpp"

namespace lstg::Subsystem::VFS::detail
{
    // see https://pkware.cachefly.net/webdocs/casestudies/APPNOTE.TXT
    // see https://github.com/zlib-ng/minizip-ng

    // 4.3.16  End of central directory record:
    struct ZipEndOfCentralDirectoryRecord
    {
        static const uint32_t kSignature = 0x06054b50;

        uint32_t Signature = 0;
        uint16_t DiskNum = 0;  // Number of this disk (or 0xffff for ZIP64)
        uint16_t DiskNumWithCentralDirectory = 0;  // Disk where central directory starts (or 0xffff for ZIP64)
        uint16_t EntryNum = 0;  // Number of central directory records on this disk (or 0xffff for ZIP64)
        uint16_t EntryNumCentralDirectory = 0;  // Total number of central directory records (or 0xffff for ZIP64)
        uint32_t SizeOfCentralDirectory = 0;  // Size of central directory (bytes) (or 0xffffffff for ZIP64)
        uint32_t OffsetOfCentralDirectory = 0;  // Offset of start of central directory,
                                                // relative to start of archive (or 0xffffffff for ZIP64)
        std::string Comment;
    };

    inline Result<void> Read(ZipEndOfCentralDirectoryRecord& out, IStream* stream, LittleEndianTag = {}) noexcept
    {
        Result<void> ret;
        uint16_t commentLength = 0;

        if (!(ret = Read(out.Signature, stream, LittleEndianTag{})))
            return ret;
        if (out.Signature != ZipEndOfCentralDirectoryRecord::kSignature)
            return make_error_code(ZipFileReadError::BadEOCDSignature);
        if (!(ret = Read(out.DiskNum, stream, LittleEndianTag{})))
            return ret;
        if (!(ret = Read(out.DiskNumWithCentralDirectory, stream, LittleEndianTag{})))
            return ret;
        if (!(ret = Read(out.EntryNum, stream, LittleEndianTag{})))
            return ret;
        if (!(ret = Read(out.EntryNumCentralDirectory, stream, LittleEndianTag{})))
            return ret;
        if (!(ret = Read(out.SizeOfCentralDirectory, stream, LittleEndianTag{})))
            return ret;
        if (!(ret = Read(out.OffsetOfCentralDirectory, stream, LittleEndianTag{})))
            return ret;
        if (!(ret = Read(commentLength, stream, LittleEndianTag{})))
            return ret;
        try
        {
            out.Comment.resize(commentLength);
        }
        catch (...)  // bad_alloc
        {
            return make_error_code(std::errc::not_enough_memory);
        }
        auto len = stream->Read(reinterpret_cast<uint8_t*>(out.Comment.data()), commentLength);
        if (!len)
            return len.GetError();
        if (*len != commentLength)
            return make_error_code(ZipFileReadError::UnexpectedEndOfStream);
        return {};
    }

    // 4.3.14  Zip64 end of central directory record
    struct Zip64EndOfCentralDirectoryRecord
    {
        static const uint32_t kSignature = 0x06064b50;
        static const uint32_t kLeadingBytes = 12;
        static const uint32_t kSizeWithOutComment = 56;

        uint32_t Signature = 0;
        uint64_t SizeOfEOCD = 0;
        uint16_t Version = 0;
        uint16_t VersionNeeded = 0;
        uint32_t DiskNum = 0;
        uint32_t DiskNumWithCentralDirectory = 0;
        uint64_t EntryNum = 0;
        uint64_t EntryNumCentralDirectory = 0;
        uint64_t SizeOfCentralDirectory = 0;
        uint64_t OffsetOfCentralDirectory = 0;
        std::string Comment;
    };

    inline Result<void> Read(Zip64EndOfCentralDirectoryRecord& out, IStream* stream, LittleEndianTag = {}) noexcept
    {
        Result<void> ret;

        if (!(ret = Read(out.Signature, stream, LittleEndianTag{})))
            return ret;
        if (out.Signature != Zip64EndOfCentralDirectoryRecord::kSignature)
            return make_error_code(ZipFileReadError::BadEOCD64Signature);
        if (!(ret = Read(out.SizeOfEOCD, stream, LittleEndianTag{})))
            return ret;
        // 4.3.14.1 The value stored into the "size of zip64 end of central
        //          directory record" SHOULD be the size of the remaining
        //          record and SHOULD NOT include the leading 12 bytes.
        if (out.SizeOfEOCD + Zip64EndOfCentralDirectoryRecord::kLeadingBytes < Zip64EndOfCentralDirectoryRecord::kSizeWithOutComment)
            return make_error_code(ZipFileReadError::BadEOCDSize);
        if (!(ret = Read(out.Version, stream, LittleEndianTag{})))
            return ret;
        if (!(ret = Read(out.VersionNeeded, stream, LittleEndianTag{})))
            return ret;
        if (!(ret = Read(out.DiskNum, stream, LittleEndianTag{})))
            return ret;
        if (!(ret = Read(out.DiskNumWithCentralDirectory, stream, LittleEndianTag{})))
            return ret;
        if (!(ret = Read(out.EntryNum, stream, LittleEndianTag{})))
            return ret;
        if (!(ret = Read(out.EntryNumCentralDirectory, stream, LittleEndianTag{})))
            return ret;
        if (!(ret = Read(out.SizeOfCentralDirectory, stream, LittleEndianTag{})))
            return ret;
        if (!(ret = Read(out.OffsetOfCentralDirectory, stream, LittleEndianTag{})))
            return ret;
        size_t commentLength = out.SizeOfEOCD + Zip64EndOfCentralDirectoryRecord::kLeadingBytes -
            Zip64EndOfCentralDirectoryRecord::kSizeWithOutComment;
        try
        {
            out.Comment.resize(commentLength);
        }
        catch (...)  // bad_alloc
        {
            return make_error_code(std::errc::not_enough_memory);
        }
        auto len = stream->Read(reinterpret_cast<uint8_t*>(out.Comment.data()), commentLength);
        if (!len)
            return len.GetError();
        if (*len != commentLength)
            return make_error_code(ZipFileReadError::UnexpectedEndOfStream);
        return {};
    }

    // 4.3.15 Zip64 end of central directory locator
    struct Zip64EndOfCentralDirectoryLocator
    {
        static const uint32_t kSignature = 0x07064b50;
        static const uint32_t kSize = 20;

        uint32_t Signature = 0;
        uint32_t DiskNumWithEndOfCentralDirectory = 0;
        uint64_t RelativeOffsetOfEndOfCentralDirectory = 0;
        uint32_t DiskNum = 0;
    };

    inline Result<void> Read(Zip64EndOfCentralDirectoryLocator& out, IStream* stream, LittleEndianTag = {}) noexcept
    {
        Result<void> ret;

        if (!(ret = Read(out.Signature, stream, LittleEndianTag{})))
            return ret;
        if (out.Signature != Zip64EndOfCentralDirectoryLocator::kSignature)
            return make_error_code(ZipFileReadError::BadEOCD64LocatorSignature);
        if (!(ret = Read(out.DiskNumWithEndOfCentralDirectory, stream, LittleEndianTag{})))
            return ret;
        if (!(ret = Read(out.RelativeOffsetOfEndOfCentralDirectory, stream, LittleEndianTag{})))
            return ret;
        if (!(ret = Read(out.DiskNum, stream, LittleEndianTag{})))
            return ret;
        return {};
    }

    // 4.3.7  Local file header
    struct ZipLocalFileHeader
    {
        static const uint32_t kSignature = 0x04034b50;
        static const size_t kSize = 30;

        uint32_t Signature = 0;
        uint16_t VersionNeeded = 0;
        uint16_t Flags = 0;
        uint16_t CompressionMethod = 0;
        uint16_t LastModFileTime = 0;
        uint16_t LastModFileDate = 0;
        uint32_t Crc32 = 0;
        uint32_t CompressedSize = 0;  // Compressed size (or 0xffffffff for ZIP64)
        uint32_t UncompressedSize = 0;  // Uncompressed size (or 0xffffffff for ZIP64)
        // uint16_t FileNameLength = 0;
        // uint16_t ExtraFieldLength = 0;
        std::string FileName;
        std::string ExtraField;
    };

    inline Result<void> Read(ZipLocalFileHeader& out, IStream* stream, LittleEndianTag = {}) noexcept
    {
        Result<void> ret;

        uint16_t fileNameLength = 0;
        uint16_t extraFieldLength = 0;

        if (!(ret = Read(out.Signature, stream, LittleEndianTag{})))
            return ret;
        if (out.Signature != ZipLocalFileHeader::kSignature)
            return make_error_code(ZipFileReadError::BadLocalFileHeaderSignature);
        if (!(ret = Read(out.VersionNeeded, stream, LittleEndianTag{})))
            return ret;
        if (!(ret = Read(out.Flags, stream, LittleEndianTag{})))
            return ret;
        if (!(ret = Read(out.CompressionMethod, stream, LittleEndianTag{})))
            return ret;
        if (!(ret = Read(out.LastModFileTime, stream, LittleEndianTag{})))
            return ret;
        if (!(ret = Read(out.LastModFileDate, stream, LittleEndianTag{})))
            return ret;
        if (!(ret = Read(out.Crc32, stream, LittleEndianTag{})))
            return ret;
        if (!(ret = Read(out.CompressedSize, stream, LittleEndianTag{})))
            return ret;
        if (!(ret = Read(out.UncompressedSize, stream, LittleEndianTag{})))
            return ret;
        if (!(ret = Read(fileNameLength, stream, LittleEndianTag{})))
            return ret;
        if (!(ret = Read(extraFieldLength, stream, LittleEndianTag{})))
            return ret;
        try
        {
            out.FileName.resize(fileNameLength);
            out.ExtraField.resize(extraFieldLength);
        }
        catch (...)  // bad_alloc
        {
            return make_error_code(std::errc::not_enough_memory);
        }
        auto len = stream->Read(reinterpret_cast<uint8_t*>(out.FileName.data()), fileNameLength);
        if (!len)
            return len.GetError();
        if (*len != fileNameLength)
            return make_error_code(ZipFileReadError::UnexpectedEndOfStream);

        len = stream->Read(reinterpret_cast<uint8_t*>(out.ExtraField.data()), extraFieldLength);
        if (!len)
            return len.GetError();
        if (*len != extraFieldLength)
            return make_error_code(ZipFileReadError::UnexpectedEndOfStream);
        return {};
    }

    inline Result<void> SkipZipLocalFileHeader(IStream* stream) noexcept
    {
        Result<void> ret;
        uint32_t signature = 0;
        uint16_t fileNameLength = 0;
        uint16_t extraFieldLength = 0;

        if (!(ret = Read(signature, stream, LittleEndianTag{})))
            return ret;
        if (signature != ZipLocalFileHeader::kSignature)
            return make_error_code(ZipFileReadError::BadLocalFileHeaderSignature);

        ret = stream->Seek(ZipLocalFileHeader::kSize - 4 - 4, StreamSeekOrigins::Current);
        if (!ret)
            return ret.GetError();

        if (!(ret = Read(fileNameLength, stream, LittleEndianTag{})))
            return ret;
        if (!(ret = Read(extraFieldLength, stream, LittleEndianTag{})))
            return ret;

        ret = stream->Seek(fileNameLength + extraFieldLength, StreamSeekOrigins::Current);
        if (!ret)
            return ret.GetError();
        return {};
    }

    // 4.3.12  Central directory structure
    struct ZipCentralDirectoryFileHeader
    {
        static const uint32_t kSignature = 0x02014b50;

        uint32_t Signature = 0;
        uint16_t Version = 0;
        uint16_t VersionNeeded = 0;
        uint16_t Flags = 0;
        uint16_t CompressionMethod = 0;
        uint16_t LastModFileTime = 0;
        uint16_t LastModFileDate = 0;
        uint32_t Crc32 = 0;
        uint32_t CompressedSize = 0;  // Compressed size (or 0xffffffff for ZIP64)
        uint32_t UncompressedSize = 0;  // Uncompressed size (or 0xffffffff for ZIP64)
        // uint16_t FileNameLength = 0;
        // uint16_t ExtraFieldLength = 0;
        // uint16_t FileCommentLength = 0;
        uint16_t DiskNumberStart = 0;
        uint16_t InternalFileAttributes = 0;
        uint32_t ExternalFileAttributes = 0;
        uint32_t RelativeOffsetOfLocalHeader = 0;
        std::string FileName;
        std::string ExtraField;
        std::string FileComment;
    };

    inline Result<void> Read(ZipCentralDirectoryFileHeader& out, IStream* stream, LittleEndianTag = {}) noexcept
    {
        Result<void> ret;

        uint16_t fileNameLength = 0;
        uint16_t extraFieldLength = 0;
        uint16_t fileCommentLength = 0;

        if (!(ret = Read(out.Signature, stream, LittleEndianTag{})))
            return ret;
        if (out.Signature != ZipCentralDirectoryFileHeader::kSignature)
            return make_error_code(ZipFileReadError::BadCDFileHeaderSignature);
        if (!(ret = Read(out.Version, stream, LittleEndianTag{})))
            return ret;
        if (!(ret = Read(out.VersionNeeded, stream, LittleEndianTag{})))
            return ret;
        if (!(ret = Read(out.Flags, stream, LittleEndianTag{})))
            return ret;
        if (!(ret = Read(out.CompressionMethod, stream, LittleEndianTag{})))
            return ret;
        if (!(ret = Read(out.LastModFileTime, stream, LittleEndianTag{})))
            return ret;
        if (!(ret = Read(out.LastModFileDate, stream, LittleEndianTag{})))
            return ret;
        if (!(ret = Read(out.Crc32, stream, LittleEndianTag{})))
            return ret;
        if (!(ret = Read(out.CompressedSize, stream, LittleEndianTag{})))
            return ret;
        if (!(ret = Read(out.UncompressedSize, stream, LittleEndianTag{})))
            return ret;
        if (!(ret = Read(fileNameLength, stream, LittleEndianTag{})))
            return ret;
        if (!(ret = Read(extraFieldLength, stream, LittleEndianTag{})))
            return ret;
        if (!(ret = Read(fileCommentLength, stream, LittleEndianTag{})))
            return ret;
        if (!(ret = Read(out.DiskNumberStart, stream, LittleEndianTag{})))
            return ret;
        if (!(ret = Read(out.InternalFileAttributes, stream, LittleEndianTag{})))
            return ret;
        if (!(ret = Read(out.ExternalFileAttributes, stream, LittleEndianTag{})))
            return ret;
        if (!(ret = Read(out.RelativeOffsetOfLocalHeader, stream, LittleEndianTag{})))
            return ret;
        try
        {
            out.FileName.resize(fileNameLength);
            out.ExtraField.resize(extraFieldLength);
            out.FileComment.resize(fileCommentLength);
        }
        catch (...)  // bad_alloc
        {
            return make_error_code(std::errc::not_enough_memory);
        }
        auto len = stream->Read(reinterpret_cast<uint8_t*>(out.FileName.data()), fileNameLength);
        if (!len)
            return len.GetError();
        if (*len != fileNameLength)
            return make_error_code(ZipFileReadError::UnexpectedEndOfStream);

        len = stream->Read(reinterpret_cast<uint8_t*>(out.ExtraField.data()), extraFieldLength);
        if (!len)
            return len.GetError();
        if (*len != extraFieldLength)
            return make_error_code(ZipFileReadError::UnexpectedEndOfStream);

        len = stream->Read(reinterpret_cast<uint8_t*>(out.FileComment.data()), fileCommentLength);
        if (!len)
            return len.GetError();
        if (*len != fileCommentLength)
            return make_error_code(ZipFileReadError::UnexpectedEndOfStream);
        return {};
    }

    // 4.5.3 -Zip64 Extended Information Extra Field (0x0001):
    struct ZipZip64ExtendedInformationExtraField
    {
        uint64_t OriginalSize = 0;
        uint64_t CompressedSize = 0;
        uint64_t RelativeHeaderOffset = 0;
        uint32_t DiskStartNumber = 0;
    };

    inline Result<void> Read(ZipZip64ExtendedInformationExtraField& out, IStream* stream, LittleEndianTag = {}) noexcept
    {
        Result<void> ret;

        if (!(ret = Read(out.OriginalSize, stream, LittleEndianTag {})))
            return ret;
        if (!(ret = Read(out.CompressedSize, stream, LittleEndianTag {})))
            return ret;
        if (!(ret = Read(out.RelativeHeaderOffset, stream, LittleEndianTag {})))
            return ret;
        if (!(ret = Read(out.DiskStartNumber, stream, LittleEndianTag {})))
            return ret;
        return {};
    }
}
