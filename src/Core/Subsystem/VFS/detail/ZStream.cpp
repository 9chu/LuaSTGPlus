/**
 * @file
 * @date 2022/2/28
 * @author 9chu
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
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

ZStream::ZStream(InflateInitTag)
    : m_bDeflateStream(false)
{
    ::memset(&m_stZStream, 0, sizeof(m_stZStream));
    auto ret = ::zng_inflateInit(&m_stZStream);
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
