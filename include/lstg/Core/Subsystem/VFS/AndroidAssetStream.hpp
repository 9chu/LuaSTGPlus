/**
 * @file
 * @date 2023/8/19
 * @author 9chu
 * 此文件为 LuaSTGPlus 项目的一部分，版权与许可声明详见 COPYRIGHT.txt。
 */
#pragma once
#include "IStream.hpp"
#include "IFileSystem.hpp"

struct AAsset;
struct AAssetManager;

namespace lstg::Subsystem::VFS
{
    /**
     * 安卓 Asset 数据流
     */
    class AndroidAssetStream :
        public IStream
    {
    public:
        AndroidAssetStream(AAssetManager* assetManager, const char* path);
        AndroidAssetStream(AAssetManager* assetManager, std::string&& path, AAsset* asset) noexcept;
        ~AndroidAssetStream() override;

        AndroidAssetStream(AndroidAssetStream&& rhs) noexcept;
        AndroidAssetStream& operator=(AndroidAssetStream&& rhs) noexcept;

    public:  // IStream
        bool IsReadable() const noexcept override;
        bool IsWriteable() const noexcept override;
        bool IsSeekable() const noexcept override;
        Result<uint64_t> GetLength() const noexcept override;
        Result<void> SetLength(uint64_t length) noexcept override;
        Result<uint64_t> GetPosition() const noexcept override;
        Result<void> Seek(int64_t offset, StreamSeekOrigins origin) noexcept override;
        Result<bool> IsEof() const noexcept override;
        Result<void> Flush() noexcept override;
        Result<size_t> Read(uint8_t* buffer, size_t length) noexcept override;
        Result<void> Write(const uint8_t* buffer, size_t length) noexcept override;
        Result<StreamPtr> Clone() const noexcept override;

    private:
        AAssetManager* m_pManager = nullptr;  // NOTE: 需要外部持有 Java 对象引用
        std::string m_stPath;
        AAsset* m_pAsset = nullptr;
    };
} // namespace lstg::Subsystem::VFS
