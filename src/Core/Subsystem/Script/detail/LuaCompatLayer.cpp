/**
 * @file
 * @date 2022/9/27
 * @author 9chu
 * 此文件为 LuaSTGPlus 项目的一部分，版权与许可声明详见 COPYRIGHT.txt。
 */
#include "LuaCompatLayer.hpp"

#include <lstg/Core/Logging.hpp>
#include <lstg/Core/AppBase.hpp>
#include <lstg/Core/Subsystem/ScriptSystem.hpp>
#include <lstg/Core/Subsystem/Script/LuaPush.hpp>

using namespace std;
using namespace lstg;
using namespace lstg::Subsystem::Script::detail;

LSTG_DEF_LOG_CATEGORY(LuaCompatLayer);

namespace
{
    inline Subsystem::VirtualFileSystem& GetVFS()
    {
        return *AppBase::GetInstance().GetSubsystem<Subsystem::VirtualFileSystem>();
    }

    inline Subsystem::ScriptSystem& GetScriptSystem()
    {
        return *AppBase::GetInstance().GetSubsystem<Subsystem::ScriptSystem>();
    }
}

// <editor-fold desc="bit">

#ifdef LSTG_PLATFORM_EMSCRIPTEN
extern "C" int luaopen_bit(lua_State *L);
#endif

// </editor-fold>
// <editor-fold desc="os">

static int OsExecute(lua_State* L)
{
    LSTG_LOG_WARN_CAT(LuaCompatLayer, "os.execute is deprecated, command: {}", luaL_checkstring(L, 1));
    lua_pushinteger(L, -1);
    return 1;
}

static int OsRemove(lua_State* L)
{
    auto path = luaL_checkstring(L, 1);

    auto& vfs = GetVFS();
    auto& scriptSys = GetScriptSystem();

    auto target = scriptSys.MakeAbsolutePathForIo(path);
    if (!target)
        luaL_error(L, "cannot alloc memory");

    auto ret = vfs.Remove(target->ToStringView());
    if (!ret)
    {
        lua_pushnil(L);
        lua_pushfstring(L, "unable to delete file '%s': %s", path, ret.GetError().message().c_str());
        return 2;
    }
    lua_pushboolean(L, 1);
    return 1;
}

static int OsRename(lua_State* L)
{
    auto from = luaL_checkstring(L, 1);
    auto to = luaL_checkstring(L, 2);

    auto& vfs = GetVFS();
    auto& scriptSys = GetScriptSystem();

    auto targetFrom = scriptSys.MakeAbsolutePathForIo(from);
    if (!targetFrom)
        luaL_error(L, "cannot alloc memory");
    auto targetTo = scriptSys.MakeAbsolutePathForIo(to);
    if (!targetTo)
        luaL_error(L, "cannot alloc memory");

    auto ret = vfs.Rename(targetFrom->ToStringView(), targetTo->ToStringView());
    if (!ret)
    {
        lua_pushnil(L);
        lua_pushfstring(L, "unable to rename file '%s' -> '%s': %s", from, to, ret.GetError().message().c_str());
        return 2;
    }
    lua_pushboolean(L, 1);
    return 1;
}

static int OsTmpName(lua_State* L)
{
    auto randChar = []() noexcept -> char {
        auto r = ::rand() % 62;

        // 0-9, 10 个字符
        if (0 <= r && r <= 9)
            return static_cast<char>('0' + r);
        // a-z, 26 个字符
        else if (10 <= r && r <= 35)
            return static_cast<char>((r - 10) + 'a');
        // A-Z, 26 个字符
        else if (36 <= r && r <= 61)
            return static_cast<char>((r - 36) + 'A');
        assert(false);
        return '0';
    };

    auto& vfs = GetVFS();

    // 我们总是创建一个名为 /storage/tmp_XXXXXXXXX 的临时文件
    while (true)
    {
        static const char kPrefix[] = { "/storage/tmp_" };
        static const size_t kPrefixLength = sizeof(kPrefix);
        static const size_t kNameLength = 9;

        char buffer[kPrefixLength + kNameLength + 1];
        ::memset(buffer, 0, sizeof(buffer));

        ::strcpy(buffer, kPrefix);
        for (size_t i = 0 ; i < 9; ++i)
        {
            assert(i + kPrefixLength < sizeof(buffer));
            buffer[i + kPrefixLength] = randChar();
        }

        // 如果文件不存在，则返回成功
        auto attr = vfs.GetFileAttribute(buffer);
        if (!attr)
        {
            if (attr.GetError() == make_error_code(errc::no_such_file_or_directory))
            {
                lua_pushstring(L, buffer);
                return 1;
            }

            // 文件系统失败
            luaL_error(L, "Fail to generate temporary filename: %s", attr.GetError().message().c_str());
        }
    }
}

