/**
 * @file
 * @author 9chu
 * @date 2022/2/26
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#pragma once
#include <cstdint>
#include <cstdlib>
#include <vector>
#include <optional>
#include <lstg/Core/Flag.hpp>
#include "ZipStructs.hpp"

namespace lstg::Subsystem::VFS::detail
{
    /**
     * Zip 文化压缩方式
     */
    enum class ZipCompressionMethods
    {
        Store = 0,
        Deflate = 1,
    };

    /**
     * Zip 文件加密方式
     */
    enum class ZipEncryptMethods
    {
        None = 0,
        ZipCrypto = 1,  // 使用 CRC 作为校验
        ZipCrypto2 = 2,  // 使用 DOS 时间作为校验
    };

    /**
     * Zuo 文件项标志位
     */
    LSTG_FLAG_BEGIN(ZipFileEntryFlags)
        None = 0,
        Encrypted = 1,
        Utf8Encoding = 2,
    LSTG_FLAG_END(ZipFileEntryFlags)

    /**
     * Zip 文件项
     */
    struct ZipFileEntry
    {
        ZipFileEntryFlags Flags = ZipFileEntryFlags::None;
        ZipCompressionMethods CompressionMethod = ZipCompressionMethods::Store;
        ZipEncryptMethods EncryptMethod = ZipEncryptMethods::None;
        ::time_t LastModified = 0;
        uint32_t Crc32 = 0;
        uint64_t LocalFileHeaderOffset = 0;
        uint64_t UncompressedSize = 0;
        uint64_t CompressedSize = 0;
        std::string FileName;
        std::string Comment;
    };

    using ZipFileEntryContainer = std::vector<ZipFileEntry>;

    /**
     * ZIP 文件读取支持
     *
     * 我们不实现对分卷的支持。
     */
    class ZipFile
    {
    public:
        explicit ZipFile(StreamPtr stream);

        ZipFile(const ZipFile&) = delete;
        ZipFile(ZipFile&&) noexcept = default;

    public:
        /**
         * 读取所有条目
         * @param out 输出
         * @return 错误码
         */
        Result<void> ReadFileEntries(ZipFileEntryContainer& out) noexcept;

        /**
         * 打开文件
         * @param entry 条目
         * @param password 密码
         * @return 错误码
         */
        Result<StreamPtr> OpenEntry(const ZipFileEntry& entry, std::string_view password) noexcept;

    private:
        Result<void> LocateCentralDirectory() noexcept;

    private:
        StreamPtr m_pUnderlayStream;

        bool m_bIsZip64 = false;
        uint64_t m_uEntryCount = 0;
        int64_t m_ullArchiveBaseOffset = 0;  // 基准偏移
        int64_t m_ullCentralDirectoryOffset = 0;  // 相对整个文件
        int64_t m_ullCentralDirectorySize = 0;
    };
}
