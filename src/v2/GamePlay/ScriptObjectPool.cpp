/**
 * @file
 * @date 2022/7/26
 * @author 9chu
 * 此文件为 LuaSTGPlus 项目的一部分，版权与许可声明详见 COPYRIGHT.txt。
 */
#include <lstg/v2/GamePlay/ScriptObjectPool.hpp>

#include <lstg/Core/Logging.hpp>
#include <lstg/Core/Subsystem/Script/LuaPush.hpp>

using namespace std;
using namespace lstg;
using namespace lstg::v2::GamePlay;

using namespace lstg::Subsystem::Script;

LSTG_DEF_LOG_CATEGORY(ScriptObjectPool);

namespace
{
#if !defined(LUA_VERSION_NUM) || LUA_VERSION_NUM < 502
    static void luaL_setfuncs(lua_State *l, const luaL_Reg *reg, int nup) noexcept
    {
        int i;
        luaL_checkstack(l, nup, "too many upvalues");
        for (; reg->name != nullptr; reg++)
        {
            /* fill the table with given functions */
            for (i = 0; i < nup; i++)  /* copy upvalues to the top */
                lua_pushvalue(l, -nup);
            lua_pushcclosure(l, reg->func, nup);  /* closure with those upvalues */
            lua_setfield(l, -(nup + 2), reg->name);
        }
        lua_pop(l, nup);  /* remove upvalues */
    }
#endif
}

static const size_t kMaxObjectCount = std::numeric_limits<uint16_t>::max();

const char* v2::GamePlay::ToString(ScriptCallbackFunctions functions) noexcept
{
    switch (functions)
    {
        case ScriptCallbackFunctions::OnInit:
            return "OnInit";
        case ScriptCallbackFunctions::OnDelete:
            return "OnDelete";
        case ScriptCallbackFunctions::OnFrame:
            return "OnFrame";
        case ScriptCallbackFunctions::OnRender:
            return "OnRender";
        case ScriptCallbackFunctions::OnCollision:
            return "OnCollision";
        case ScriptCallbackFunctions::OnKill:
            return "OnKill";
        default:
            assert(false);
            return "<unknown>";
    }
}

ScriptObjectPool::ScriptObjectPool(Subsystem::Script::LuaState& state, IScriptObjectBridge* bridge)
    : m_stState(state), m_pBridge(bridge)
{
#ifdef LSTG_DEVELOPMENT
    LuaStack::BalanceChecker checker(m_stState);
#endif

    // 在保护上下文完成脚本环境初始化
    m_stState.ProtectedCallNative([&](lua_State* L) {
        // 创建对象表
        ::lua_createtable(L, kMaxObjectCount / 2, 0);  // t
        m_stObjectTableRef = LuaReference(L, -1);

        // 创建元表
        ::lua_createtable(L, 0, 2);  // t t t
        luaL_Reg methods[] = {
            {
                "__index", [](lua_State* L) -> int {
                    // 取出自己
                    auto self = static_cast<ScriptObjectPool*>(lua_touserdata(L, lua_upvalueindex(1)));
                    assert(self && self->m_pBridge);

                    // 取出对象 ID
                    luaL_checktype(L, 1, LUA_TTABLE);  // t k ...
                    auto key = luaL_checkstring(L, 2);
                    lua_rawgeti(L, 1, kIndexOfScriptObjectIdInObject);  // t k ... i(ID)
                    auto id = static_cast<ScriptObjectId>(lua_tointeger(L, -1));
                    lua_settop(L, 2);  // t k

                    // 转换到 EntityID
                    auto it = self->m_stEntityIdMapping.find(id);
                    if (it == self->m_stEntityIdMapping.end())
                        luaL_error(L, "entity is already disposed, sid=%d", static_cast<int>(id));

                    // 调用 Bridge 方法
                    return self->m_pBridge->OnGetAttribute(L, it->second, key);
                },
            },
            {
                "__newindex", [](lua_State* L) -> int {
                    // 取出自己
                    auto self = static_cast<ScriptObjectPool*>(lua_touserdata(L, lua_upvalueindex(1)));
                    assert(self && self->m_pBridge);

                    // 取出对象 ID
                    luaL_checktype(L, 1, LUA_TTABLE);  // t k v ...
                    auto key = luaL_checkstring(L, 2);
                    lua_rawgeti(L, 1, kIndexOfScriptObjectIdInObject);  // t k v ... i(ID)
                    auto id = static_cast<ScriptObjectId>(lua_tointeger(L, -1));
                    lua_settop(L, 3);  // t k v

                    // 转换到 EntityID
                    auto it = self->m_stEntityIdMapping.find(id);
                    if (it == self->m_stEntityIdMapping.end())
                        luaL_error(L, "entity is already disposed, sid=%d", static_cast<int>(id));

                    // 调用 Bridge 方法
                    if (!self->m_pBridge->OnSetAttribute(L, it->second, key, LuaStack::AbsIndex(3)))
                        lua_rawset(L, 1);
                    return 0;
                },
            },
            { nullptr, nullptr },
        };
        ::lua_pushlightuserdata(L, this);  // t t t p(this)
        ::luaL_setfuncs(L, methods, 1);  // t t t
        m_stObjectMetaTableRef = LuaReference(L, -1);
    }).ThrowIfError();
}

