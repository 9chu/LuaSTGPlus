/**
 * @file
 * @author 9chu
 * @date 2022/9/27
 * 此文件为 LuaSTGPlus 项目的一部分，版权与许可声明详见 COPYRIGHT.txt。
 */
#include <lstg/v2/Bridge/LfsBridgeModule.hpp>

#include <lstg/Core/Logging.hpp>
#include <lstg/Core/AppBase.hpp>
#include <lstg/Core/Subsystem/ScriptSystem.hpp>
#include <lstg/Core/Subsystem/VirtualFileSystem.hpp>

using namespace std;
using namespace lstg;
using namespace lstg::v2::Bridge;

template <typename... TArgs>
using Unpack = Subsystem::Script::Unpack<TArgs...>;

using AbsIndex = Subsystem::Script::LuaStack::AbsIndex;
using LuaStack = Subsystem::Script::LuaStack;

using Path = Subsystem::VFS::Path;

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

// <editor-fold desc="LfsDirIterator">

LfsDirIterator::LfsDirIterator(Subsystem::VFS::DirectoryIteratorPtr visitor)
    : m_pVisitor(std::move(visitor))
{}

Subsystem::Script::Unpack<std::optional<std::string>, std::optional<std::string>> LfsDirIterator::Next()
{
    auto ret = m_pVisitor->Next();
    if (!ret)
        return { optional<string> {}, fmt::format("{}", ret.GetError()) };

    auto filename = m_pVisitor->GetName().ToString();
    if (filename.empty())  // 迭代完成返回空文件名
        return { optional<string> {}, {} };
    return { m_pVisitor->GetName().ToString(), {} };
}

void LfsDirIterator::Close()
{
    // 无行为
}

// </editor-fold>

// <editor-fold desc="LfsBridgeModule">

Unpack<std::variant<std::nullptr_t, AbsIndex>, std::optional<std::string>> LfsBridgeModule::GetFileInfo(LuaStack& st, std::string_view path,
    std::optional<std::string_view> field)
{
    auto& vfs = GetVFS();
    auto& scriptSys = GetScriptSystem();

    auto target = scriptSys.MakeAbsolutePathForIo(path);
    if (!target)
        st.Error("cannot alloc memory");
    auto ret = vfs.GetFileAttribute(target->ToStringView());
    if (!ret)
    {
        auto errMsg = fmt::format("Unable to stat '{}': {}", path, ret.GetError());
        return { nullptr_t {}, std::move(errMsg) };
    }

    // 返回单个属性
    if (field)
    {
        if (*field == "mode")
            st.PushValue("");
        else if (*field == "dev" || *field == "ino" || *field == "nlink" || *field == "uid" || *field == "gid" || *field == "rdev")
            st.PushValue(0);
        else if (*field == "access" || *field == "modification" || *field == "change")
            st.PushValue(static_cast<int64_t>(ret->LastModified));
        else if (*field == "size")
            st.PushValue(ret->Size);
        else if (*field == "permissions")  // fake 777
            st.PushValue("rwxrwxrwx");
        else
            st.Error("invalid attribute name %s", *field);
        return { AbsIndex(st.GetTop()), optional<string> {} };
    }

    // 返回一个 table
    st.NewTable();
    st.SetField(-1, "mode", "");
    st.SetField(-1, "dev", 0);
    st.SetField(-1, "ino", 0);
    st.SetField(-1, "nlink", 0);
    st.SetField(-1, "uid", 0);
    st.SetField(-1, "gid", 0);
    st.SetField(-1, "rdev", 0);
    st.SetField(-1, "access", static_cast<int64_t>(ret->LastModified));
    st.SetField(-1, "modification", static_cast<int64_t>(ret->LastModified));
    st.SetField(-1, "change", static_cast<int64_t>(ret->LastModified));
    st.SetField(-1, "size", ret->Size);
    st.SetField(-1, "permissions", "rwxrwxrwx");
    return { AbsIndex(st.GetTop()), optional<string> {} };
}

Unpack<bool, std::optional<std::string>> LfsBridgeModule::ChangeDir(LuaStack& st, std::string_view path)
{
    auto& vfs = GetVFS();
    auto& scriptSys = GetScriptSystem();

    auto target = scriptSys.MakeAbsolutePathForIo(path);
    if (!target)
        st.Error("cannot alloc memory");
    auto ret = vfs.GetFileAttribute(target->ToStringView());
    if (!ret)
    {
        auto errMsg = fmt::format("Unable to change working directory to '{}': {}", path, ret.GetError());
        return { false, std::move(errMsg) };
    }
    if (ret->Type != Subsystem::VFS::FileType::Directory)
    {
        auto errMsg = fmt::format("Unable to change working directory to '{}': not a directory", path);
        return { false, std::move(errMsg) };
    }

    scriptSys.SetIoWorkingDirectory(target->ToString());
    return { true, optional<string> {} };
}

std::string_view LfsBridgeModule::GetDir(LuaStack& st)
{
    auto& scriptSys = GetScriptSystem();
    return scriptSys.GetIoWorkingDirectory();
}

