/**
 * @file
 * @date 2022/3/2
 * @author 9chu
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#include <lstg/Core/Subsystem/ScriptSystem.hpp>

#include <lstg/Core/Logging.hpp>
#include <lstg/Core/Subsystem/Script/LuaPush.hpp>
#include <lstg/Core/Subsystem/SubsystemContainer.hpp>

using namespace std;
using namespace lstg;
using namespace lstg::Subsystem;

LSTG_DEF_LOG_CATEGORY(ScriptSystem);

namespace
{
    /**
     * 注册模块加载器
     * 使得 require 可以通过 VFS 进行 lua 脚本加载
     * @param stack Lua 栈
     * @param self 脚本系统指针
     */
    void ExportModuleLoader(Script::LuaStack& stack, ScriptSystem* self)
    {
        // table.insert( package.loader, 1, Loader )
        stack.GetGlobal("package");  // package
        stack.PushValue("loaders");  // package loaders
        stack.RawGet(-2);  // package package["loaders"]
        auto count = lua_objlen(stack, -1);
        for (int i = static_cast<int>(count); i > 0; --i)
        {
            stack.PushValue(i + 1);  // t t i+1
            stack.PushValue(i);  // t t i+1 i
            stack.RawGet(-3);  // t t i+1 t[i]
            stack.RawSet(-3);  // t t
        }
        stack.PushValue(1);  // t t 1
        stack.PushValue(static_cast<void*>(self));  // t t 1 *
        lua_pushcclosure(stack, [](lua_State* L) -> int {
            auto self = static_cast<ScriptSystem*>(lua_touserdata(L, lua_upvalueindex(1)));
            assert(self);

            // 生成完整路径和块名称
            const char* path = luaL_checkstring(L, 1);
            const char* fullPath = lua_pushfstring(L, "%s/%s.lua",
                self->GetSandBox().GetVirtualFileSystem().GetAssetBaseDirectory().c_str(), path);
            const char* chunkName = lua_pushfstring(L, "@%s", fullPath);

            // 读取文件
            vector<uint8_t> buffer;
            auto ret = self->GetSandBox().GetVirtualFileSystem().ReadFile(buffer, path);
            if (ret)
            {
                // 编译
                luaL_loadbuffer(L, reinterpret_cast<const char*>(buffer.data()), buffer.size(), chunkName);
                return 1;
            }
            
            lua_pushfstring(L, "open file \"%s\" fail: %s", path, ret.GetError().message().c_str());
            return 1;
        }, 1);  // t t 1 f
        stack.RawSet(-3);  // t t
        stack.Pop(2);
    }

    /**
     * 注册全局的 import 方法
     * @param stack Lua 栈
     * @param self 脚本系统指针
     */
    void ExportImport(Script::LuaStack& stack, ScriptSystem* self)
    {
        stack.PushValue(static_cast<void*>(self));
        lua_pushcclosure(stack, [](lua_State* L) -> int {
            auto self = static_cast<ScriptSystem*>(lua_touserdata(L, lua_upvalueindex(1)));
            assert(self);

            const char* path = luaL_checkstring(L, 1);

            // 转发给 SandBox
            Script::LuaStack st(L);
            auto ret = self->GetSandBox().ImportScript(path);
            if (ret)
            {
                st.PushValue(*ret);
                return 1;
            }
            
            st.Error("import file \"%s\" fail: %s", path, ret.GetError().message().c_str());
        }, 1);
        stack.SetGlobal("import");
    }
}

ScriptSystem::ScriptSystem(SubsystemContainer& container)
    : m_stSandBox(*(container.Get<VirtualFileSystem>()), m_stState)
{
    Script::LuaStack::BalanceChecker stackChecker(m_stState);

#ifndef LSTG_PLATFORM_EMSCRIPTEN
    // 打开 JIT
    if (0 == luaJIT_setmode(m_stState, 0, LUAJIT_MODE_ENGINE | LUAJIT_MODE_ON))
        LSTG_LOG_WARN_CAT(ScriptSystem, "Unable to turn on JIT engine");
#endif

    // 打开标准库
    m_stState.OpenStandardLibrary();

    // 增加 require 和 import 方法
    ExportModuleLoader(m_stState, this);
    ExportImport(m_stState, this);
}

Result<void> ScriptSystem::LoadScript(std::string_view path, bool sandbox) noexcept
{
    // 沙箱模式加载
    if (sandbox)
    {
        auto ret = m_stSandBox.ImportScript(path, true);
        if (!ret)
            return ret.GetError();
        return {};
    }

    Script::LuaStack::BalanceChecker stackChecker(m_stState);

    // 生成完整路径
    string fullPath;
    try
    {
        fullPath = fmt::format("{}/{}", m_stSandBox.GetVirtualFileSystem().GetAssetBaseDirectory(), path);
    }
    catch (...)  // bad_alloc
    {
        return make_error_code(errc::not_enough_memory);
    }

    // 读取文件
    vector<uint8_t> buffer;
    auto ret = GetSandBox().GetVirtualFileSystem().ReadFile(buffer, fullPath);
    if (!ret)
        return ret.GetError();

    // 编译
    try
    {
        string chunkName = fmt::format("@{}", path);
        auto load = luaL_loadbuffer(m_stState, reinterpret_cast<const char*>(buffer.data()), buffer.size(), chunkName.c_str());
        if (load != 0)
        {
            LSTG_LOG_ERROR_CAT(ScriptSystem, "Fail to compile \"{}\": {}", path, lua_tostring(m_stState, -1));
            lua_pop(m_stState, 1);
            return make_error_code(static_cast<Script::LuaError>(load));
        }
    }
    catch (...)  // bad_alloc
    {
        return make_error_code(errc::not_enough_memory);
    }

    // 执行
    auto call = m_stState.ProtectedCallWithTraceback(0, 0);
    if (!call)
    {
        LSTG_LOG_ERROR_CAT(ScriptSystem, "Fail to exec \"{}\": {}", path, lua_tostring(m_stState, -1));
        lua_pop(m_stState, 1);
        return call.GetError();
    }

    LSTG_LOG_TRACE_CAT(ScriptSystem, "File \"{}\" loaded", path);
    return {};
}

void ScriptSystem::OnUpdate(double elapsedTime) noexcept
{
    m_stSandBox.Update(elapsedTime);
}

void ScriptSystem::LogCallFail(const char* func, const char* what) noexcept
{
    LSTG_LOG_ERROR_CAT(ScriptSystem, "Fail to call \"{}\": {}", func, what);
}
