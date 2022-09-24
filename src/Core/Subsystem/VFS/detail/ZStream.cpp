/**
 * @file
 * @date 2022/2/28
 * @author 9chu
 * 此文件为 LuaSTGPlus 项目的一部分，版权与许可声明详见 COPYRIGHT.txt。
 */
#include "ZStream.hpp"

#include <cassert>
#include <system_error>
#include "ZLibError.hpp"

using namespace std;
using namespace lstg;
using namespace lstg::Subsystem::VFS::detail;

ZStream::ZStream(DeflateInitTag, int compressionLevel)
    : m_bDeflateStream(true)
{
    ::memset(&m_stZStream, 0, sizeof(m_stZStream));
    auto ret = ::zng_deflateInit(&m_stZStream, compressionLevel);
    if (ret != Z_OK)
        throw system_error(make_error_code(static_cast<ZLibError>(ret)));
}

ZStream::ZStream(InflateInitTag, bool rawDeflateData)
    : m_bDeflateStream(false)
{
    ::memset(&m_stZStream, 0, sizeof(m_stZStream));
    auto ret = ::zng_inflateInit2(&m_stZStream, rawDeflateData ? -MAX_WBITS : MAX_WBITS);
    if (ret != Z_OK)
        throw system_error(make_error_code(static_cast<ZLibError>(ret)));
}

ZStream::ZStream(const ZStream& org)
    : m_bDeflateStream(org.m_bDeflateStream)
{
    int ret;
    if (m_bDeflateStream)
        ret = ::zng_deflateCopy(&m_stZStream, const_cast<zng_stream*>(&(org.m_stZStream)));
    else
        ret = ::zng_inflateCopy(&m_stZStream, const_cast<zng_stream*>(&(org.m_stZStream)));
    if (ret != Z_OK)
        throw system_error(make_error_code(static_cast<ZLibError>(ret)));
}

ZStream::~ZStream()
{
    int ok = 0;
    if (m_bDeflateStream)
        ok = ::zng_deflateEnd(&m_stZStream);
    else
        ok = ::zng_inflateEnd(&m_stZStream);
    static_cast<void>(ok);
    assert(ok == Z_OK);
}