// </editor-fold>
// <editor-fold desc="io.file">

#define LSTG_STREAM_WRAPPER "__IStreamWrapper"

struct IStreamWrapper
{
    Subsystem::VFS::StreamPtr Stream;
    std::string TmpBuffer;  // 供 I/O 方法使用

    /**
     * 读取一行
     * @param dest 目的缓冲区
     * @return 是否读取到一行，如果遇到 EOF，返回 false
     */
    Result<bool> ReadLine(std::string& dest) const noexcept
    {
        if (!Stream)
            return make_error_code(errc::invalid_argument);

        bool eof = false;
        dest.clear();
        try
        {
            while (true) // FIXME: slow
            {
                char ch = '\0';
                auto ret = Stream->Read(reinterpret_cast<uint8_t*>(&ch), 1);
                if (!ret)
                    return ret.GetError();
                if (*ret == 0)
                {
                    eof = true;
                    break;
                }
                if (ch == '\n')
                    break;
                dest.push_back(ch);
            }
        }
        catch (...)
        {
            return make_error_code(errc::not_enough_memory);
        }
        if (eof && dest.empty())
            return false;
        return true;
    }

    /**
     * 从当前位置开始读取内容
     * @param dest 输出目标
     * @param count 期望读取的长度
     * @return 是否成功，如果遇到 EOF，返回 false
     */
    Result<bool> Read(std::string& dest, size_t count) const noexcept
    {
        if (!Stream)
            return make_error_code(errc::invalid_argument);

        bool eof = false;
        dest.clear();
        try
        {
            while (!eof && count > 0)
            {
                static const size_t kBufferStretchSize = 1024;

                auto sz = dest.size();
                auto readSize = std::min(kBufferStretchSize, count);
                dest.resize(sz + readSize);

                auto ret = Stream->Read(reinterpret_cast<uint8_t*>(dest.data() + sz), readSize);
                if (!ret)
                    return ret.GetError();
                assert(*ret <= readSize);
                if (*ret < readSize)
                    eof = true;
                dest.resize(sz + *ret);
                count -= *ret;
            }
        }
        catch (...)
        {
            return make_error_code(errc::not_enough_memory);
        }
        if (eof && dest.empty())
            return false;
        return true;
    }
};

static IStreamWrapper* NewStreamWrapperUserData(lua_State* L, Subsystem::VFS::StreamPtr stream)
{
    auto pf = static_cast<IStreamWrapper*>(lua_newuserdata(L, sizeof(IStreamWrapper)));
    luaL_getmetatable(L, LSTG_STREAM_WRAPPER);
    assert(lua_istable(L, -1));
    lua_setmetatable(L, -2);

    new(pf) IStreamWrapper();
    pf->Stream = std::move(stream);
    return pf;
}

static Result<IStreamWrapper*> OpenStream(lua_State* L, const char* path, Subsystem::VFS::FileAccessMode access,
    Subsystem::VFS::FileOpenFlags openFlags)
{
    auto& vfs = GetVFS();
    auto& scriptSys = GetScriptSystem();

    // 转换到绝对路径
    auto absolutePath = scriptSys.MakeAbsolutePathForIo(path);
    if (!absolutePath)
        return absolutePath.GetError();

    // 打开文件
    auto ret = vfs.OpenFile(absolutePath->ToStringView(), access, openFlags);
    if (!ret)
        return ret.GetError();

    // 创建 UserData
    auto wrapper = NewStreamWrapperUserData(L, std::move(*ret));
    return wrapper;
}

static IStreamWrapper* ConvertToStreamWrapper(lua_State* L, int arg)
{
    return static_cast<IStreamWrapper*>(luaL_checkudata(L, arg, LSTG_STREAM_WRAPPER));
}

static IStreamWrapper* TryConvertToStreamWrapper(lua_State* L, int arg) noexcept
{
    void* p = lua_touserdata(L, arg);
    if (p != nullptr)
    {
        if (lua_getmetatable(L, arg))
        {
            lua_getfield(L, LUA_REGISTRYINDEX, LSTG_STREAM_WRAPPER);
            if (lua_rawequal(L, -1, -2))
            {
                lua_pop(L, 2);
                return static_cast<IStreamWrapper*>(p);
            }
        }
    }
    return nullptr;
}

