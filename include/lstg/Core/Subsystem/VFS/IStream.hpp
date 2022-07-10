/**
 * @file
 * @date 2022/2/20
 * @author 9chu
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#pragma once
#include <memory>
#include "../../Result.hpp"

namespace lstg::Subsystem::VFS
{
    /**
     * 流查找原点
     */
    enum class StreamSeekOrigins
    {
        Begin,
        Current,
        End,
    };

    class IStream;
    using StreamPtr = std::shared_ptr<IStream>;

    /**
     * 流式 IO 抽象接口
     * 对流的操作应保证 STL 级别的线程安全，即各个线程操作自己持有的 IStream 是安全的。
     */
    class IStream
    {
    public:
        IStream() noexcept = default;
        virtual ~IStream() noexcept = default;

    public:
        /**
         * 返回流是否可读
         */
        virtual bool IsReadable() const noexcept = 0;

        /**
         * 返回流是否可写
         */
        virtual bool IsWriteable() const noexcept = 0;

        /**
         * 返回流是否允许查找
         */
        virtual bool IsSeekable() const noexcept = 0;

        /**
         * 返回流的长度
         */
        virtual Result<uint64_t> GetLength() const noexcept = 0;

        /**
         * 设置流的长度
         */
        virtual Result<void> SetLength(uint64_t length) noexcept = 0;

        /**
         * 返回当前的读写位置
         */
        virtual Result<uint64_t> GetPosition() const noexcept = 0;

        /**
         * 寻找读写位置
         * @param offset 偏移量
         * @param origin 起点
         * @return 是否成功
         */
        virtual Result<void> Seek(int64_t offset, StreamSeekOrigins origin) noexcept = 0;

        /**
         * 检查是否到达结尾
         * 通常用于指示读操作是否达到结尾。
         * 对于部分流来说其写操作具备扩展流的作用，因此 Eof 不能判定流已经到达结尾。
         * @return 是否到达流结尾
         */
        virtual Result<bool> IsEof() const noexcept = 0;

        /**
         * 刷新缓冲区
         * 清除该流的所有缓冲区，并使得所有缓冲数据被写入到基础设备。
         * 若操作不支持则应当忽略该方法的调用。
         */
        virtual Result<void> Flush() noexcept = 0;

        /**
         * 读取若干字节
         * 从流中读取count个字节，返回真实读取的个数并提升读写位置到对应数量的字节。
         * 如果遇到EOF，则会读取小于指定个数的字符或者直接返回0。
         * @param buffer 输出缓冲区
         * @param length 数量
         * @return 真实读取数量
         */
        virtual Result<size_t> Read(uint8_t* buffer, size_t length) noexcept = 0;

        /**
         * 将若干字节写入流
         * @param buffer 缓冲区
         * @param length 要写入的数量
         */
        virtual Result<void> Write(const uint8_t* buffer, size_t length) noexcept = 0;

        /**
         * 复制流
         * 复制后的流应当具备单独的读写位置和线程安全性。
         */
        virtual Result<StreamPtr> Clone() const noexcept = 0;
    };

    struct LittleEndianTag {};
    struct BigEndianTag {};

    // <editor-fold desc="Read 方法">

#define LSTG_DEF_READ_BY_REF_METHOD(TYPE, NAME) \
    inline Result<void> Read(TYPE& out, IStream* stream, LittleEndianTag) noexcept \
    {                                                                              \
        auto ret = Read##NAME##LE(stream);                                         \
        if (ret)                                                                   \
        {                                                                          \
            out = *ret;                                                            \
            return {};                                                             \
        }                                                                          \
        return ret.GetError();                                                     \
    }                                                                              \
    inline Result<void> Read(TYPE& out, IStream* stream, BigEndianTag) noexcept    \
    {                                                                              \
        auto ret = Read##NAME##BE(stream);                                         \
        if (ret)                                                                   \
        {                                                                          \
            out = *ret;                                                            \
            return {};                                                             \
        }                                                                          \
        return ret.GetError();                                                     \
    }

