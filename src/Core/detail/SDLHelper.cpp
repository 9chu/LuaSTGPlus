/**
 * @file
 * @author 9chu
 * @date 2022/2/16
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#include "SDLHelper.hpp"

#include <lstg/Core/Subsystem/VFS/IStream.hpp>

using namespace std;
using namespace lstg;

const detail::SDLErrorCategory& detail::SDLErrorCategory::GetInstance() noexcept
{
    static const SDLErrorCategory kInstance;
    return kInstance;
}

const char* detail::SDLErrorCategory::name() const noexcept
{
    return "SDLError";
}

std::string detail::SDLErrorCategory::message(int ev) const
{
    // SDL 并没有一个枚举类型的错误码，这里总是获取最近一次的错误
    return SDL_GetError();
}

detail::SDLRWOpsPtr detail::CreateRWOpsFromStream(Subsystem::VFS::IStream* stream) noexcept
{
    SDLRWOpsPtr p;
    p.reset(::SDL_AllocRW());
    if (!p)
        return nullptr;

    p->type = SDL_RWOPS_UNKNOWN;
    p->hidden.unknown.data1 = stream;
    p->size = [](struct SDL_RWops* context) noexcept -> Sint64 {
        auto stream = static_cast<Subsystem::VFS::IStream*>(context->hidden.unknown.data1);
        auto ret = stream->GetLength();
        if (!ret)
            return -1;
        return static_cast<Sint64>(*ret);
    };
    p->close = [](struct SDL_RWops* context) noexcept -> int {
        auto stream = static_cast<Subsystem::VFS::IStream*>(context->hidden.unknown.data1);
        stream->Flush();  // ignore error
        ::SDL_FreeRW(context);
        return 0;
    };
    p->read = [](struct SDL_RWops* context, void* ptr, size_t size, size_t maxnum) noexcept -> size_t {
        auto stream = static_cast<Subsystem::VFS::IStream*>(context->hidden.unknown.data1);
        auto total = size * maxnum;
        if (total == 0)
            return 0;
        auto ret = stream->Read(static_cast<uint8_t*>(ptr), total);
        if (!ret)
            return 0;
        assert((*ret) % size == 0);  // 极端情况下不一定可以整除
        return *ret / size;
    };
    p->write = [](struct SDL_RWops* context, const void* ptr, size_t size, size_t num) noexcept -> size_t {
        auto stream = static_cast<Subsystem::VFS::IStream*>(context->hidden.unknown.data1);
        auto total = size * num;
        if (total == 0)
            return 0;
        auto ret = stream->Write(static_cast<const uint8_t*>(ptr), total);
        if (!ret)
            return 0;
        return num;
    };
    p->seek = [](struct SDL_RWops* context, Sint64 offset, int whence) noexcept -> Sint64 {
        auto stream = static_cast<Subsystem::VFS::IStream*>(context->hidden.unknown.data1);
        Subsystem::VFS::StreamSeekOrigins origins = Subsystem::VFS::StreamSeekOrigins::Begin;
        switch (whence)
        {
            case RW_SEEK_SET:
                origins = Subsystem::VFS::StreamSeekOrigins::Begin;
                break;
            case RW_SEEK_CUR:
                origins = Subsystem::VFS::StreamSeekOrigins::Current;
                break;
            case RW_SEEK_END:
                origins = Subsystem::VFS::StreamSeekOrigins::End;
                break;
            default:
                assert(false);
                break;
        }
        auto ret = stream->Seek(offset, origins);
        if (!ret)
            return -1;
        auto cur = stream->GetPosition();
        if (!cur)
            return -1;
        return static_cast<Sint64>(*cur);
    };
    return p;
}
