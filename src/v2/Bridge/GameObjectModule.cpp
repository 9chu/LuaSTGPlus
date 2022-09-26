/**
 * @file
 * @author 9chu
 * @date 2022/4/17
 * 此文件为 LuaSTGPlus 项目的一部分，版权与许可声明详见 COPYRIGHT.txt。
 */
#include <lstg/v2/Bridge/GameObjectModule.hpp>

#include <glm/gtx/norm.hpp>
#include <lstg/Core/Logging.hpp>
#include <lstg/v2/GamePlay/GameWorld.hpp>
#include <lstg/v2/GamePlay/Components/Transform.hpp>
#include <lstg/v2/GamePlay/Components/Movement.hpp>
#include <lstg/v2/GamePlay/Components/Renderer.hpp>
#include <lstg/v2/GamePlay/Components/Collider.hpp>
#include <lstg/v2/GamePlay/Components/LifeTime.hpp>
#include <lstg/v2/GamePlay/Components/Script.hpp>
#include "detail/Helper.hpp"

using namespace std;
using namespace lstg;
using namespace lstg::v2::Bridge;

using namespace lstg::Subsystem::Script;
using namespace lstg::v2;
using namespace lstg::v2::GamePlay;

LSTG_DEF_LOG_CATEGORY(GameObjectModule);

void GameObjectModule::GetObjectTable()
{
    LSTG_LOG_DEPRECATED(GameObjectModule, ObjTable);
}

int32_t GameObjectModule::GetObjectCount()
{
    auto& world = detail::GetGlobalApp().GetDefaultWorld();
    return static_cast<int32_t>(world.GetScriptObjectPool().GetCurrentObjects());
}

void GameObjectModule::UpdateObjectList()
{
    LSTG_LOG_DEPRECATED(GameObjectModule, UpdateObjList);
}

void GameObjectModule::UpdateObjects(LuaStack& stack)
{
    auto& world = detail::GetGlobalApp().GetDefaultWorld();
    if (!world.GetScriptObjectPool().IsOnMainThread(stack))
        stack.Error("ObjFrame should be called on main thread");
    world.Frame();
}

void GameObjectModule::RenderObjects(LuaStack& stack)
{
    auto& world = detail::GetGlobalApp().GetDefaultWorld();
    if (!world.GetScriptObjectPool().IsOnMainThread(stack))
        stack.Error("ObjRender should be called on main thread");
    world.Render();
}

void GameObjectModule::SetBound(double left, double right, double bottom, double top)
{
    auto& world = detail::GetGlobalApp().GetDefaultWorld();
    world.SetBoundary({left, top, std::abs(right - left), std::abs(top - bottom)});
}

void GameObjectModule::BoundCheck(LuaStack& stack)
{
    auto& world = detail::GetGlobalApp().GetDefaultWorld();
    if (!world.GetScriptObjectPool().IsOnMainThread(stack))
        stack.Error("BoundCheck should be called on main thread");
    world.DeleteOutOfBoundaryEntities();
}

void GameObjectModule::CollisionCheck(LuaStack& stack, int32_t groupIdA, int32_t groupIdB)
{
    auto& world = detail::GetGlobalApp().GetDefaultWorld();
    if (!world.GetScriptObjectPool().IsOnMainThread(stack))
        stack.Error("CollisionCheck should be called on main thread");
    if (groupIdA >= static_cast<int32_t>(Components::kColliderGroupCount) ||
        groupIdB >= static_cast<int32_t>(Components::kColliderGroupCount) || groupIdA < 0 || groupIdB < 0)
    {
        stack.Error("Invalid group id");
    }
    world.CollisionCheck(groupIdA, groupIdB);
}

void GameObjectModule::UpdateXY(LuaStack& stack)
{
    auto& world = detail::GetGlobalApp().GetDefaultWorld();
    if (!world.GetScriptObjectPool().IsOnMainThread(stack))
        stack.Error("ObjRender should be called on main thread");
    world.UpdateCoordinate();
}