template <typename T>
inline int PushResult(lua_State* L, Result<T> ret)
{
    if (!ret)
    {
        lua_pushnil(L);
        lua_pushfstring(L, "%s", ret.GetError().message().c_str());
        lua_pushinteger(L, ret.GetError().value());
        return 3;
    }

    if constexpr (!std::is_same_v<T, void>)
    {
        Subsystem::Script::LuaStack st { L };
        return st.PushValue(*ret);
    }
    else
    {
        // 特殊处理以区分 nil
        lua_pushboolean(L, true);
        return 1;
    }
}

static int IoFileClose(lua_State* L)  // file:close
{
    auto wrapper = ConvertToStreamWrapper(L, 1);
    if (!wrapper->Stream)
        luaL_error(L, "stream already disposed");
    wrapper->Stream.reset();
    return 0;
}

static int IoFileFlush(lua_State* L)  // file:flush
{
    auto wrapper = ConvertToStreamWrapper(L, 1);
    if (!wrapper->Stream)
        luaL_error(L, "stream already disposed");
    return PushResult(L, wrapper->Stream->Flush());
}

static int IoFileLines(lua_State* L)  // file:lines
{
    auto iterator = [](lua_State* L) {
        auto wrapper = ConvertToStreamWrapper(L, lua_upvalueindex(1));
        auto autoClose = lua_toboolean(L, lua_upvalueindex(2));
        if (!wrapper->Stream)
            luaL_error(L, "stream already disposed");

        // 读取一行
        auto ret = wrapper->ReadLine(wrapper->TmpBuffer);
        if (!ret)  // Error
        {
            if (autoClose)
                wrapper->Stream.reset();
            luaL_error(L, "%s", ret.GetError().message().c_str());
        }
        if (!*ret)  // EOF
        {
            if (autoClose)
                wrapper->Stream.reset();
            return 0;
        }
        lua_pushlstring(L, wrapper->TmpBuffer.data(), wrapper->TmpBuffer.length());
        return 1;
    };

    ConvertToStreamWrapper(L, 1);
    lua_pushvalue(L, 1);
    lua_pushboolean(L, false);
    lua_pushcclosure(L, iterator, 2);
    return 1;
}

static int IoFileRead(lua_State* L)  // file:read
{
    auto wrapper = ConvertToStreamWrapper(L, 1);
    if (!wrapper->Stream)
        luaL_error(L, "stream already disposed");

    auto n = lua_gettop(L) - 1;
    if (n == 0)
    {
        // 没有参数时，fallback 到单行
        auto ret = wrapper->ReadLine(wrapper->TmpBuffer);
        if (!ret)  // Error
            return PushResult(L, ret);
        if (!*ret)  // EOF
            return 0;
        lua_pushlstring(L, wrapper->TmpBuffer.data(), wrapper->TmpBuffer.length());
        return 1;
    }

    int retCount = 0;
    lua_checkstack(L, n + 3 /* error codes */);
    for (auto i = 0; i < n; ++i)
    {
        auto arg = i + 2;

        // 当参数为数字时，读取一定长度的 buffer
        if (lua_type(L, arg) == LUA_TNUMBER)
        {
            auto readCount = static_cast<size_t>(std::min<lua_Integer>(0, lua_tointeger(L, arg)));
            auto ret = wrapper->Read(wrapper->TmpBuffer, readCount);
            if (!ret)  // Error
                return PushResult(L, ret);
            if (!*ret)  // EOF
            {
                ++retCount;
                lua_pushnil(L);
                break;
            }
            else
            {
                ++retCount;
                lua_pushlstring(L, wrapper->TmpBuffer.data(), wrapper->TmpBuffer.length());
            }
        }
        else
        {
            const char* format = luaL_checkstring(L, arg);
            if (::strcmp(format, "*l") == 0)
            {
                // 读取一行
                auto ret = wrapper->ReadLine(wrapper->TmpBuffer);
                if (!ret)  // Error
                    return PushResult(L, ret);
                if (!*ret)  // EOF
                {
                    ++retCount;
                    lua_pushnil(L);
                    break;
                }
                else
                {
                    ++retCount;
                    lua_pushlstring(L, wrapper->TmpBuffer.data(), wrapper->TmpBuffer.length());
                }
            }
            else if (::strcmp(format, "*a") == 0)
            {
                // 全部读取
                auto ret = wrapper->Read(wrapper->TmpBuffer, static_cast<size_t>(-1));
                if (!ret)  // Error
                    return PushResult(L, ret);
                if (!*ret)  // EOF
                {
                    ++retCount;
                    lua_pushliteral(L, "");
                    break;
                }
                else
                {
                    ++retCount;
                    lua_pushlstring(L, wrapper->TmpBuffer.data(), wrapper->TmpBuffer.length());
                }
            }
            else if (::strcmp(format, "*n") == 0)
            {
                // FIXME: Implement this
                luaL_error(L, "not implemented yet");
            }
            else
            {
                luaL_error(L, "invalid format '%s'", format);
            }
        }
    }
    return retCount;
}