Unpack<AbsIndex, LfsDirIterator> LfsBridgeModule::IterateDir(LuaStack& st, std::string_view path)
{
    auto& vfs = GetVFS();
    auto& scriptSys = GetScriptSystem();

    auto target = scriptSys.MakeAbsolutePathForIo(path);
    if (!target)
        st.Error("cannot alloc memory");
    auto ret = vfs.VisitDirectory(target->ToStringView());
    if (!ret)
    {
        // path 总是以 '\0' 结尾
        st.Error("cannot visit '%s': %s", path.data(), ret.GetError().message().c_str());
    }

    st.PushValue(&LfsDirIterator::Next);
    return { AbsIndex(st.GetTop()), LfsDirIterator(std::move(*ret)) };
}

Unpack<std::nullptr_t, std::string_view> LfsBridgeModule::MakeLink(LuaStack& st)
{
    return { {}, "make_link is not supported on LuaSTGPlus" };
}

Unpack<std::nullptr_t, std::string_view> LfsBridgeModule::FileLock(LuaStack& st)
{
    return { {}, "lock is not supported on LuaSTGPlus" };
}

Unpack<bool, std::optional<std::string>> LfsBridgeModule::MakeDir(LuaStack& st, std::string_view path)
{
    auto& vfs = GetVFS();
    auto& scriptSys = GetScriptSystem();

    auto target = scriptSys.MakeAbsolutePathForIo(path);
    if (!target)
        st.Error("cannot alloc memory");
    auto ret = vfs.CreateDirectory(target->ToStringView());
    if (!ret)
    {
        auto errMsg = fmt::format("Unable to make directory to '{}': {}", path, ret.GetError());
        return { false, std::move(errMsg) };
    }
    return { true, optional<string> {} };
}

Unpack<bool, std::optional<std::string>> LfsBridgeModule::RemoveDir(LuaStack& st, std::string_view path)
{
    auto& vfs = GetVFS();
    auto& scriptSys = GetScriptSystem();

    auto target = scriptSys.MakeAbsolutePathForIo(path);
    if (!target)
        st.Error("cannot alloc memory");

    // 确保是文件夹
    auto ret = vfs.GetFileAttribute(target->ToStringView());
    if (!ret)
    {
        auto errMsg = fmt::format("Unable to remove directory '{}': {}", path, ret.GetError());
        return { false, std::move(errMsg) };
    }
    if (ret->Type != Subsystem::VFS::FileType::Directory)
    {
        auto errMsg = fmt::format("Unable to remove directory '{}': not a directory", path);
        return { false, std::move(errMsg) };
    }

    // 删除
    auto ret2 = vfs.Remove(target->ToStringView());
    if (!ret2)
    {
        auto errMsg = fmt::format("Unable to remove directory '{}': {}", path, ret.GetError());
        return { false, std::move(errMsg) };
    }
    return { true, optional<string> {} };
}

Unpack<std::nullptr_t, std::string_view> LfsBridgeModule::GetLinkInfo(LuaStack& st)
{
    return { {}, "symlinkattributes is not supported on LuaSTGPlus" };
}

Unpack<std::nullptr_t, std::string_view> LfsBridgeModule::SetMode(LuaStack& st)
{
    return { {}, "setmode is not supported on LuaSTGPlus" };
}

Unpack<std::nullptr_t, std::string_view> LfsBridgeModule::Touch(LuaStack& st)
{
    return { {}, "touch is not supported on LuaSTGPlus" };
}

Unpack<std::nullptr_t, std::string_view> LfsBridgeModule::Unlock(LuaStack& st)
{
    return { {}, "unlock is not supported on LuaSTGPlus" };
}

Unpack<std::nullptr_t, std::string_view> LfsBridgeModule::LockDir(LuaStack& st)
{
    return { {}, "lock_dir is not supported on LuaSTGPlus" };
}

Unpack<bool, std::optional<std::string>> LfsBridgeModule::RemoveFile(LuaStack& st, std::string_view path)
{
    auto& vfs = GetVFS();
    auto& scriptSys = GetScriptSystem();

    auto target = scriptSys.MakeAbsolutePathForIo(path);
    if (!target)
        st.Error("cannot alloc memory");

    // 确保是文件
    auto ret = vfs.GetFileAttribute(target->ToStringView());
    if (!ret)
    {
        auto errMsg = fmt::format("Unable to delete file '{}': {}", path, ret.GetError());
        return { false, std::move(errMsg) };
    }
    if (ret->Type != Subsystem::VFS::FileType::RegularFile)
    {
        auto errMsg = fmt::format("Unable to delete file '{}': not a file", path);
        return { false, std::move(errMsg) };
    }

    // 删除
    auto ret2 = vfs.Remove(target->ToStringView());
    if (!ret2)
    {
        auto errMsg = fmt::format("Unable to delete file '{}': {}", path, ret.GetError());
        return { false, std::move(errMsg) };
    }
    return { true, optional<string> {} };
}

// </editor-fold>