#define LSTG_DEF_READ_SIGNED_METHOD(TYPE, NAME)              \
    inline Result<TYPE> Read##NAME##LE(IStream* stream) noexcept \
    {                                                            \
        auto r = ReadU##NAME##LE(stream);                        \
        if (r)                                                   \
        {                                                        \
            TYPE ret = 0;                                        \
            auto v = *r;                                         \
            static_assert(sizeof(v) == sizeof(ret));             \
            ::memcpy(&ret, &v, sizeof(ret));                     \
            return { OkTag{}, ret };                             \
        }                                                        \
        return r.GetError();                                     \
    }                                                            \
    inline Result<TYPE> Read##NAME##BE(IStream* stream) noexcept \
    {                                                            \
        auto r = ReadU##NAME##BE(stream);                        \
        if (r)                                                   \
        {                                                        \
            TYPE ret = 0;                                        \
            auto v = *r;                                         \
            static_assert(sizeof(v) == sizeof(ret));             \
            ::memcpy(&ret, &v, sizeof(ret));                     \
            return { OkTag{}, ret };                             \
        }                                                        \
        return r.GetError();                                     \
    }                                                            \
    LSTG_DEF_READ_BY_REF_METHOD(TYPE, U##NAME)

    /**
     * 读取无符号 8bits 整数
     */
    inline Result<uint8_t> ReadUInt8(IStream* stream) noexcept
    {
        uint8_t buf[1];
        auto r = stream->Read(buf, sizeof(buf));
        if (r)
        {
            if (*r != sizeof(buf))
                return make_error_code(std::errc::io_error);
            return buf[0];
        }
        return r.GetError();
    }

    /**
     * 读取无符号 8bits 整数
     * @param out 输出变量
     * @param stream 流
     * @return 是否成功
     */
    inline Result<void> Read(uint8_t& out, IStream* stream, LittleEndianTag) noexcept
    {
        auto ret = ReadUInt8(stream);
        if (ret)
        {
            out = *ret;
            return {};
        }
        return ret.GetError();
    }

    inline Result<void> Read(uint8_t& out, IStream* stream, BigEndianTag) noexcept
    {
        auto ret = ReadUInt8(stream);
        if (ret)
        {
            out = *ret;
            return {};
        }
        return ret.GetError();
    }

    /**
     * 读取有符号 8bits 整数
     */
    inline Result<int8_t> ReadInt8(IStream* stream) noexcept
    {
        auto r = ReadUInt8(stream);
        if (r)
        {
            int8_t ret;
            ::memcpy(&ret, &(*r), sizeof(ret));
            return ret;
        }
        return r.GetError();
    }

    /**
     * 读取有符号 8bits 整数
     * @param out 输出变量
     * @param stream 流
     * @return 是否成功
     */
    inline Result<void> Read(int8_t& out, IStream* stream, LittleEndianTag) noexcept
    {
        auto ret = ReadInt8(stream);
        if (ret)
        {
            out = *ret;
            return {};
        }
        return ret.GetError();
    }

    inline Result<void> Read(int8_t& out, IStream* stream, BigEndianTag) noexcept
    {
        auto ret = ReadInt8(stream);
        if (ret)
        {
            out = *ret;
            return {};
        }
        return ret.GetError();
    }

    /**
     * 读取无符号 16bits 整数（小端序）
     */
    inline Result<uint16_t> ReadUInt16LE(IStream* stream) noexcept
    {
        uint8_t buf[2];
        auto r = stream->Read(buf, sizeof(buf));
        if (r)
        {
            if (*r != sizeof(buf))
                return make_error_code(std::errc::io_error);
            return {OkTag{}, static_cast<uint16_t>(buf[0]) | (static_cast<uint16_t>(buf[1]) << 8)};
        }
        return r.GetError();
    }

    /**
     * 读取无符号 16bits 整数（大端序）
     */
    inline Result<uint16_t> ReadUInt16BE(IStream* stream) noexcept
    {
        uint8_t buf[2];
        auto r = stream->Read(buf, sizeof(buf));
        if (r)
        {
            if (*r != sizeof(buf))
                return make_error_code(std::errc::io_error);
            return {OkTag{}, static_cast<uint16_t>(buf[1]) | (static_cast<uint16_t>(buf[0]) << 8)};
        }
        return r.GetError();
    }

    LSTG_DEF_READ_BY_REF_METHOD(uint16_t, UInt16)
    LSTG_DEF_READ_SIGNED_METHOD(int16_t, Int16)

    /**
     * 读取无符号 32bits 整数（小端序）
     */
    inline Result<uint32_t> ReadUInt32LE(IStream* stream) noexcept
    {
        uint8_t buf[4];
        auto r = stream->Read(buf, sizeof(buf));
        if (r)
        {
            if (*r != sizeof(buf))
                return make_error_code(std::errc::io_error);
            return {OkTag{}, static_cast<uint32_t>(buf[0]) | (static_cast<uint32_t>(buf[1]) << 8) |
                (static_cast<uint32_t>(buf[2]) << 16) | (static_cast<uint32_t>(buf[3]) << 24)};
        }
        return r.GetError();
    }

    /**
     * 读取无符号 32bits 整数（大端序）
     */
    inline Result<uint32_t> ReadUInt32BE(IStream* stream) noexcept
    {
        uint8_t buf[4];
        auto r = stream->Read(buf, sizeof(buf));
        if (r)
        {
            if (*r != sizeof(buf))
                return make_error_code(std::errc::io_error);
            return {OkTag{}, static_cast<uint32_t>(buf[3]) | (static_cast<uint32_t>(buf[2]) << 8) |
                (static_cast<uint32_t>(buf[1]) << 16) | (static_cast<uint32_t>(buf[0]) << 24)};
        }
        return r.GetError();
    }

    LSTG_DEF_READ_BY_REF_METHOD(uint32_t, UInt32)
    LSTG_DEF_READ_SIGNED_METHOD(int32_t, Int32)

    /**
     * 读取无符号 64bits 整数（小端序）
     */
    inline Result<uint64_t> ReadUInt64LE(IStream* stream) noexcept
    {
        uint8_t buf[8];
        auto r = stream->Read(buf, sizeof(buf));
        if (r)
        {
            if (*r != sizeof(buf))
                return make_error_code(std::errc::io_error);
            return {OkTag{}, static_cast<uint64_t>(buf[0]) | (static_cast<uint64_t>(buf[1]) << 8) |
                (static_cast<uint64_t>(buf[2]) << 16) | (static_cast<uint64_t>(buf[3]) << 24) |
                (static_cast<uint64_t>(buf[4]) << 32) | (static_cast<uint64_t>(buf[5]) << 40) |
                (static_cast<uint64_t>(buf[6]) << 48) | (static_cast<uint64_t>(buf[7]) << 56)};
        }
        return r.GetError();
    }

    /**
     * 读取无符号 64bits 整数（大端序）
     */
    inline Result<uint64_t> ReadUInt64BE(IStream* stream) noexcept
    {
        uint8_t buf[8];
        auto r = stream->Read(buf, sizeof(buf));
        if (r)
        {
            if (*r != sizeof(buf))
                return make_error_code(std::errc::io_error);
            return {OkTag{}, static_cast<uint64_t>(buf[7]) | (static_cast<uint64_t>(buf[6]) << 8) |
                (static_cast<uint64_t>(buf[5]) << 16) | (static_cast<uint64_t>(buf[4]) << 24) |
                (static_cast<uint64_t>(buf[3]) << 32) | (static_cast<uint64_t>(buf[2]) << 40) |
                (static_cast<uint64_t>(buf[1]) << 48) | (static_cast<uint64_t>(buf[0]) << 56)};
        }
        return r.GetError();
    }

    LSTG_DEF_READ_BY_REF_METHOD(uint64_t, UInt64)
    LSTG_DEF_READ_SIGNED_METHOD(int64_t, Int64)

    /**
     * 读取有符号 32bits 浮点数（小端序）
     */
    inline Result<float> ReadFloatLE(IStream* stream) noexcept
    {
        auto r = ReadUInt32LE(stream);
        if (r)
        {
            float ret;
            ::memcpy(&ret, &(*r), sizeof(ret));
            return ret;
        }
        return r.GetError();
    }

    /**
     * 读取有符号 32bits 浮点数（大端序）
     */
    inline Result<float> ReadFloatBE(IStream* stream) noexcept
    {
        auto r = ReadUInt32BE(stream);
        if (r)
        {
            float ret;
            ::memcpy(&ret, &(*r), sizeof(ret));
            return ret;
        }
        return r.GetError();
    }

    LSTG_DEF_READ_BY_REF_METHOD(float, Float)

    /**
     * 读取有符号 64bits 浮点数（小端序）
     */
    inline Result<double> ReadDoubleLE(IStream* stream) noexcept
    {
        auto r = ReadUInt64LE(stream);
        if (r)
        {
            double ret;
            ::memcpy(&ret, &(*r), sizeof(ret));
            return ret;
        }
        return r.GetError();
    }

    /**
     * 读取有符号 64bits 浮点数（大端序）
     */
    inline Result<double> ReadDoubleBE(IStream* stream) noexcept
    {
        auto r = ReadUInt64BE(stream);
        if (r)
        {
            double ret;
            ::memcpy(&ret, &(*r), sizeof(ret));
            return ret;
        }
        return r.GetError();
    }

    LSTG_DEF_READ_BY_REF_METHOD(double, Double)

    /**
     * 读取数组
     * @tparam T 元素类型
     * @tparam N 最大大小
     * @param arr 数组
     * @param stream 流
     * @param count 读取个数
     * @return 是否成功
     */
    template <typename T, size_t N>
    inline Result<void> Read(T(&arr)[N], IStream* stream, LittleEndianTag, size_t count=N) noexcept
    {
        assert(count <= N);
        Result<void> ret;
        for (size_t i = 0; i < count; ++i)
        {
            if (!(ret = Read(arr[i], stream, LittleEndianTag{})))
                return ret;
        }
        return {};
    }

#undef LSTG_DEF_READ_BY_REF_METHOD
#undef LSTG_DEF_READ_SIGNED_METHOD

    // </editor-fold>

    // <editor-fold desc="Write 方法">

    // TODO: 正交写方法

    /**
     * 写入无符号 8bits 整数
     */
    inline Result<void> WriteUInt8(IStream* stream, uint8_t b) noexcept
    {
        uint8_t buf[1] = { b };
        return stream->Write(buf, sizeof(buf));
    }

    /**
     * 写入无符号 16bits 整数（小端序）
     */
    inline Result<void> WriteUInt16LE(IStream* stream, uint16_t value) noexcept
    {
        uint8_t buf[2] = {
            static_cast<uint8_t>(value),
            static_cast<uint8_t>((value >> 8u) & 0xFFu),
        };
        return stream->Write(buf, sizeof(buf));
    }

    /**
     * 写入无符号 16bits 整数（大端序）
     */
    inline Result<void> WriteUInt16BE(IStream* stream, uint16_t value) noexcept
    {
        uint8_t buf[2] = {
            static_cast<uint8_t>((value >> 8u) & 0xFFu),
            static_cast<uint8_t>(value),
        };
        return stream->Write(buf, sizeof(buf));
    }

    /**
     * 写入无符号 32bits 整数（小端序）
     */
    inline Result<void> WriteUInt32LE(IStream* stream, uint32_t value) noexcept
    {
        uint8_t buf[4] = {
            static_cast<uint8_t>(value),
            static_cast<uint8_t>((value >> 8u) & 0xFFu),
            static_cast<uint8_t>((value >> 16u) & 0xFFu),
            static_cast<uint8_t>((value >> 24u) & 0xFFu),
        };
        return stream->Write(buf, sizeof(buf));
    }

    /**
     * 写入无符号 32bits 整数（大端序）
     */
    inline Result<void> WriteUInt32BE(IStream* stream, uint32_t value) noexcept
    {
        uint8_t buf[4] = {
            static_cast<uint8_t>((value >> 24u) & 0xFFu),
            static_cast<uint8_t>((value >> 16u) & 0xFFu),
            static_cast<uint8_t>((value >> 8u) & 0xFFu),
            static_cast<uint8_t>(value),
        };
        return stream->Write(buf, sizeof(buf));
    }

    /**
     * 写入无符号 64bits 整数（小端序）
     */
    inline Result<void> WriteUInt64LE(IStream* stream, uint64_t value) noexcept
    {
        uint8_t buf[8] = {
            static_cast<uint8_t>(value & 0xFFull),
            static_cast<uint8_t>((value >> 8ull) & 0xFFull),
            static_cast<uint8_t>((value >> 16ull) & 0xFFull),
            static_cast<uint8_t>((value >> 24ull) & 0xFFull),
            static_cast<uint8_t>((value >> 32ull) & 0xFFull),
            static_cast<uint8_t>((value >> 40ull) & 0xFFull),
            static_cast<uint8_t>((value >> 48ull) & 0xFFull),
            static_cast<uint8_t>((value >> 56ull) & 0xFFull),
        };
        return stream->Write(buf, sizeof(buf));
    }

    /**
     * 写入无符号 64bits 整数（大端序）
     */
    inline Result<void> WriteUInt64BE(IStream* stream, uint64_t value) noexcept
    {
        uint8_t buf[8] = {
            static_cast<uint8_t>((value >> 56ull) & 0xFFull),
            static_cast<uint8_t>((value >> 48ull) & 0xFFull),
            static_cast<uint8_t>((value >> 40ull) & 0xFFull),
            static_cast<uint8_t>((value >> 32ull) & 0xFFull),
            static_cast<uint8_t>((value >> 24ull) & 0xFFull),
            static_cast<uint8_t>((value >> 16ull) & 0xFFull),
            static_cast<uint8_t>((value >> 8ull) & 0xFFull),
            static_cast<uint8_t>(value & 0xFFull),
        };
        return stream->Write(buf, sizeof(buf));
    }

    /**
     * 写入有符号 8bits 整数
     */
    inline Result<void> WriteInt8(IStream* stream, int8_t value) noexcept
    {
        uint8_t v;
        ::memcpy(&v, &value, sizeof(v));
        return WriteUInt8(stream, v);
    }

    /**
     * 写入有符号 16bits 整数（小端序）
     */
    inline Result<void> WriteInt16LE(IStream* stream, int16_t value) noexcept
    {
        uint16_t v;
        ::memcpy(&v, &value, sizeof(v));
        return WriteUInt16LE(stream, v);
    }

    /**
     * 写入有符号 16bits 整数（大端序）
     */
    inline Result<void> WriteInt16BE(IStream* stream, int16_t value) noexcept
    {
        uint16_t v;
        ::memcpy(&v, &value, sizeof(v));
        return WriteUInt16BE(stream, v);
    }

    /**
     * 写入有符号 32bits 整数（小端序）
     */
    inline Result<void> WriteInt32LE(IStream* stream, int32_t value) noexcept
    {
        uint32_t v;
        ::memcpy(&v, &value, sizeof(v));
        return WriteUInt32LE(stream, v);
    }

    /**
     * 写入有符号 32bits 整数（大端序）
     */
    inline Result<void> WriteInt32BE(IStream* stream, int32_t value) noexcept
    {
        uint32_t v;
        ::memcpy(&v, &value, sizeof(v));
        return WriteUInt32BE(stream, v);
    }

    /**
     * 写入有符号 64bits 整数（小端序）
     */
    inline Result<void> WriteInt64LE(IStream* stream, int64_t value) noexcept
    {
        uint64_t v;
        ::memcpy(&v, &value, sizeof(v));
        return WriteUInt64LE(stream, v);
    }

    /**
     * 写入有符号 64bits 整数（大端序）
     */
    inline Result<void> WriteInt64BE(IStream* stream, int64_t value) noexcept
    {
        uint64_t v;
        ::memcpy(&v, &value, sizeof(v));
        return WriteUInt64BE(stream, v);
    }

    /**
     * 写入有符号 32bits 浮点数（小端序）
     */
    inline Result<void> WriteFloatLE(IStream* stream, float value) noexcept
    {
        uint32_t v;
        ::memcpy(&v, &value, sizeof(v));
        return WriteUInt32LE(stream, v);
    }

    /**
     * 写入有符号 32bits 浮点数（大端序）
     */
    inline Result<void> WriteFloatBE(IStream* stream, float value) noexcept
    {
        uint32_t v;
        ::memcpy(&v, &value, sizeof(v));
        return WriteUInt32BE(stream, v);
    }

    /**
     * 写入有符号 64bits 浮点数（小端序）
     */
    inline Result<void> WriteDoubleLE(IStream* stream, double value) noexcept
    {
        uint64_t v;
        ::memcpy(&v, &value, sizeof(v));
        return WriteUInt64LE(stream, v);
    }

    /**
     * 写入有符号 64bits 浮点数（大端序）
     */
    inline Result<void> WriteDoubleBE(IStream* stream, double value) noexcept
    {
        uint64_t v;
        ::memcpy(&v, &value, sizeof(v));
        return WriteUInt64BE(stream, v);
    }

    // </editor-fold>

    /**
     * 从流中读取所有数据
     * @tparam TContainer 输出容器类型
     * @param out 输出容器
     * @param stream 流
     * @return 是否成功
     */
    template <typename TContainer>
    inline Result<void> ReadAll(TContainer& out, IStream* stream) noexcept
    {
        static const size_t kExpandSize = 4 * 1024;

        static_assert(sizeof(typename TContainer::value_type) == 1);
        assert(stream);
        out.clear();

        try
        {
            while (true)
            {
                auto sz = out.size();
                out.resize(sz + kExpandSize);

                auto err = stream->Read(reinterpret_cast<uint8_t*>(out.data() + sz), kExpandSize);
                if (!err)
                    return err.GetError();

                out.resize(sz + *err);
                if (*err < kExpandSize)
                    break;
            }
        }
        catch (...)
        {
            return make_error_code(std::errc::not_enough_memory);
        }
        return {};
    }
}