void GameObjectModule::AfterFrame(LuaStack& stack)
{
    auto& world = detail::GetGlobalApp().GetDefaultWorld();
    if (!world.GetScriptObjectPool().IsOnMainThread(stack))
        stack.Error("ObjRender should be called on main thread");
    world.AfterFrame();
}

LuaStack::AbsIndex GameObjectModule::NewObject(LuaStack& stack, AbsIndex cls)
{
    assert(cls == 1);

    // 检查参数
    if (stack.TypeOf(cls) != LUA_TTABLE)
        stack.Error("invalid argument #1, luastg object class required for 'New'.");
    stack.GetField(cls, "is_class");  // t(class) ... b
    if (!lua_toboolean(stack, -1))
        stack.Error("invalid argument #1, luastg object class required for 'New'.");
    stack.Pop(1);  // t(class) ...

    // 创建对象
    auto& world = detail::GetGlobalApp().GetDefaultWorld();
    auto ret = world.CreateEntity(stack, cls);
    if (!ret)
        stack.Error("fail to create entity");
    return *ret;
}

void GameObjectModule::DelObject(LuaStack& stack, AbsIndex object)
{
    assert(object == 1);

    // 检查参数
    if (stack.TypeOf(object) != LUA_TTABLE)
        stack.Error("invalid argument #1, luastg object required for 'Del'.");
    stack.RawGet(object, kIndexOfScriptObjectIdInObject);  // t(object) ... n(id)
    auto id = static_cast<ScriptObjectId>(luaL_checkinteger(stack, -1));
    stack.Pop(1);  // t(object) ...

    // 调用方法
    auto& world = detail::GetGlobalApp().GetDefaultWorld();
    assert(stack.GetTop() >= object.Index);
    if (!world.DeleteEntity(stack, id, stack.GetTop() - object))
        stack.Error("invalid argument #1, invalid luastg object.");
}

void GameObjectModule::KillObject(LuaStack& stack, AbsIndex object)
{
    assert(object == 1);

    // 检查参数
    if (stack.TypeOf(object) != LUA_TTABLE)
        stack.Error("invalid argument #1, luastg object required for 'Kill'.");
    stack.RawGet(object, kIndexOfScriptObjectIdInObject);  // t(object) ... n(id)
    auto id = static_cast<ScriptObjectId>(luaL_checkinteger(stack, -1));
    stack.Pop(1);  // t(object) ...

    // 调用方法
    auto& world = detail::GetGlobalApp().GetDefaultWorld();
    assert(stack.GetTop() >= object.Index);
    if (!world.KillEntity(stack, id, stack.GetTop() - object))
        stack.Error("invalid argument #1, invalid luastg object.");
}

bool GameObjectModule::IsObjectValid(LuaStack& stack, AbsIndex object)
{
    assert(object == 1);

    // 检查参数
    if (stack.TypeOf(object) != LUA_TTABLE)  // 必须是 table
        return false;
    stack.RawGet(object, kIndexOfScriptObjectIdInObject);  // t(object) ... n(id)
    if (stack.TypeOf(-1) != LUA_TNUMBER)  // table[1] 必须是 integer
        return false;
    auto id = static_cast<ScriptObjectId>(lua_tointeger(stack, -1));

    // 获取对象
    auto& world = detail::GetGlobalApp().GetDefaultWorld();
    world.GetScriptObjectPool().PushScriptObject(stack, id);

    // 比较是否是同一个
    if (lua_rawequal(stack, -1, object))
    {
        assert(world.GetEntityByScriptObjectId(id) && world.GetEntityByScriptObjectId(id)->operator bool());
        return true;
    }
    return false;
}

