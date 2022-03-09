/**
 * @file
 * @author 9chu
 * @date 2022/2/26
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#include "ZipFile.hpp"

#include <lstg/Core/Subsystem/VFS/ContainerStream.hpp>
#include <lstg/Core/Subsystem/VFS/WindowedStream.hpp>
#include <lstg/Core/Subsystem/VFS/InflateStream.hpp>
#include "ZipPkDecryptStream.hpp"

using namespace std;
using namespace lstg;
using namespace lstg::Subsystem::VFS::detail;

using lstg::Subsystem::VFS::IStream;
using lstg::Subsystem::VFS::StreamPtr;
using lstg::Subsystem::VFS::StreamSeekOrigins;

namespace
{
    Result<bool> ReverseSearchPattern(IStream* stream, const uint8_t* pattern, size_t len, size_t maxSearch) noexcept
    {
        static const unsigned kSearchBufferSize = 64;

        uint8_t buffer[kSearchBufferSize];
        size_t available = 0;
        size_t searchOrigin = 0;

        // 获取当前搜索的起始位置（不包含）
        if (!stream->IsSeekable())
        {
            return make_error_code(errc::not_supported);
        }
        else
        {
            auto ret = stream->GetPosition();
            if (!ret)
                return ret.GetError();
            searchOrigin = *ret;
        }

        assert(len * 2 <= kSearchBufferSize);  // 被搜索子串长度不能超过 kSearchBufferSize / 2
        while (maxSearch > 0)
        {
            // 计算需要读取的量
            auto readCount = std::min<size_t>(std::min<size_t>(maxSearch, kSearchBufferSize - available), searchOrigin);

            // 执行 memmove 调整缓冲区
            assert(readCount + available <= kSearchBufferSize);
            ::memmove(buffer + readCount, buffer, available);

            // 发起读操作
            assert(searchOrigin >= readCount);
            auto seek = stream->Seek(static_cast<ssize_t>(searchOrigin) - static_cast<ssize_t>(readCount),
                StreamSeekOrigins::Begin);
            if (!seek)
                return seek.GetError();
            auto ret = stream->Read(buffer, readCount);
            if (!ret)
                return ret.GetError();
            if (*ret != readCount)
                return make_error_code(errc::io_error);

            // 调整位置
            maxSearch -= readCount;
            available += readCount;
            searchOrigin -= readCount;

            // 在缓冲区中搜索
            if (available < len)
                return false;
            for (size_t i = 0; i <= available - len; ++i)
            {
                // 确保倒序搜索
                auto at = buffer + available - len - i;
                assert(at >= buffer && at + len <= buffer + available);
                if (::memcmp(pattern, at, len) == 0)
                {
                    // 找到匹配串，此时需要 Seek 到对应的位置
                    if (i != 0)
                    {
                        seek = stream->Seek(static_cast<ssize_t>(searchOrigin + (at - buffer)), StreamSeekOrigins::Begin);
                        if (!seek)
                            return seek.GetError();
                    }
                    return true;
                }
            }
            available = len - 1;
        }
        return false;
    }

    time_t DosDateTimeToUnixTimestamp(uint16_t dosTime, uint16_t dosDate) noexcept
    {
        ::tm tm;
        ::memset(&tm, 0, sizeof(tm));
        tm.tm_mday = (uint16_t)(dosDate & 0x1F);
        tm.tm_mon = (uint16_t)(((dosDate & 0x1E0) / 0x20) - 1);
        tm.tm_year = (uint16_t)(((dosDate & 0x0FE00) / 0x0200) + 80);
        tm.tm_hour = (uint16_t)((dosTime & 0xF800) / 0x800);
        tm.tm_min = (uint16_t)((dosTime & 0x7E0) / 0x20);
        tm.tm_sec = (uint16_t)(2 * (dosTime & 0x1F));
        tm.tm_isdst = -1;
        return ::mktime(&tm);
    }

    std::tuple<uint16_t, uint16_t> UnixTimestampToDosDateTime(time_t t) noexcept
    {
        ::tm tm;
        ::memset(&tm, 0, sizeof(tm));

#ifdef LSTG_PLATFORM_WIN32
        ::localtime_s(&t, &tm);
#else
        ::localtime_r(&t, &tm);
#endif
        uint16_t dosDate =
            (tm.tm_mday & 0x1F) |
            (((tm.tm_mon + 1) * 0x20) & 0x1E0) |
            (((tm.tm_year - 80) * 0x0200) & 0x0FE00);
        uint16_t dosTime =
            ((tm.tm_hour * 0x800) & 0xF800) |
            ((tm.tm_min * 0x20) & 0x7E0) |
            ((tm.tm_sec / 2) & 0x1F);
        return {dosTime, dosDate};
    }

    uint16_t GenPkVerifier(uint32_t crc32) noexcept
    {
        return ((crc32 >> 16) & 0xFF) << 8 | ((crc32 >> 24) & 0xFF);
    }

    uint16_t GenPkVerifier2(uint16_t dosTime, uint16_t dosDate) noexcept
    {
        return ((dosDate & 0xFF) << 8) | ((dosTime >> 8) & 0xFF);
    }
}

ZipFile::ZipFile(StreamPtr stream)
    : m_pUnderlayStream(std::move(stream))
{
    auto ret = LocateCentralDirectory();
    ret.ThrowIfError();
}

Result<void> ZipFile::ReadFileEntries(ZipFileEntryContainer& out) noexcept
{
    out.clear();

    // 跳到 CentralDirectory 开始
    auto seek = m_pUnderlayStream->Seek(m_ullCentralDirectoryOffset, StreamSeekOrigins::Begin);
    if (!seek)
        return seek.GetError();

    // 读取所有条目
    out.reserve(m_uEntryCount);
    for (uint64_t i = 0; i < m_uEntryCount; ++i)
    {
        ZipFileEntry entry;

        ZipCentralDirectoryFileHeader cd;
        auto ret = Read(cd, m_pUnderlayStream.get());
        if (!ret)
            return ret.GetError();

        // 校验标志位
        if (((cd.Flags >> 0) & 1) == 1)  // bit0: 加密
            entry.Flags |= ZipFileEntryFlags::Encrypted;
        if (((cd.Flags >> 3) & 1) == 1 && (cd.Crc32 == 0 || cd.CompressedSize == 0 || cd.UncompressedSize == 0))  // bit3: DataDescriptor
        {
            // FIXME: 在使用 DataDescriptor 的时候 大小信息和CRC校验信息会延后放置
            // 在部分实现下，即便设置了 DataDescriptor 也会有 CRC32 和大小信息
            // 因此这里偷懒处理
            return make_error_code(ZipFileReadError::DataDescriptorNotSupported);
        }
        if (((cd.Flags >> 5) & 1) == 1)  // bit5: compressed patched data
            return make_error_code(ZipFileReadError::CompressedPatchedDataNotSupported);
        if (((cd.Flags >> 6) & 1) == 1)  // bit6: strong encryption
            return make_error_code(ZipFileReadError::StrongEncryptionNotSupported);
        if (((cd.Flags >> 11) & 1) == 1)  // bit11: Language encoding flag
            entry.Flags |= ZipFileEntryFlags::Utf8Encoding;
        if (((cd.Flags >> 13) & 1) == 1)  // bit13: Central Directory encryption
            return make_error_code(ZipFileReadError::CentralDirectoryEncryptionNotSupported);

        // 校验压缩算法
        if (cd.CompressionMethod == 0)
            entry.CompressionMethod = ZipCompressionMethods::Store;
        else if (cd.CompressionMethod == 8)
            entry.CompressionMethod = ZipCompressionMethods::Deflate;
        else
            return make_error_code(ZipFileReadError::CompressionMethodNotSupported);

        // 设置加解密数据
        if (entry.Flags & ZipFileEntryFlags::Encrypted)
            entry.EncryptMethod = (((cd.Flags >> 3) & 1) == 1) ? ZipEncryptMethods::ZipCrypto2 : ZipEncryptMethods::ZipCrypto;

        // 设置时间戳
        entry.LastModified = DosDateTimeToUnixTimestamp(cd.LastModFileTime, cd.LastModFileDate);

        // 设置 CRC32
        entry.Crc32 = cd.Crc32;

        // 读取大小、偏移
        if (cd.UncompressedSize == numeric_limits<uint32_t>::max() || cd.CompressedSize == numeric_limits<uint32_t>::max() ||
            cd.RelativeOffsetOfLocalHeader == numeric_limits<uint32_t>::max())
        {
            if (!m_bIsZip64)
                return make_error_code(ZipFileReadError::BadCDEntrySize);

            optional<ZipZip64ExtendedInformationExtraField> zip64ExtraField;

            // 搜索扩展头部
            BufferViewStream extendStream(Span<uint8_t> { reinterpret_cast<uint8_t*>(cd.ExtraField.data()), cd.ExtraField.size() });
            while (!extendStream.IsEof())
            {
                uint16_t tag = 0, size = 0;
                if (!(ret = Read(tag, &extendStream, LittleEndianTag {})))
                    return ret.GetError();
                if (!(ret = Read(size, &extendStream, LittleEndianTag {})))
                    return ret.GetError();

                if (tag == 0x01)   // Zip64 Extended
                {
                    zip64ExtraField = {};
                    if (!(ret = Read(*zip64ExtraField, &extendStream)))
                        return ret.GetError();
                    break;
                }
            }

            // 如果没找到扩展头部，则报错
            if (!zip64ExtraField)
                return make_error_code(ZipFileReadError::MissingZip64ExtraField);

            // 赋值
            entry.LocalFileHeaderOffset = zip64ExtraField->RelativeHeaderOffset + m_ullArchiveBaseOffset;
            entry.UncompressedSize = zip64ExtraField->OriginalSize;
            entry.CompressedSize = zip64ExtraField->CompressedSize;
        }
        else
        {
            // 直接赋值
            entry.LocalFileHeaderOffset = cd.RelativeOffsetOfLocalHeader + m_ullArchiveBaseOffset;
            entry.UncompressedSize = cd.UncompressedSize;
            entry.CompressedSize = cd.CompressedSize;
        }

        // 文件名和注释
        entry.FileName = std::move(cd.FileName);
        entry.Comment = std::move(cd.FileComment);

        out.emplace_back(std::move(entry));
    }

    return {};
}

Result<StreamPtr> ZipFile::OpenEntry(const ZipFileEntry& entry, std::string_view password) noexcept
{
    // 复制以保证多线程使用安全
    auto clone = m_pUnderlayStream->Clone();
    if (!clone)
        return clone.GetError();

    StreamPtr stream = static_pointer_cast<IStream>(std::move(*clone));

    // 定位到范围
    auto ret = stream->Seek(static_cast<int64_t>(entry.LocalFileHeaderOffset), StreamSeekOrigins::Begin);
    if (!ret)
        return ret.GetError();

    // 读 LocalFileHeader
    ret = SkipZipLocalFileHeader(stream.get());
    if (!ret)
        return ret.GetError();

    try
    {
        // 封装范围
        {
            auto windowedStream = make_shared<WindowedStream>(std::move(stream), entry.CompressedSize);
            stream = static_pointer_cast<IStream>(std::move(windowedStream));
        }

        // 是否有加密
        if (entry.Flags & ZipFileEntryFlags::Encrypted)
        {
            shared_ptr<ZipPkDecryptStream> decryptStream;
            if (entry.EncryptMethod == ZipEncryptMethods::ZipCrypto)
            {
                decryptStream = make_shared<ZipPkDecryptStream>(std::move(stream), password, GenPkVerifier(entry.Crc32));
            }
            else
            {
                assert(entry.EncryptMethod == ZipEncryptMethods::ZipCrypto2);
                auto [dosTime, dosDate] = UnixTimestampToDosDateTime(entry.LastModified);
                decryptStream = make_shared<ZipPkDecryptStream>(std::move(stream), password, GenPkVerifier2(dosTime, dosDate));
            }
            stream = static_pointer_cast<IStream>(std::move(decryptStream));
        }

        // 是否有压缩
        if (entry.CompressionMethod == ZipCompressionMethods::Deflate)
        {
            auto decompressStream = make_shared<InflateStream>(std::move(stream), entry.UncompressedSize);
            stream = static_pointer_cast<IStream>(std::move(decompressStream));
        }

        return stream;
    }
    catch (const system_error& ex)
    {
        return ex.code();
    }
    catch (...)  // bad_alloc
    {
        return make_error_code(errc::not_enough_memory);
    }
}

Result<void> ZipFile::LocateCentralDirectory() noexcept
{
    static const unsigned kMaxSearchDistance = (1 << 20);
    static const uint8_t kMagic[] = { 0x50, 0x4b, 0x05, 0x06 };

    ZipEndOfCentralDirectoryRecord eocd;
    Zip64EndOfCentralDirectoryRecord eocd64;
    Zip64EndOfCentralDirectoryLocator eocd64Locator;

    // 搜索 EOCD 的位置
    m_pUnderlayStream->Seek(0, StreamSeekOrigins::End);
    auto found = ReverseSearchPattern(m_pUnderlayStream.get(), kMagic, 4, kMaxSearchDistance);
    if (!found)
        return found.GetError();
    if (!*found)
        return make_error_code(ZipFileReadError::EOCDNotFound);

    // 读取 EOCD
    auto eocdOffset = m_pUnderlayStream->GetPosition();
    if (!eocdOffset)
        return eocdOffset.GetError();
    auto ret = Read(eocd, m_pUnderlayStream.get());
    if (!ret)
        return ret;

    // 检查是否是 Zip64
    if (eocd.DiskNum == numeric_limits<uint16_t>::max() ||
        eocd.DiskNumWithCentralDirectory == numeric_limits<uint16_t>::max() ||
        eocd.EntryNum == numeric_limits<uint16_t>::max() ||
        eocd.EntryNumCentralDirectory == numeric_limits<uint16_t>::max() ||
        eocd.SizeOfCentralDirectory == numeric_limits<uint32_t>::max() ||
        eocd.OffsetOfCentralDirectory == numeric_limits<uint32_t>::max())
    {
        m_bIsZip64 = true;

        // 往前移动找到 EndOfCentralDirectoryLocator
        if (*eocdOffset < Zip64EndOfCentralDirectoryLocator::kSize)
            return make_error_code(ZipFileReadError::UnexpectedEndOfStream);
        ret = m_pUnderlayStream->Seek(eocdOffset - Zip64EndOfCentralDirectoryLocator::kSize,
            StreamSeekOrigins::Begin);
        if (!ret)
            return ret.GetError();

        // 读取 Locator
        ret = Read(eocd64Locator, m_pUnderlayStream.get());
        if (!ret)
            return ret;

        // 检查格式是否支持
        if (eocd64Locator.DiskNum != 0 || eocd64Locator.DiskNumWithEndOfCentralDirectory != 0)
            return make_error_code(ZipFileReadError::MultiDiskFormatNotSupported);

        // 定位到 EndOfCentralDirectoryRecord
        // NOTE: 这里有个疑问，如果直接 Seek 到这个 Offset，意味着 Zip64 这种格式不能在前面直接追加数据，会导致找不到 EOCD64
        ret = m_pUnderlayStream->Seek(eocd64Locator.RelativeOffsetOfEndOfCentralDirectory, StreamSeekOrigins::Begin);
        if (!ret)
            return ret.GetError();

        // 读取 Record64
        ret = Read(eocd64, m_pUnderlayStream.get());
        if (!ret)
            return ret;

        // 检查格式是否支持
        if (eocd64.DiskNum != 0 || eocd64.DiskNumWithCentralDirectory != 0)
            return make_error_code(ZipFileReadError::MultiDiskFormatNotSupported);
        if (eocd64.EntryNum != eocd64.EntryNumCentralDirectory)
            return make_error_code(ZipFileReadError::MultiDiskFormatNotSupported);

        eocdOffset = eocd64Locator.RelativeOffsetOfEndOfCentralDirectory;
        m_uEntryCount = eocd64.EntryNum;
        m_ullCentralDirectoryOffset = static_cast<int64_t>(eocd64.OffsetOfCentralDirectory);
        m_ullCentralDirectorySize = static_cast<int64_t>(eocd64.SizeOfCentralDirectory);

        // 转换到 int64_t，方便计算防止溢出，换言之理论上不会处理过大的文件，没有意义
        if (m_ullCentralDirectoryOffset < 0)
            return make_error_code(ZipFileReadError::CDOffsetTooBig);
        if (m_ullCentralDirectorySize < 0)
            return make_error_code(ZipFileReadError::CDSizeTooBig);
    }
    else
    {
        m_bIsZip64 = false;

        // 检查格式是否支持
        if (eocd.DiskNum != 0 || eocd.DiskNumWithCentralDirectory != 0)
            return make_error_code(ZipFileReadError::MultiDiskFormatNotSupported);
        if (eocd.EntryNum != eocd.EntryNumCentralDirectory)
            return make_error_code(ZipFileReadError::MultiDiskFormatNotSupported);

        m_uEntryCount = eocd.EntryNum;
        m_ullCentralDirectoryOffset = eocd.OffsetOfCentralDirectory;
        m_ullCentralDirectorySize = eocd.SizeOfCentralDirectory;
    }

    // 首先尝试用 Offset 定位 CentralDirectory
    ret = m_pUnderlayStream->Seek(m_ullCentralDirectoryOffset, StreamSeekOrigins::Begin);
    if (ret)
    {
        uint32_t signature = 0;
        ret = Read(signature, m_pUnderlayStream.get(), LittleEndianTag{});
        if (ret)
        {
            if (signature == ZipCentralDirectoryFileHeader::kSignature)
            {
                // 保证 CentralDirectory 总是在 EndOfCentralDirectory 之前
                if (static_cast<uint64_t>(m_ullCentralDirectoryOffset) >= *eocdOffset)
                    return make_error_code(ZipFileReadError::CDLocationInvalid);
                if (static_cast<uint64_t>(m_ullCentralDirectoryOffset + m_ullCentralDirectorySize) > *eocdOffset)
                    return make_error_code(ZipFileReadError::CDLocationInvalid);
                return {};
            }
        }
    }

    // 尝试用 Size 定位 CentralDirectory
    if (*eocdOffset < static_cast<uint64_t>(m_ullCentralDirectorySize))
        return make_error_code(ZipFileReadError::CDLocationInvalid);
    auto realOffsetOfCD = static_cast<int64_t>(*eocdOffset - m_ullCentralDirectorySize);
    ret = m_pUnderlayStream->Seek(realOffsetOfCD, StreamSeekOrigins::Begin);
    if (!ret)
        return ret.GetError();

    uint32_t signature = 0;
    ret = Read(signature, m_pUnderlayStream.get(), LittleEndianTag{});
    if (!ret)
        return ret.GetError();
    if (signature != ZipCentralDirectoryFileHeader::kSignature)
        return make_error_code(ZipFileReadError::BadCDFileHeaderSignature);

    // 使用 Size 重新定位 Offset
    m_ullArchiveBaseOffset = realOffsetOfCD - m_ullCentralDirectoryOffset;
    m_ullCentralDirectoryOffset = realOffsetOfCD;

    // 保证 CentralDirectory 总是在 EndOfCentralDirectory 之前
    if (static_cast<uint64_t>(m_ullCentralDirectoryOffset) >= *eocdOffset)
        return make_error_code(ZipFileReadError::CDLocationInvalid);
    if (static_cast<uint64_t>(m_ullCentralDirectoryOffset + m_ullCentralDirectorySize) > *eocdOffset)
        return make_error_code(ZipFileReadError::CDLocationInvalid);
    return {};
}