static int IoFileSeek(lua_State* L)  // file:seek
{
    static const Subsystem::VFS::StreamSeekOrigins kMode[] = {
        Subsystem::VFS::StreamSeekOrigins::Begin, Subsystem::VFS::StreamSeekOrigins::Current, Subsystem::VFS::StreamSeekOrigins::End
    };
    static const char* const kModeNames[] = { "set", "cur", "end", nullptr };

    auto wrapper = ConvertToStreamWrapper(L, 1);
    if (!wrapper->Stream)
        luaL_error(L, "stream already disposed");

    int op = luaL_checkoption(L, 2, "cur", kModeNames);
    long offset = luaL_optlong(L, 3, 0);

    auto ret = wrapper->Stream->Seek(offset, kMode[op]);
    if (!ret)
        return PushResult(L, ret);

    return PushResult(L, wrapper->Stream->GetPosition());
}

static int IoFileSetBufferingMode(lua_State* L)  // file:setvbuf
{
    auto wrapper = ConvertToStreamWrapper(L, 1);
    if (!wrapper->Stream)
        luaL_error(L, "stream already disposed");

    // Stream 不支持修改 Buffering，总是返回成功
    return PushResult<void>(L, {});
}

static int IoFileWrite(lua_State* L)  // file:write
{
    auto wrapper = ConvertToStreamWrapper(L, 1);
    if (!wrapper->Stream)
        luaL_error(L, "stream already disposed");

    auto n = lua_gettop(L) - 1;
    for (auto i = 0; i < n; ++i)
    {
        auto arg = i + 2;
        if (lua_type(L, arg) == LUA_TNUMBER)
        {
            char buffer[128];
            ::memset(buffer, 0, sizeof(buffer));
            fmt::format_to(buffer, "{}", lua_tonumber(L, arg));
            auto ret = wrapper->Stream->Write(reinterpret_cast<const uint8_t*>(buffer), ::strlen(buffer));
            if (!ret)
                return PushResult(L, ret);
        }
        else
        {
            size_t l = 0;
            const char* s = ::luaL_checklstring(L, arg, &l);
            auto ret = wrapper->Stream->Write(reinterpret_cast<const uint8_t*>(s), l);
            if (!ret)
                return PushResult(L, ret);
        }
    }
    return PushResult<void>(L, {});
}

static int IoFileGC(lua_State* L)  // file:__gc
{
    auto wrapper = ConvertToStreamWrapper(L, 1);
    wrapper->~IStreamWrapper();
    return 0;
}

static int IoFileToString(lua_State* L)  // file:__tostring
{
    auto wrapper = ConvertToStreamWrapper(L, 1);
    if (wrapper->Stream == nullptr)
        lua_pushliteral(L, "file (closed)");
    else
        lua_pushfstring(L, "file (%p)", wrapper->Stream.get());
    return 1;
}

static const luaL_Reg kIoFileLib[] = {
    { "close", IoFileClose },
    { "flush", IoFileFlush },
    { "lines", IoFileLines },
    { "read", IoFileRead },
    { "seek", IoFileSeek },
    { "setvbuf", IoFileSetBufferingMode },
    { "write", IoFileWrite },
    { "__gc", IoFileGC },
    { "__tostring", IoFileToString },
    { nullptr, nullptr }
};

static void CreateStreamWrapperMetaTable(lua_State* L)
{
    Subsystem::Script::LuaStack st{L};
    Subsystem::Script::LuaStack::BalanceChecker stackChecker(st);

    luaL_newmetatable(L, LSTG_STREAM_WRAPPER);
    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");
    luaL_register(L, nullptr, kIoFileLib);
    lua_pop(L, 1);
}

// </editor-fold>
// <editor-fold desc="io">

static int IoClose(lua_State* L)  // io.close
{
    // 如果第一个参数是 IStreamWrapper
    auto stream = TryConvertToStreamWrapper(L, 1);
    if (stream)
        return IoFileClose(L);

    // 转发给原有实现
    auto f = lua_tocfunction(L, lua_upvalueindex(1));
    assert(f);
    return f(L);
}