GameObjectModule::Unpack<double, double> GameObjectModule::GetObjectVelocity(LuaStack& stack, AbsIndex object)
{
    assert(object == 1);

    // 检查参数
    if (stack.TypeOf(object) != LUA_TTABLE)
        stack.Error("invalid lstg object for 'GetV'.");
    stack.RawGet(object, kIndexOfScriptObjectIdInObject);  // t(object) ... n(id)
    auto id = static_cast<ScriptObjectId>(luaL_checkinteger(stack, -1));

    // 获取 Entity
    auto& world = detail::GetGlobalApp().GetDefaultWorld();
    auto entity = world.GetEntityByScriptObjectId(id);
    if (!entity)
        stack.Error("invalid lstg object for 'GetV'.");

    // 获取移动组件
    auto movementComponent = entity->TryGetComponent<Components::Movement>();
    if (!movementComponent)
        stack.Error("invalid lstg object for 'GetV'.");

    // 返回速度大小和方向
    auto v = movementComponent->Velocity;
    auto vlen = glm::length(v);
    double angle = 0;
    if (vlen != 0)
        angle = glm::degrees(::atan2(v.y, v.x));
    return { vlen, angle };
}

void GameObjectModule::SetObjectVelocity(LuaStack& stack, AbsIndex object, double velocity, double angle, std::optional<bool> track)
{
    assert(object == 1);

    // 检查参数
    if (stack.TypeOf(object) != LUA_TTABLE)
        stack.Error("invalid lstg object for 'GetV'.");
    stack.RawGet(object, kIndexOfScriptObjectIdInObject);  // t(object) ... n(id)
    auto id = static_cast<ScriptObjectId>(luaL_checkinteger(stack, -1));
    stack.Pop(1);  // t(object) ...

    // 获取 Entity
    auto& world = detail::GetGlobalApp().GetDefaultWorld();
    auto entity = world.GetEntityByScriptObjectId(id);
    if (!entity)
        stack.Error("invalid lstg object for 'SetV'.");

    // 获取移动组件
    auto movementComponent = entity->TryGetComponent<Components::Movement>();
    if (!movementComponent)
        stack.Error("invalid lstg object for 'SetV'.");

    // 获取变换组件
    auto transformComponent = track ? entity->TryGetComponent<Components::Transform>() : nullptr;
    if (track && !transformComponent)
        stack.Error("invalid lstg object for 'SetV'.");

    angle = glm::radians(angle);
    movementComponent->Velocity.x = velocity * ::cos(angle);
    movementComponent->Velocity.y = velocity * ::sin(angle);
    if (track && *track)
    {
        assert(transformComponent);
        transformComponent->Rotation = angle;
    }
}

void GameObjectModule::SetImageStateByObject(LuaStack& stack, AbsIndex object, const char* blend, int32_t a, int32_t r, int32_t g,
    int32_t b)
{
    assert(object == 1);

    // 检查参数
    if (stack.TypeOf(object) != LUA_TTABLE)
        stack.Error("invalid lstg object for 'SetImgState'.");
    stack.RawGet(object, kIndexOfScriptObjectIdInObject);  // t(object) ... n(id)
    auto id = static_cast<ScriptObjectId>(luaL_checkinteger(stack, -1));
    stack.Pop(1);  // t(object) ...

    // 获取 Entity
    auto& world = detail::GetGlobalApp().GetDefaultWorld();
    auto entity = world.GetEntityByScriptObjectId(id);
    if (!entity)
        stack.Error("invalid lstg object for 'SetImgState'.");

    // 获取渲染组件
    auto rendererComponent = entity->TryGetComponent<Components::Renderer>();
    if (!rendererComponent)
        stack.Error("invalid lstg object for 'SetImgState'.");

    // 设置混合模式
    BlendMode mode(blend);
    Subsystem::Render::ColorRGBA32 color(std::clamp(r, 0, 255), std::clamp(g, 0, 255), std::clamp(b, 0, 255), std::clamp(a, 0, 255));
    Subsystem::Render::Drawing2D::SpriteColorComponents colors = { color, color, color, color };
    switch (rendererComponent->RenderData.index())
    {
        case 1:
            {
                auto& spriteRenderer = std::get<1>(rendererComponent->RenderData);
                spriteRenderer.Asset->SetDefaultBlendMode(mode);
                spriteRenderer.Asset->SetDefaultBlendColor(colors);
            }
            break;
        case 2:
            {
                auto& spriteSequenceRenderer = std::get<2>(rendererComponent->RenderData);
                spriteSequenceRenderer.Asset->SetDefaultBlendMode(mode);
                spriteSequenceRenderer.Asset->SetDefaultBlendColor(colors);
            }
            break;
        default:
#ifdef LSTG_DEVELOPMENT
            // 兼容处理，此时不报错
            LSTG_LOG_WARN_CAT(GameObjectModule, "invalid lstg object for 'SetImgState'.");
#endif
            break;
    }
}

