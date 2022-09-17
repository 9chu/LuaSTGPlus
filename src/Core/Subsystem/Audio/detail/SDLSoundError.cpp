/**
 * @file
 * @date 2022/9/13
 * @author 9chu
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#include "SDLSoundError.hpp"

#include <cassert>

//#include <SDL_sound_internal.h>
// from SDL_sound_internal
#define ERR_IS_INITIALIZED       "Already initialized"
#define ERR_NOT_INITIALIZED      "Not initialized"
#define ERR_INVALID_ARGUMENT     "Invalid argument"
#define ERR_OUT_OF_MEMORY        "Out of memory"
#define ERR_NOT_SUPPORTED        "Operation not supported"
#define ERR_UNSUPPORTED_FORMAT   "Sound format unsupported"
#define ERR_NOT_A_HANDLE         "Not a file handle"
#define ERR_NO_SUCH_FILE         "No such file"
#define ERR_PAST_EOF             "Past end of file"
#define ERR_IO_ERROR             "I/O error"
#define ERR_COMPRESSION          "(De)compression error"
#define ERR_PREV_ERROR           "Previous decoding already caused an error"
#define ERR_PREV_EOF             "Previous decoding already triggered EOF"
#define ERR_CANNOT_SEEK          "Sample is not seekable"

using namespace std;
using namespace lstg;
using namespace lstg::Subsystem::Audio::detail;

const SDLSoundErrorCategory& SDLSoundErrorCategory::GetInstance() noexcept
{
    static const SDLSoundErrorCategory kInstance;
    return kInstance;
}

const char* SDLSoundErrorCategory::name() const noexcept
{
    return "SDLSoundError";
}

std::string SDLSoundErrorCategory::message(int ev) const
{
    switch (static_cast<SDLSoundErrorCodes>(ev))
    {
        default:
            assert(false);
        case SDLSoundErrorCodes::Unknown:
            return "Unknown";
        case SDLSoundErrorCodes::IsInitialized:
            return ERR_IS_INITIALIZED;
        case SDLSoundErrorCodes::NotInitialized:
            return ERR_NOT_INITIALIZED;
        case SDLSoundErrorCodes::InvalidArgument:
            return ERR_INVALID_ARGUMENT;
        case SDLSoundErrorCodes::OutOfMemory:
            return ERR_OUT_OF_MEMORY;
        case SDLSoundErrorCodes::NotSupported:
            return ERR_NOT_SUPPORTED;
        case SDLSoundErrorCodes::UnsupportedFormat:
            return ERR_UNSUPPORTED_FORMAT;
        case SDLSoundErrorCodes::NotHandle:
            return ERR_NOT_A_HANDLE;
        case SDLSoundErrorCodes::NoSuchFile:
            return ERR_NO_SUCH_FILE;
        case SDLSoundErrorCodes::PastEOF:
            return ERR_PAST_EOF;
        case SDLSoundErrorCodes::IOError:
            return ERR_IO_ERROR;
        case SDLSoundErrorCodes::CompressionError:
            return ERR_COMPRESSION;
        case SDLSoundErrorCodes::PrevError:
            return ERR_PREV_ERROR;
        case SDLSoundErrorCodes::PrevEOF:
            return ERR_PREV_EOF;
        case SDLSoundErrorCodes::NotSeekable:
            return ERR_CANNOT_SEEK;
    }
}

SDLSoundErrorCodes Subsystem::Audio::detail::FromErrorString(const char* str) noexcept
{
    if (::strcmp(str, ERR_IS_INITIALIZED) == 0)
        return SDLSoundErrorCodes::IsInitialized;
    if (::strcmp(str, ERR_NOT_INITIALIZED) == 0)
        return SDLSoundErrorCodes::NotInitialized;
    if (::strcmp(str, ERR_INVALID_ARGUMENT) == 0)
        return SDLSoundErrorCodes::InvalidArgument;
    if (::strcmp(str, ERR_OUT_OF_MEMORY) == 0)
        return SDLSoundErrorCodes::OutOfMemory;
    if (::strcmp(str, ERR_NOT_SUPPORTED) == 0)
        return SDLSoundErrorCodes::NotSupported;
    if (::strcmp(str, ERR_UNSUPPORTED_FORMAT) == 0)
        return SDLSoundErrorCodes::UnsupportedFormat;
    if (::strcmp(str, ERR_NOT_A_HANDLE) == 0)
        return SDLSoundErrorCodes::NotHandle;
    if (::strcmp(str, ERR_NO_SUCH_FILE) == 0)
        return SDLSoundErrorCodes::NoSuchFile;
    if (::strcmp(str, ERR_PAST_EOF) == 0)
        return SDLSoundErrorCodes::PastEOF;
    if (::strcmp(str, ERR_IO_ERROR) == 0)
        return SDLSoundErrorCodes::IOError;
    if (::strcmp(str, ERR_COMPRESSION) == 0)
        return SDLSoundErrorCodes::CompressionError;
    if (::strcmp(str, ERR_PREV_ERROR) == 0)
        return SDLSoundErrorCodes::PrevError;
    if (::strcmp(str, ERR_PREV_EOF) == 0)
        return SDLSoundErrorCodes::PrevEOF;
    if (::strcmp(str, ERR_CANNOT_SEEK) == 0)
        return SDLSoundErrorCodes::NotSeekable;
    return SDLSoundErrorCodes::Unknown;
}