static int IoInput(lua_State* L)  // io.input
{
    // FIXME: Implement this
    if (lua_gettop(L) != 0)
        luaL_error(L, "not implemented yet");

    // 转发给原有实现
    auto f = lua_tocfunction(L, lua_upvalueindex(1));
    assert(f);
    return f(L);
}

static int IoLines(lua_State* L)  // io.lines
{
    if (lua_gettop(L) != 0)
    {
        // 此时我们将要打开文件
        lua_settop(L, 1);
        auto stream = OpenStream(L, luaL_checkstring(L, 1), Subsystem::VFS::FileAccessMode::Read, Subsystem::VFS::FileOpenFlags::None);
        if (!stream)
            luaL_error(L, "%s", stream.GetError().message().c_str());
        lua_remove(L, 1);

        // 转发给 file:lines()
        return IoFileLines(L);
    }

    // 转发给原有实现
    auto f = lua_tocfunction(L, lua_upvalueindex(1));
    assert(f);
    return f(L);
}

static int IoOpen(lua_State* L)  // io.open
{
    auto mode = luaL_optstring(L, 2, "r");
    if (::strcmp(mode, "r") == 0 || ::strcmp(mode, "rb") == 0)
    {
        auto stream = OpenStream(L, luaL_checkstring(L, 1), Subsystem::VFS::FileAccessMode::Read, Subsystem::VFS::FileOpenFlags::None);
        if (!stream)
            return PushResult<void>(L, stream.GetError());
        return 1;
    }
    else if (::strcmp(mode, "w") == 0 || ::strcmp(mode, "wb") == 0)
    {
        auto stream = OpenStream(L, luaL_checkstring(L, 1), Subsystem::VFS::FileAccessMode::Write, Subsystem::VFS::FileOpenFlags::Truncate);
        if (!stream)
            return PushResult<void>(L, stream.GetError());
        return 1;
    }
    else if (::strcmp(mode, "a") == 0 || ::strcmp(mode, "ab") == 0)
    {
        auto stream = OpenStream(L, luaL_checkstring(L, 1), Subsystem::VFS::FileAccessMode::Write, Subsystem::VFS::FileOpenFlags::None);
        if (!stream)
            return PushResult<void>(L, stream.GetError());
        auto ret = (*stream)->Stream->Seek(0, Subsystem::VFS::StreamSeekOrigins::End);
        if (!ret)
            return PushResult<void>(L, ret.GetError());
        return 1;
    }
    else if (::strcmp(mode, "r+") == 0 || ::strcmp(mode, "rb+") == 0 || ::strcmp(mode, "r+b") == 0)
    {
        auto stream = OpenStream(L, luaL_checkstring(L, 1), Subsystem::VFS::FileAccessMode::ReadWrite, Subsystem::VFS::FileOpenFlags::None);
        if (!stream)
            return PushResult<void>(L, stream.GetError());
        return 1;
    }
    else if (::strcmp(mode, "w+") == 0 || ::strcmp(mode, "wb+") == 0 || ::strcmp(mode, "w+b") == 0)
    {
        auto stream = OpenStream(L, luaL_checkstring(L, 1), Subsystem::VFS::FileAccessMode::ReadWrite,
            Subsystem::VFS::FileOpenFlags::Truncate);
        if (!stream)
            return PushResult<void>(L, stream.GetError());
        return 1;
    }
    else if (::strcmp(mode, "a+") == 0 || ::strcmp(mode, "ab+") == 0 || ::strcmp(mode, "a+b") == 0)
    {
        auto stream = OpenStream(L, luaL_checkstring(L, 1), Subsystem::VFS::FileAccessMode::ReadWrite, Subsystem::VFS::FileOpenFlags::None);
        if (!stream)
            return PushResult<void>(L, stream.GetError());
        auto ret = (*stream)->Stream->Seek(0, Subsystem::VFS::StreamSeekOrigins::End);
        if (!ret)
            return PushResult<void>(L, ret.GetError());
        return 1;
    }

    luaL_error(L, "unexpected mode '%s'", mode);
    return 0;
}

static int IoOutput(lua_State* L)  // io.output
{
    // FIXME: Implement this
    if (lua_gettop(L) != 0)
        luaL_error(L, "not implemented yet");

    // 转发给原有实现
    auto f = lua_tocfunction(L, lua_upvalueindex(1));
    assert(f);
    return f(L);
}

static int IoPopen(lua_State* L)  // io.popen
{
    // 不支持该操作
    return PushResult<void>(L, make_error_code(errc::not_supported));
}