double GameObjectModule::CalculateAngle(LuaStack& stack, std::variant<double, AbsIndex> a, std::variant<double, AbsIndex> b,
    std::optional<double> c, std::optional<double> d)
{
    if (a.index() == 1 && b.index() == 1)
    {
        // 检查参数
        if (stack.TypeOf(std::get<AbsIndex>(a)) != LUA_TTABLE)
            stack.Error("invalid lstg object #1 for 'Angle'.");
        stack.RawGet(std::get<AbsIndex>(a), kIndexOfScriptObjectIdInObject);
        auto id1 = static_cast<ScriptObjectId>(luaL_checkinteger(stack, -1));
        stack.Pop(1);

        if (stack.TypeOf(std::get<AbsIndex>(b)) != LUA_TTABLE)
            stack.Error("invalid lstg object #2 for 'Angle'.");
        stack.RawGet(std::get<AbsIndex>(b), kIndexOfScriptObjectIdInObject);
        auto id2 = static_cast<ScriptObjectId>(luaL_checkinteger(stack, -1));
        stack.Pop(1);

        // 获取 Entity
        auto& world = detail::GetGlobalApp().GetDefaultWorld();
        auto entity1 = world.GetEntityByScriptObjectId(id1);
        if (!entity1)
            stack.Error("invalid lstg object #1 for 'Angle'.");
        auto transformComponent1 = entity1->TryGetComponent<Components::Transform>();
        if (!transformComponent1)
            stack.Error("invalid lstg object #1 for 'Angle'.");

        auto entity2 = world.GetEntityByScriptObjectId(id2);
        if (!entity2)
            stack.Error("invalid lstg object #2 for 'Angle'.");
        auto transformComponent2 = entity2->TryGetComponent<Components::Transform>();
        if (!transformComponent2)
            stack.Error("invalid lstg object #2 for 'Angle'.");

        // 计算结果
        auto dt = transformComponent2->Location - transformComponent1->Location;
        return dt.x == 0 && dt.y == 0 ? 0 : glm::degrees(::atan2(dt.y, dt.x));
    }
    else
    {
        if (a.index() != 0 || b.index() != 0 || !c || !d)
            stack.Error("invalid arguments for 'Angle'.");

        auto dt = v2::Vec2 { *c - std::get<0>(a), *d - std::get<0>(b) };
        return dt.x == 0 && dt.y == 0 ? 0 : glm::degrees(::atan2(dt.y, dt.x));
    }
}

double GameObjectModule::CalculateDistance(LuaStack& stack, std::variant<double, AbsIndex> a, std::variant<double, AbsIndex> b,
    std::optional<double> c, std::optional<double> d)
{
    if (a.index() == 1 && b.index() == 1)
    {
        // 检查参数
        if (stack.TypeOf(std::get<AbsIndex>(a)) != LUA_TTABLE)
            stack.Error("invalid lstg object #1 for 'Dist'.");
        stack.RawGet(std::get<AbsIndex>(a), kIndexOfScriptObjectIdInObject);
        auto id1 = static_cast<ScriptObjectId>(luaL_checkinteger(stack, -1));
        stack.Pop(1);

        if (stack.TypeOf(std::get<AbsIndex>(b)) != LUA_TTABLE)
            stack.Error("invalid lstg object #2 for 'Dist'.");
        stack.RawGet(std::get<AbsIndex>(b), kIndexOfScriptObjectIdInObject);
        auto id2 = static_cast<ScriptObjectId>(luaL_checkinteger(stack, -1));
        stack.Pop(1);

        // 获取 Entity
        auto& world = detail::GetGlobalApp().GetDefaultWorld();
        auto entity1 = world.GetEntityByScriptObjectId(id1);
        if (!entity1)
            stack.Error("invalid lstg object #1 for 'Dist'.");
        auto transformComponent1 = entity1->TryGetComponent<Components::Transform>();
        if (!transformComponent1)
            stack.Error("invalid lstg object #1 for 'Dist'.");

        auto entity2 = world.GetEntityByScriptObjectId(id2);
        if (!entity2)
            stack.Error("invalid lstg object #2 for 'Dist'.");
        auto transformComponent2 = entity2->TryGetComponent<Components::Transform>();
        if (!transformComponent2)
            stack.Error("invalid lstg object #2 for 'Dist'.");

        // 计算结果
        auto dt = transformComponent2->Location - transformComponent1->Location;
        return glm::length(dt);
    }
    else
    {
        if (a.index() != 0 || b.index() != 0 || !c || !d)
            stack.Error("invalid arguments for 'Angle'.");

        auto dt = v2::Vec2 { *d - std::get<0>(b), *c - std::get<0>(a) };
        return glm::length(dt);
    }
}

