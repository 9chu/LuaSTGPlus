/**
 * @file
 * @date 2022/3/3
 * @author 9chu
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#include <lstg/Core/Subsystem/Script/SandBox.hpp>

#include <lstg/Core/Logging.hpp>

using namespace std;
using namespace lstg;
using namespace lstg::Subsystem::Script;

LSTG_DEF_LOG_CATEGORY(SandBox);

SandBox::SandBox(VirtualFileSystem& fs, LuaStack mainThread)
    : m_stFileSystem(fs), m_stMainThread(mainThread)
{
#ifdef LSTG_SHIPPING
    m_dCheckInterval = 0.;
#else
    m_dCheckInterval = 1.;  // 1秒
#endif
}

Result<LuaReference> SandBox::ImportScript(std::string_view path, bool force) noexcept
{
    auto ret = ModuleLocator(path, force);
    if (!ret)
        return ret.GetError();
    return (*ret)->Env;
}

void SandBox::Update(double elapsedTime) noexcept
{
    if (m_dCheckInterval < numeric_limits<double>::epsilon())
        return;

    m_dCheckTimer += elapsedTime;
    if (m_dCheckTimer < m_dCheckInterval)
        return;
    m_dCheckTimer = 0.;

    // 扫描所有文件
    for (auto& it : m_stFiles)
    {
        auto& imported = it.second;
        auto attr = m_stFileSystem.GetFileAttribute(imported.Path.ToStringView());
        if (attr)
        {
            auto mod = attr->LastModified;
            if (mod != imported.LastModified)
            {
                LSTG_LOG_INFO_CAT(SandBox, "Reloading file \"{}\"", imported.Path.ToStringView());
                imported.LastModified = mod;

                ExecFile(imported);
            }
        }
    }
}

Result<Subsystem::VFS::Path> SandBox::MakeFullPath(std::string_view modname) noexcept
{
    try
    {
        return VFS::Path::Normalize(fmt::format("{}/{}", m_stBaseDirectory, modname));
    }
    catch (...)  // bad_alloc
    {
        return make_error_code(errc::not_enough_memory);
    }
}

Result<SandBox::ImportedFile*> SandBox::ModuleLocator(std::string_view modname, bool forceReload) noexcept
{
    LuaStack::BalanceChecker stackChecker(m_stMainThread);

    // 生成完整路径
    Result<VFS::Path> fullPath = MakeFullPath(modname);
    if (!fullPath)
        return fullPath.GetError();

    // 检查文件节点是否已经存在
    ImportedFile* imported = nullptr;
    {
        auto it = m_stFiles.find(fullPath->ToStringView());
        if (it != m_stFiles.end())
            imported = &(it->second);
    }

    // 如果文件已经存在，则直接返回
    if (imported && !forceReload)
        return imported;

    // 检查文件是否存在
    auto fileAttr = m_stFileSystem.GetFileAttribute(fullPath->ToStringView());
    if (!fileAttr)
    {
        auto ec = fileAttr.GetError();
        LSTG_LOG_ERROR_CAT(SandBox, "GetFileAttribute on file \"{}\" failed ({}:{})", fullPath->ToStringView(), ec.category().name(),
            ec.value());
        return ec;
    }

    // 如果文件节点不存在，则创建节点
    if (!imported)
    {
        // 在受保护上下文创建一个环境表和一个透传函数
        // 否则任何 lua 端错误都可能导致 stack unwind
        // 在 emscripten 上可能触发 noexcept 导致 terminate()
        lua_checkstack(m_stMainThread, 3);
        lua_pushcfunction(m_stMainThread, [](lua_State* L) -> int {
            // 构造 ENV 表
            lua_newtable(L);  // t(env)
            lua_newtable(L);  // t(env) t(meta)
            lua_pushliteral(L, "__index");  // t(env) t(meta) s("__index")
            lua_pushvalue(L, LUA_GLOBALSINDEX);  // t(env) t(meta) s("__index") t(_G)
            lua_rawset(L, -3);  // t(env) t(meta)
            lua_setmetatable(L, -2);  // t(env)

            // 构造透传函数
            lua_pushvalue(L, -1);  // t(env) t(env)
            lua_pushcclosure(L, [](lua_State* L) -> int {
                lua_pushvalue(L, lua_upvalueindex(1));
                return 1;
            }, 1);  // t(env) f
            return 2;
        });
        auto call = m_stMainThread.ProtectedCall(0, 2);
        if (!call)
        {
            assert(false);
            lua_pop(m_stMainThread, 1);
            return call.GetError();
        }

        LuaReference envRef(m_stMainThread, -2);
        LuaReference loaderRef(m_stMainThread, -1);
        m_stMainThread.Pop(2);

        try
        {
            ImportedFile newImported;
            newImported.Path = *fullPath;
            newImported.LuaChunkName = fmt::format("@{}", modname);
            newImported.LastModified = fileAttr->LastModified;
            newImported.Env = std::move(envRef);
            newImported.ModuleLoader = std::move(loaderRef);
            auto it = m_stFiles.emplace(fullPath->ToStringView(), std::move(newImported));
            imported = &(it.first->second);
        }
        catch (...)  // bad_alloc
        {
            return make_error_code(errc::not_enough_memory);
        }
    }
    assert(imported);

    // 加载文件
    ExecFile(*imported);

    // 不管加载文件是否成功都返回模块对象
    return imported;
}

Result<void> SandBox::ExecFile(ImportedFile& file)
{
    LuaStack::BalanceChecker stackChecker(m_stMainThread);

    assert(!file.Env.IsEmptyOrNil());

    // 读取文件
    vector<uint8_t> buffer;
    auto ret = m_stFileSystem.ReadFile(buffer, file.Path.ToStringView());
    if (!ret)
    {
        auto ec = ret.GetError();
        LSTG_LOG_ERROR_CAT(SandBox, "Fail to read file \"{}\" ({}:{})", file.Path.ToStringView(), ec.category().name(), ec.value());
        return ec;
    }

    // 加载文件
    lua_checkstack(m_stMainThread, 2);
    auto load = luaL_loadbuffer(m_stMainThread, reinterpret_cast<const char*>(buffer.data()), buffer.size(), file.LuaChunkName.c_str());
    if (load != 0)
    {
        LSTG_LOG_ERROR_CAT(SandBox, "Fail to compile file \"{}\": {}", file.Path.ToStringView(), lua_tostring(m_stMainThread, -1));
        lua_pop(m_stMainThread, 1);
        return make_error_code(static_cast<LuaError>(load));
    }

    // 设置执行环境
    m_stMainThread.PushValue(file.Env);
    lua_setfenv(m_stMainThread, -2);

    // 执行脚本
    auto exec = m_stMainThread.ProtectedCallWithTraceback(0, 0);
    if (!exec)
    {
        LSTG_LOG_ERROR_CAT(SandBox, "Fail to exec file \"{}\": {}", file.Path.ToStringView(), lua_tostring(m_stMainThread, -1));
        lua_pop(m_stMainThread, 1);
        return exec.GetError();
    }

    LSTG_LOG_INFO_CAT(SandBox, "File \"{}\" loaded", file.Path.ToStringView());
    return {};
}