static int IoTmpFile(lua_State* L)  // io.tmpfile
{
    // 生成临时文件
    OsTmpName(L);
    auto path = luaL_checkstring(L, -1);

    // 此时我们将要打开文件
    auto stream = OpenStream(L, path, Subsystem::VFS::FileAccessMode::ReadWrite, Subsystem::VFS::FileOpenFlags::Truncate);
    if (!stream)
        return PushResult<void>(L, stream.GetError());
    return 1;
}

static int IoType(lua_State* L)  // io.type
{
    // 如果第一个参数是 IStreamWrapper
    auto stream = TryConvertToStreamWrapper(L, 1);
    if (stream)
    {
        if (stream->Stream)
            lua_pushliteral(L, "file");
        else
            lua_pushliteral(L, "closed file");
        return 1;
    }

    // 转发给原有实现
    auto f = lua_tocfunction(L, lua_upvalueindex(1));
    assert(f);
    return f(L);
}

// </editor-fold>
// <editor-fold desc="base">

struct LoadFileInfo
{
    std::variant<FILE*, Subsystem::VFS::StreamPtr> File;
    std::optional<uint8_t> PeekBuffer;
    uint8_t Buffer[LUAL_BUFFERSIZE];
    std::error_code ReadError {};

    Result<uint8_t> GetChar() noexcept
    {
        if (PeekBuffer)
            return *PeekBuffer;

        if (File.index() == 0)
        {
            auto fp = std::get<0>(File);
            auto ret = ::getc(fp);
            if (ret == EOF)
            {
                if (::feof(fp))
                    return make_error_code(errc::result_out_of_range);
                if (::ferror(fp))
                    return make_error_code(errc::io_error);
                assert(false);  // never happend
            }
            return static_cast<uint8_t>(ret);
        }
        else
        {
            assert(File.index() == 1);
            auto s = std::get<1>(File);
            uint8_t b;
            auto ret = s->Read(&b, 1);
            if (!ret)
                return ret.GetError();
            if (*ret == 0)
                return make_error_code(errc::result_out_of_range);
            return b;
        }
    }

    void UngetChar(uint8_t ch) noexcept
    {
        assert(!PeekBuffer);
        PeekBuffer = ch;
    }

    /**
     * 读取字符到缓冲区
     * @return 是否成功，如果遇到 EOF，返回 false
     */
    Result<bool> Read(size_t& sz) noexcept
    {
        sz = 0;
        if (File.index() == 0)
        {
            auto fp = std::get<0>(File);

            if (::feof(fp))
                return false;
            sz = ::fread(Buffer, 1, sizeof(Buffer), fp);
            if (sz > 0)
                return true;
            ReadError = make_error_code(errc::io_error);
            return ReadError;
        }
        else
        {
            assert(File.index() == 1);
            auto s = std::get<1>(File);
            auto ret = s->Read(Buffer, sizeof(Buffer));
            if (!ret)
            {
                ReadError = ret.GetError();
                return ReadError;
            }
            if (*ret == 0)
                return false;
            sz = *ret;
            return true;
        }
    }
};

static const char* FileReader(lua_State* L, void* ud, size_t* size) noexcept
{
    auto* lf = static_cast<LoadFileInfo*>(ud);

    if (lf->PeekBuffer)
    {
        lf->Buffer[0] = *lf->PeekBuffer;
        lf->PeekBuffer.reset();
        *size = 1;
        return reinterpret_cast<const char*>(lf->Buffer);
    }

    auto ret = lf->Read(*size);
    if (!ret)  // Error
        return nullptr;
    if (!*ret)  // EOF
        return nullptr;
    return reinterpret_cast<const char*>(lf->Buffer);
}

static int ReportFileErr(lua_State* L, const char* what, int filenameIndex, std::error_code ec)
{
    const char* filename = lua_tostring(L, filenameIndex) + 1;
    lua_pushfstring(L, "cannot %s %s: %s", what, filename, ec.message().c_str());
    lua_remove(L, filenameIndex);
    return LUA_ERRFILE;
}