bool GameObjectModule::BoxCheck(LuaStack& stack, AbsIndex object, double left, double right, double bottom, double top)
{
    assert(object == 1);

    // 检查参数
    if (stack.TypeOf(object) != LUA_TTABLE)
        stack.Error("invalid lstg object for 'BoxCheck'.");
    stack.RawGet(object, kIndexOfScriptObjectIdInObject);  // t(object) ... n(id)
    auto id = static_cast<ScriptObjectId>(luaL_checkinteger(stack, -1));
    stack.Pop(1);  // t(object) ...

    // 获取 Entity
    auto& world = detail::GetGlobalApp().GetDefaultWorld();
    auto entity = world.GetEntityByScriptObjectId(id);
    if (!entity)
        stack.Error("invalid lstg object for 'BoxCheck'.");

    // 获取渲染组件
    auto transformComponent = entity->TryGetComponent<Components::Transform>();
    if (!transformComponent)
        stack.Error("invalid lstg object for 'BoxCheck'.");

    auto loc = transformComponent->Location;
    return (loc.x > left && loc.x < right && loc.y > bottom && loc.y < top);
}

void GameObjectModule::ResetPool()
{
    auto& world = detail::GetGlobalApp().GetDefaultWorld();
    world.Clear();
}

void GameObjectModule::DefaultRenderFunc(LuaStack& stack, AbsIndex object)
{
    assert(object == 1);

    // 检查参数
    if (stack.TypeOf(object) != LUA_TTABLE)
        stack.Error("invalid argument #1, luastg object required for 'Kill'.");
    stack.RawGet(object, kIndexOfScriptObjectIdInObject);  // t(object) ... n(id)
    auto id = static_cast<ScriptObjectId>(luaL_checkinteger(stack, -1));
    stack.Pop(1);  // t(object) ...

    // 获取 Entity
    auto& world = detail::GetGlobalApp().GetDefaultWorld();
    auto entity = world.GetEntityByScriptObjectId(id);
    if (!entity)
        stack.Error("invalid lstg object for 'DefaultRenderFunc'.");

    world.RenderEntityDefault(*entity);
}