ScriptObjectPool::~ScriptObjectPool()
{
#ifdef LSTG_DEVELOPMENT
    LuaStack::BalanceChecker checker(m_stState);
#endif

    // 清理 MetaTable，防止卸载后用户执行相关方法
    m_stState.PushValue(m_stObjectMetaTableRef);
    assert(m_stState.TypeOf(-1) == LUA_TTABLE);
    m_stState.RawSet(-1, "__index", nullptr_t{});
    m_stState.RawSet(-1, "__newindex", nullptr_t{});
    m_stState.Pop(1);
}

Result<std::tuple<ScriptObjectId, Subsystem::Script::LuaStack::AbsIndex>> ScriptObjectPool::Alloc(Subsystem::Script::LuaStack stack,
    Subsystem::Script::LuaStack::AbsIndex classIndex, ECS::EntityId id) noexcept
{
    assert(stack.TypeOf(classIndex) == LUA_TTABLE);

    // 上限检查
    if (m_uCurrentObjects >= kMaxObjectCount)
    {
        LSTG_LOG_ERROR_CAT(ScriptObjectPool, "Too many objects, cnt={}", m_uCurrentObjects);
        return make_error_code(errc::not_enough_memory);
    }

    // 分配ID
    auto scriptId = ++m_uNextObjectId;
    try
    {
        assert(m_stEntityIdMapping.find(scriptId) == m_stEntityIdMapping.end());
        m_stEntityIdMapping.emplace(scriptId, id);
    }
    catch (...)  // bad_alloc
    {
        LSTG_LOG_ERROR_CAT(ScriptObjectPool, "Alloc memory fail");
        return make_error_code(errc::not_enough_memory);
    }

    // 分配对象
    // FIXME: 这里的 lua 侧错误无法捕获进行处理
#ifdef LSTG_DEVELOPMENT
    auto top = stack.GetTop();
#endif
    lua_checkstack(stack, 3);
    stack.PushValue(m_stObjectTableRef);  // ... t(objectTable)
    assert(!stack.RawHas(-1, static_cast<int>(scriptId)));
    lua_createtable(stack, 2, 0);  // ... t(objectTable) t(object)
    stack.PushValue(classIndex);  // ... t(objectTable) t(object) t(classTable)
    stack.RawSet(-2, kIndexOfClassInObject);  // ... t(objectTable) t(object)
    stack.PushValue(static_cast<int>(scriptId));  // ... t(objectTable) t(object) i(scriptId)
    stack.RawSet(-2, kIndexOfScriptObjectIdInObject);  // ... t(objectTable) t(object)
    stack.PushValue(m_stObjectMetaTableRef);  // ... t(objectTable) t(object) t(metaTable)
    ::lua_setmetatable(stack, -2);  // ... t(objectTable) t(object)
    ::lua_pushvalue(stack, -1);  // ... t(objectTable) t(object) t(object)
    stack.RawSet(-3, static_cast<int>(scriptId));  // ... t(objectTable) t(object)
    stack.Remove(-2);  // ... t(object)
#ifdef LSTG_DEVELOPMENT
    assert(stack.GetTop() == top + 1);
#endif

    ++m_uCurrentObjects;
    return make_tuple(scriptId, LuaStack::AbsIndex(stack.GetTop()));
}