static int LoadFileVFS(lua_State* L, const char* filename)
{
    auto& vfs = GetVFS();
    auto& scriptSys = GetScriptSystem();

    // 打开文件流
    LoadFileInfo loadFile;
    int filenameIndex = lua_gettop(L) + 1;
    if (filename == nullptr)
    {
        // 标准输入
        lua_pushliteral(L, "=stdin");
        loadFile.File = stdin;  // 注意 File 不需要 fclose
    }
    else
    {
        // 从 VFS 打开
        lua_pushfstring(L, "@%s", filename);
        auto path = scriptSys.MakeAbsolutePathForIo(filename);
        if (!path)
            return ReportFileErr(L, "open", filenameIndex, path.GetError());
        auto ret = vfs.OpenFile(path->ToStringView(), Subsystem::VFS::FileAccessMode::Read, Subsystem::VFS::FileOpenFlags::None);
        if (!ret)
            return ReportFileErr(L, "open", filenameIndex, ret.GetError());
        loadFile.File = std::move(*ret);
    }

    // Peek 字符以跳过 shebang 或者决定是否是二进制文件
    auto c = loadFile.GetChar();
    if (!c)
        return ReportFileErr(L, "read", filenameIndex, c.GetError());
    if (*c == '#')
    {
        // 跳过 shebang
        while (true)
        {
            c = loadFile.GetChar();
            if (!c)
                return ReportFileErr(L, "read", filenameIndex, c.GetError());
            if (*c == '\n')
                break;
        }
        assert(*c == '\n');
        // 放回一个 '\n' 用于制造空行
    }
    else
    {
        // 此时放回读取的字符
    }
    loadFile.UngetChar(*c);

    // 转发到 lua_load
    auto status = ::lua_load(L, FileReader, &loadFile, lua_tostring(L, filenameIndex));
    if (loadFile.ReadError)
    {
        lua_settop(L, filenameIndex);
        return ReportFileErr(L, "read", filenameIndex, loadFile.ReadError);
    }
    lua_remove(L, filenameIndex);
    return status;
}

static int LoadFile(lua_State* L)
{
    const char* fileName = luaL_optstring(L, 1, nullptr);
    auto status = LoadFileVFS(L, fileName);
    if (status == 0)
    {
        return 1;
    }
    else
    {
        lua_pushnil(L);
        lua_insert(L, -2);
        return 2;
    }
}

static int DoFile(lua_State* L)
{
    const char* fileName = luaL_optstring(L, 1, nullptr);
    int n = lua_gettop(L);
    if (LoadFileVFS(L, fileName) != 0)
        lua_error(L);
    lua_call(L, 0, LUA_MULTRET);
    return lua_gettop(L) - n;
}

static int Print(lua_State* L)
{
    struct LineBuffer
    {
        const char* FileName = nullptr;
        const char* Name = nullptr;
        int Line = 0;

        string Buffer;

        void Flush() noexcept
        {
            lstg::Logging::GetInstance().Log("LUA", LogLevel::Info, lstg::detail::GetLogCurrentTime(), {FileName, Name, Line}, "{}",
                Buffer);
            Buffer.clear();
        }

        Result<void> Append(char ch) noexcept
        {
            try
            {
                if (ch == '\n')
                    Flush();
                else
                    Buffer.push_back(ch);
                return {};
            }
            catch (...)
            {
                return make_error_code(errc::not_enough_memory);
            }
        }

        Result<void> Append(const char* what) noexcept
        {
            try
            {
                while (*what != '\0')
                {
                    auto ch = *(what++);
                    if (ch == '\n')
                    {
                        Flush();
                        continue;
                    }
                    Buffer.push_back(ch);
                }
                return {};
            }
            catch (...)
            {
                return make_error_code(errc::not_enough_memory);
            }
        }
    };

    LineBuffer buffer;
    Subsystem::Script::LuaStack{L}.GetCallerSourceLocation(buffer.FileName, buffer.Name, buffer.Line);

    int n = lua_gettop(L);
    lua_getglobal(L, "tostring");
    for (int i = 1; i <= n; ++i)
    {
        lua_pushvalue(L, -1);
        lua_pushvalue(L, i);
        lua_call(L, 1, 1);
        auto s = lua_tostring(L, -1);
        if (s == nullptr)
            return luaL_error(L, LUA_QL("tostring") " must return a string to " LUA_QL("print"));
        if (i > 1)
        {
            fputs("\t", stdout);
            if (!buffer.Append('\t'))
                return luaL_error(L, "cannot alloc memory");
        }
        fputs(s, stdout);
        if (!buffer.Append(s))
            return luaL_error(L, "cannot alloc memory");
        lua_pop(L, 1);
    }
    fputs("\n", stdout);
    buffer.Flush();
    return 0;
}

// </editor-fold>