int GameObjectModule::NextObject(lua_State* L)
{
    auto groupId = static_cast<int32_t>(luaL_checkinteger(L, 1));  // i(groupId)
    auto id = static_cast<ScriptObjectId>(luaL_checknumber(L, 2));  // id
    if (id == std::numeric_limits<ScriptObjectId>::max())
        return 0;

    auto& world = detail::GetGlobalApp().GetDefaultWorld();

    // 先获取实例
    auto entity = world.GetEntityByScriptObjectId(id);
    if (!entity)
        return 0;

    optional<ScriptObjectId> nextObjectId;

    // 检查需要在组中遍历还是在全局对象中遍历
    if (groupId < 0 || groupId >= static_cast<int32_t>(Components::kColliderGroupCount))
    {
        auto& lifeTime = world.GetLifeTimeRoot();
        auto* tailer = &lifeTime.LifeTimeTailer;
        auto nextEntity = entity;

        while (true)
        {
            auto lifeTimeComponent = nextEntity->TryGetComponent<Components::LifeTime>();
            if (!lifeTimeComponent)
            {
                assert(false);
                break;
            }
            if (!lifeTimeComponent->NextNode() || lifeTimeComponent->NextNode() == tailer)
                break;  // 迭代终止

            nextEntity = lifeTimeComponent->NextNode()->BindingEntity;
            assert(nextEntity);

            // 获取下一个ID
            auto scriptComponent = nextEntity->TryGetComponent<Components::Script>();
            if (!scriptComponent)
                continue;
            assert(scriptComponent->Pool == &world.GetScriptObjectPool());

            nextObjectId = static_cast<int32_t>(scriptComponent->ScriptObjectId);
            break;
        }
    }
    else
    {
        auto& collider = world.GetColliderRoot();
        auto& tailer = collider.ColliderGroupTailers[groupId];
        auto nextEntity = entity;

        while (true)
        {
            auto colliderComponent = nextEntity->TryGetComponent<Components::Collider>();
            if (!colliderComponent)
            {
                assert(false);
                break;
            }
            if (colliderComponent->Group != static_cast<uint32_t>(groupId))
            {
#ifdef LSTG_DEVELOPMENT
                LSTG_LOG_WARN_CAT(GameObjectModule, "Collider group changed while iterating");
#endif
                break;
            }
            if (!colliderComponent->NextNode() || colliderComponent->NextNode() == &tailer)
                break;  // 迭代终止

            nextEntity = colliderComponent->NextNode()->BindingEntity;
            assert(nextEntity);

            // 获取下一个ID
            auto scriptComponent = nextEntity->TryGetComponent<Components::Script>();
            if (!scriptComponent)
                continue;
            assert(scriptComponent->Pool == &world.GetScriptObjectPool());

            nextObjectId = static_cast<int32_t>(scriptComponent->ScriptObjectId);
            break;
        }
    }

    // 推入下一个ID和当前对象
    lua_pushnumber(L, nextObjectId ? *nextObjectId : std::numeric_limits<ScriptObjectId>::max());
    world.GetScriptObjectPool().PushScriptObject(L, id);
    return 2;
}

GameObjectModule::Unpack<GameObjectModule::AbsIndex, int32_t, double> GameObjectModule::EnumerateObjectList(LuaStack& stack,
    int32_t groupId)
{
    auto& world = detail::GetGlobalApp().GetDefaultWorld();
    ScriptObjectId firstObjectId = std::numeric_limits<ScriptObjectId>::max();

    // 检查需要在组中遍历还是在全局对象中遍历
    if (groupId < 0 || groupId >= static_cast<int32_t>(Components::kColliderGroupCount))
    {
        auto& lifeTime = world.GetLifeTimeRoot();
        auto* current = lifeTime.LifeTimeHeader.NextNode();
        auto* tailer = &lifeTime.LifeTimeTailer;

        while (current && current != tailer)
        {
            auto& entity = current->BindingEntity;
            current = current->NextNode();

            auto scriptComponent = entity.TryGetComponent<Components::Script>();
            if (scriptComponent)
            {
                firstObjectId = scriptComponent->ScriptObjectId;
                break;
            }
        }
    }
    else
    {
        auto& collider = world.GetColliderRoot();
        auto* current = collider.ColliderGroupHeaders[groupId].NextNode();
        auto* tailer = &collider.ColliderGroupTailers[groupId];

        while (current && current != tailer)
        {
            auto& entity = current->BindingEntity;
            current = current->NextNode();

            auto scriptComponent = entity.TryGetComponent<Components::Script>();
            if (scriptComponent)
            {
                firstObjectId = scriptComponent->ScriptObjectId;
                break;
            }
        }
    }

    stack.PushValue(NextObject);
    return { AbsIndex {stack.GetTop()}, groupId, firstObjectId };  // 返回组ID，第一个对象ID
}