void ScriptObjectPool::Free(Subsystem::Script::LuaStack stack, ScriptObjectId scriptId) noexcept
{
#ifdef LSTG_DEVELOPMENT
    LuaStack::BalanceChecker checker(stack);
#endif

    // 释放ID
    auto it = m_stEntityIdMapping.find(scriptId);
    assert(it != m_stEntityIdMapping.end());
    m_stEntityIdMapping.erase(it);

    // 从 Lua 表删除
    lua_checkstack(stack, 2);
    stack.PushValue(m_stObjectTableRef);  // ... t(objectTable)
    assert(stack.RawHas(-1, static_cast<int>(scriptId)));
    stack.PushValue(nullptr_t {});  // ... t(objectTable) n
    stack.RawSet(-2, static_cast<int>(scriptId));  // ... t(objectTable)
    stack.Pop(1);

    --m_uCurrentObjects;
}

void ScriptObjectPool::PushScriptObject(Subsystem::Script::LuaStack stack, ScriptObjectId scriptId) noexcept
{
    // 获取对象
    stack.PushValue(m_stObjectTableRef);  // ... t(objectTable)
    assert(stack.TypeOf(-1) == LUA_TTABLE);
    stack.RawGet(-1, static_cast<int>(scriptId));  // ...t(objectTable) t(object)
    stack.Remove(-2);  // ... t(object)
    assert(stack.TypeOf(-1) == LUA_TTABLE || stack.TypeOf(-1) == LUA_TNIL);
}

ScriptCallbackInvokeResult ScriptObjectPool::InvokeCallback(Subsystem::Script::LuaStack stack, ScriptObjectId scriptId,
    ScriptCallbackFunctions callback, unsigned args) noexcept
{
#ifdef LSTG_DEVELOPMENT
    auto checkTop = stack.GetTop();
#endif

    // 获取对象
    lua_checkstack(stack, 5);
    PushScriptObject(stack, static_cast<int>(scriptId));  // ... {args...} t(object)
    if (stack.TypeOf(-1) != LUA_TTABLE)
    {
        LSTG_LOG_ERROR_CAT(ScriptObjectPool, "Entity is already disposed, sid={}", scriptId);
        lua_pop(stack, args + 1);
        return ScriptCallbackInvokeResult::Disposed;
    }
    stack.RawGet(-1, kIndexOfClassInObject);  // ... {args...} t(object) t(class)
    if (stack.TypeOf(-1) != LUA_TTABLE)
    {
        LSTG_LOG_ERROR_CAT(ScriptObjectPool, "Invalid class object, sid={}, type={}", scriptId, stack.TypeOf(-1));
        lua_pop(stack, args + 2);
        return ScriptCallbackInvokeResult::InvalidClass;
    }
    stack.RawGet(-1, static_cast<int>(callback));  // ... {args...} t(object) t(class) f(callback)
    if (stack.TypeOf(-1) == LUA_TNIL)
    {
        lua_pop(stack, args + 3);
        return ScriptCallbackInvokeResult::CallbackNotDefined;
    }
    else if (stack.TypeOf(-1) != LUA_TFUNCTION)
    {
        LSTG_LOG_ERROR_CAT(ScriptObjectPool, "Attempt to call non-function callback \"{}\" on entity, sid={}", ToString(callback),
            scriptId);
        lua_pop(stack, args + 3);
        return ScriptCallbackInvokeResult::InvalidCallback;
    }

    // 准备调用
    auto top = stack.GetTop();
    assert(top >= args + 3u);
    stack.Insert(top - (args + 3u - 1u));  // ... f(callback) {args...} t(object) t(class)
    lua_pop(stack, 1);  // ... f(callback) {args...} t(object)
    stack.Insert((top - 1u) - (args + 1u - 1u));  // ... f(callback) t(object) {args...}
    auto ret = stack.ProtectedCallWithTraceback(args + 1, 0);  // ...
    if (!ret)
    {
        LSTG_LOG_ERROR_CAT(ScriptObjectPool, "Uncaught error in callback \"{}\" on entity {}: {}", ToString(callback), scriptId,
            lua_tostring(stack, -1));
        lua_pop(stack, 1);
        return ScriptCallbackInvokeResult::CallError;
    }

#ifdef LSTG_DEVELOPMENT
    assert(stack.GetTop() == checkTop - args);
#endif
    return ScriptCallbackInvokeResult::Ok;
}

std::optional<ECS::EntityId> ScriptObjectPool::GetEntityId(ScriptObjectId scriptObjectId) noexcept
{
    auto it = m_stEntityIdMapping.find(scriptObjectId);
    if (it == m_stEntityIdMapping.end())
        return nullopt;
    return it->second;
}