void LuaCompatLayer::Register(LuaState& state)
{
    Subsystem::Script::LuaStack::BalanceChecker stackChecker(state);

    // <editor-fold desc="bit">

#ifdef LSTG_PLATFORM_EMSCRIPTEN
    luaopen_bit(state);
    lua_pop(state, 1);
#endif

    // </editor-fold>
    // <editor-fold desc="math">

    // 修补高版本没有 math.mod
    lua_getglobal(state, "math");  // t
    assert(!lua_isnil(state, -1));
    lua_getfield(state, -1, "mod");  // t f|n
    if (lua_isnil(state, -1))
    {
        lua_pop(state, 1);  // t
        lua_getfield(state, -1, "fmod");  // t f
        lua_setfield(state, -2, "mod");  // t
        lua_pop(state, 1);
    }
    else
    {
        lua_pop(state, 2);
    }

    // </editor-fold>
    // <editor-fold desc="os">

    // 修补 os
    lua_getglobal(state, "os");
    assert(!lua_isnil(state, -1));
    lua_pushcfunction(state, OsExecute);
    lua_setfield(state, -2, "execute");
    lua_pushcfunction(state, OsRemove);
    lua_setfield(state, -2, "remove");
    lua_pushcfunction(state, OsRename);
    lua_setfield(state, -2, "rename");
    lua_pushcfunction(state, OsTmpName);
    lua_setfield(state, -2, "tmpname");
    lua_pop(state, 1);

    // </editor-fold>
    // <editor-fold desc="io">

    CreateStreamWrapperMetaTable(state);

    // 修补 io
    lua_getglobal(state, "io");
    assert(!lua_isnil(state, -1));
    lua_getfield(state, -1, "close");  // io.close
    assert(lua_iscfunction(state, -1));
    lua_pushcclosure(state, IoClose, 1);
    lua_setfield(state, -2, "close");
    lua_getfield(state, -1, "input");  // io.input
    assert(lua_iscfunction(state, -1));
    lua_pushcclosure(state, IoInput, 1);
    lua_setfield(state, -2, "input");
    lua_getfield(state, -1, "lines");  // io.lines
    assert(lua_iscfunction(state, -1));
    lua_pushcclosure(state, IoLines, 1);
    lua_setfield(state, -2, "lines");
    lua_pushcfunction(state, IoOpen);  // io.open
    lua_setfield(state, -2, "open");
    lua_getfield(state, -1, "output");  // io.output
    assert(lua_iscfunction(state, -1));
    lua_pushcclosure(state, IoOutput, 1);
    lua_setfield(state, -2, "output");
    lua_pushcfunction(state, IoPopen);  // io.popen
    lua_setfield(state, -2, "popen");
    lua_pushcfunction(state, IoTmpFile);  // io.tmpfile
    lua_setfield(state, -2, "tmpfile");
    lua_getfield(state, -1, "type");  // io.type
    assert(lua_iscfunction(state, -1));
    lua_pushcclosure(state, IoType, 1);
    lua_setfield(state, -2, "type");
    lua_pop(state, 1);

    // </editor-fold>
    // <editor-fold desc="base">

    lua_pushcfunction(state, LoadFile);  // loadfile
    lua_setglobal(state, "loadfile");
    lua_pushcfunction(state, DoFile);  // dofile
    lua_setglobal(state, "dofile");
    lua_pushcfunction(state, Print);  // print
    lua_setglobal(state, "print");

    // </editor-fold>
    // <editor-fold desc="package">

    // 屏蔽所有 loader，增加 preloader
    lua_getglobal(state, "package");
    assert(lua_istable(state, -1));
    lua_getfield(state, -1, "loaders");
    auto cnt = lua_objlen(state, -1);
    for (size_t i = 0; i < cnt; ++i)
    {
        lua_pushnil(state);
        lua_rawseti(state, -2, static_cast<int>(i + 1));
    }
    cnt = lua_objlen(state, -1);
    assert(lua_objlen(state, -1) == 0);
    lua_pushcfunction(state, [](lua_State* L) {  // preloader
        const char* name = luaL_checkstring(L, 1);
        lua_getglobal(L, "package");
        if (!lua_istable(L, -1))
            luaL_error(L, LUA_QL("package") " must be a table");
        lua_getfield(L, -1, "preload");
        if (!lua_istable(L, -1))
            luaL_error(L, LUA_QL("package.preload") " must be a table");
        lua_getfield(L, -1, name);
        if (lua_isnil(L, -1))
            lua_pushfstring(L, "\n\tno field package.preload['%s']", name);
        return 1;
    });
    lua_rawseti(state, -2, 1);
    lua_pop(state, 2);

    // </editor-fold>
}