int GameObjectModule::GetObjectAttribute(lua_State* L)
{
    auto& world = detail::GetGlobalApp().GetDefaultWorld();
    auto bridge = world.GetScriptObjectPool().GetScriptObjectBridge();
    assert(bridge);

    // 取出对象 ID
    luaL_checktype(L, 1, LUA_TTABLE);  // t k ...
    auto key = luaL_checkstring(L, 2);
    lua_rawgeti(L, 1, kIndexOfScriptObjectIdInObject);  // t k ... i(ID)
    auto id = static_cast<ScriptObjectId>(lua_tointeger(L, -1));
    lua_settop(L, 2);  // t k

    // 获取 Entity
    auto entity = world.GetEntityByScriptObjectId(id);
    if (!entity)
        luaL_error(L, "entity is already disposed, sid=%d", static_cast<int>(id));

    return bridge->OnGetAttribute(L, entity->GetId(), key);
}

int GameObjectModule::SetObjectAttribute(lua_State* L)
{
    auto& world = detail::GetGlobalApp().GetDefaultWorld();
    auto bridge = world.GetScriptObjectPool().GetScriptObjectBridge();
    assert(bridge);

    // 取出对象 ID
    luaL_checktype(L, 1, LUA_TTABLE);  // t k v ...
    auto key = luaL_checkstring(L, 2);
    lua_rawgeti(L, 1, kIndexOfScriptObjectIdInObject);  // t k v ... i(ID)
    auto id = static_cast<ScriptObjectId>(lua_tointeger(L, -1));
    lua_settop(L, 3);  // t k v

    // 获取 Entity
    auto entity = world.GetEntityByScriptObjectId(id);
    if (!entity)
        luaL_error(L, "entity is already disposed, sid=%d", static_cast<int>(id));

    if (!bridge->OnSetAttribute(L, entity->GetId(), key, AbsIndex { 3u }))
        lua_rawset(L, 1);
    return 0;
}

void GameObjectModule::ParticleFire(LuaStack& stack, AbsIndex object)
{
    assert(object == 1);

    // 检查参数
    if (stack.TypeOf(object) != LUA_TTABLE)
        stack.Error("invalid lstg object for 'ParticleFire'.");
    stack.RawGet(object, kIndexOfScriptObjectIdInObject);  // t(object) ... n(id)
    auto id = static_cast<ScriptObjectId>(luaL_checkinteger(stack, -1));
    stack.Pop(1);  // t(object) ...

    // 获取 Entity
    auto& world = detail::GetGlobalApp().GetDefaultWorld();
    auto entity = world.GetEntityByScriptObjectId(id);
    if (!entity)
        stack.Error("invalid lstg object for 'ParticleFire'.");

    // 获取粒子组件
    auto rendererComponent = entity->TryGetComponent<Components::Renderer>();
    if (!rendererComponent || rendererComponent->RenderData.index() != 3)
    {
        LSTG_LOG_WARN_CAT(GameObjectModule, "entity doesn't contain particle system");
        return;
    }

    auto& particleRenderer = std::get<Components::Renderer::ParticleRenderer>(rendererComponent->RenderData);
    assert(particleRenderer.Emitter);
    particleRenderer.Emitter->SetAlive();
}

void GameObjectModule::ParticleStop(LuaStack& stack, AbsIndex object)
{
    assert(object == 1);

    // 检查参数
    if (stack.TypeOf(object) != LUA_TTABLE)
        stack.Error("invalid lstg object for 'ParticleFire'.");
    stack.RawGet(object, kIndexOfScriptObjectIdInObject);  // t(object) ... n(id)
    auto id = static_cast<ScriptObjectId>(luaL_checkinteger(stack, -1));
    stack.Pop(1);  // t(object) ...

    // 获取 Entity
    auto& world = detail::GetGlobalApp().GetDefaultWorld();
    auto entity = world.GetEntityByScriptObjectId(id);
    if (!entity)
        stack.Error("invalid lstg object for 'ParticleFire'.");

    // 获取粒子组件
    auto rendererComponent = entity->TryGetComponent<Components::Renderer>();
    if (!rendererComponent || rendererComponent->RenderData.index() != 3)
    {
        LSTG_LOG_WARN_CAT(GameObjectModule, "entity doesn't contain particle system");
        return;
    }

    auto& particleRenderer = std::get<Components::Renderer::ParticleRenderer>(rendererComponent->RenderData);
    assert(particleRenderer.Emitter);
    particleRenderer.Emitter->SetDead();
}

