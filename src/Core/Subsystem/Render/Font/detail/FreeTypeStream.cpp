/**
 * @file
 * @date 2022/6/27
 * @author 9chu
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#include "FreeTypeStream.hpp"

#include <lstg/Core/Logging.hpp>

using namespace std;
using namespace lstg;
using namespace lstg::Subsystem::Render::Font::detail;

LSTG_DEF_LOG_CATEGORY(FreeTypeStream);

unsigned long FreeTypeStream::OnRead(FT_Stream stream, unsigned long offset, unsigned char* buffer, unsigned long count) noexcept
{
    auto self = static_cast<FreeTypeStream*>(stream);
    assert(self->Stream);

    // 执行 Seek 操作
    if (self->pos != offset)
    {
        auto ret = self->Stream->Seek(static_cast<int64_t>(offset), VFS::StreamSeekOrigins::Begin);
        if (!ret)
        {
            LSTG_LOG_ERROR_CAT(FreeTypeStream, "Error perform seek on stream, ret={}", ret.GetError());
            // 只有当 count == 0 时才能返回非 0 值表示错误
            return count == 0 ? ret.GetError().value() : 0;
        }
    }

    // 执行读操作
    if (count > 0)
    {
        auto read = self->Stream->Read(buffer, count);
        if (!read)
        {
            LSTG_LOG_ERROR_CAT(FreeTypeStream, "Error perform read on stream, ret={}", read.GetError());
            return 0;
        }

        self->RefreshPosition();
        return *read;
    }

    self->RefreshPosition();
    return 0;
}

void FreeTypeStream::OnClose(FT_Stream stream) noexcept
{
    auto self = static_cast<FreeTypeStream*>(stream);
    assert(self->Stream);
    self->Stream.reset();
}

FreeTypeStream::FreeTypeStream(VFS::StreamPtr s)
{
    // 将输入流转换到可以随机访问的内存流
    Stream = ConvertToSeekableStream(std::move(s)).ThrowIfError();
    assert(Stream->IsSeekable() && Stream->GetLength());

    // 无用
    base = nullptr;
    descriptor.pointer = nullptr;
    pathname.pointer = nullptr;
    memory = nullptr;
    cursor = nullptr;
    limit = nullptr;

    // 填充大小
    size = Stream->GetLength().ThrowIfError();
//    auto sz = Stream->GetLength();
//    if (!sz)
//    {
//        if (sz.GetError() == make_error_code(errc::not_supported))
//            size = 0x7FFFFFFF;
//        else
//            sz.ThrowIfError();
//    }
//    size = static_cast<unsigned long>(*sz);

    // 填充位置
    pos = 0;
    RefreshPosition();

    // 回调
    read = OnRead;
    close = OnClose;
}

void FreeTypeStream::RefreshPosition() noexcept
{
    auto p = Stream->GetPosition();
    if (p)
        pos = *p;
    else
        LSTG_LOG_ERROR_CAT(FreeTypeStream, "Error perform tell on stream, ret={}", p.GetError());
}
