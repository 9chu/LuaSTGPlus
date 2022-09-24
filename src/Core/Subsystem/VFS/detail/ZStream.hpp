/**
 * @file
 * @date 2022/2/28
 * @author 9chu
 * 此文件为 LuaSTGPlus 项目的一部分，版权与许可声明详见 COPYRIGHT.txt。
 */
#pragma once
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <zlib-ng.h>

namespace lstg::Subsystem::VFS::detail
{
    struct DeflateInitTag {};
    struct InflateInitTag {};

    class ZStream
    {
    public:
        ZStream(DeflateInitTag, int compressionLevel = Z_DEFAULT_COMPRESSION);
        ZStream(InflateInitTag, bool rawDeflateData = false);

        ZStream(const ZStream& org);
        ZStream(ZStream&&) = delete;
        ZStream& operator=(ZStream&&) = delete;

        ~ZStream();

    public:
        const ::zng_stream* operator*() const noexcept { return &m_stZStream; }
        ::zng_stream* operator*() noexcept { return &m_stZStream; }

        const ::zng_stream* operator->() const noexcept { return &m_stZStream; }
        ::zng_stream* operator->() noexcept { return &m_stZStream; }

    private:
        bool m_bDeflateStream = false;
        ::zng_stream m_stZStream{};
    };
}