int32_t GameObjectModule::ParticleGetCount(LuaStack& stack, AbsIndex object)
{
    assert(object == 1);

    // 检查参数
    if (stack.TypeOf(object) != LUA_TTABLE)
        stack.Error("invalid lstg object for 'ParticleFire'.");
    stack.RawGet(object, kIndexOfScriptObjectIdInObject);  // t(object) ... n(id)
    auto id = static_cast<ScriptObjectId>(luaL_checkinteger(stack, -1));
    stack.Pop(1);  // t(object) ...

    // 获取 Entity
    auto& world = detail::GetGlobalApp().GetDefaultWorld();
    auto entity = world.GetEntityByScriptObjectId(id);
    if (!entity)
        stack.Error("invalid lstg object for 'ParticleFire'.");

    // 获取粒子组件
    auto rendererComponent = entity->TryGetComponent<Components::Renderer>();
    if (!rendererComponent || rendererComponent->RenderData.index() != 3)
    {
        LSTG_LOG_WARN_CAT(GameObjectModule, "entity doesn't contain particle system");
        return 0;
    }

    auto& particleRenderer = std::get<Components::Renderer::ParticleRenderer>(rendererComponent->RenderData);
    assert(particleRenderer.Emitter);
    return static_cast<int32_t>(particleRenderer.Emitter->GetParticleCount());
}

float GameObjectModule::ParticleGetEmission(LuaStack& stack, AbsIndex object)
{
    assert(object == 1);

    // 检查参数
    if (stack.TypeOf(object) != LUA_TTABLE)
        stack.Error("invalid lstg object for 'ParticleFire'.");
    stack.RawGet(object, kIndexOfScriptObjectIdInObject);  // t(object) ... n(id)
    auto id = static_cast<ScriptObjectId>(luaL_checkinteger(stack, -1));
    stack.Pop(1);  // t(object) ...

    // 获取 Entity
    auto& world = detail::GetGlobalApp().GetDefaultWorld();
    auto entity = world.GetEntityByScriptObjectId(id);
    if (!entity)
        stack.Error("invalid lstg object for 'ParticleFire'.");

    // 获取粒子组件
    auto rendererComponent = entity->TryGetComponent<Components::Renderer>();
    if (!rendererComponent || rendererComponent->RenderData.index() != 3)
    {
        LSTG_LOG_WARN_CAT(GameObjectModule, "entity doesn't contain particle system");
        return 0;
    }

    auto& particleRenderer = std::get<Components::Renderer::ParticleRenderer>(rendererComponent->RenderData);
    auto emitter = particleRenderer.Emitter;
    assert(emitter);
    auto override = emitter->GetEmissionOverride();
    if (!override)
        return static_cast<float>(emitter->GetConfig()->EmissionPerSecond);
    return *override;
}

void GameObjectModule::ParticleSetEmission(LuaStack& stack, AbsIndex object, float count)
{
    assert(object == 1);

    // 检查参数
    if (stack.TypeOf(object) != LUA_TTABLE)
        stack.Error("invalid lstg object for 'ParticleFire'.");
    stack.RawGet(object, kIndexOfScriptObjectIdInObject);  // t(object) ... n(id)
    auto id = static_cast<ScriptObjectId>(luaL_checkinteger(stack, -1));
    stack.Pop(1);  // t(object) ...

    // 获取 Entity
    auto& world = detail::GetGlobalApp().GetDefaultWorld();
    auto entity = world.GetEntityByScriptObjectId(id);
    if (!entity)
        stack.Error("invalid lstg object for 'ParticleFire'.");

    // 获取粒子组件
    auto rendererComponent = entity->TryGetComponent<Components::Renderer>();
    if (!rendererComponent || rendererComponent->RenderData.index() != 3)
    {
        LSTG_LOG_WARN_CAT(GameObjectModule, "entity doesn't contain particle system");
        return;
    }

    auto& particleRenderer = std::get<Components::Renderer::ParticleRenderer>(rendererComponent->RenderData);
    assert(particleRenderer.Emitter);
    particleRenderer.Emitter->SetEmissionOverride(count);
}
