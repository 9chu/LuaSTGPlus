/**
 * @file
 * @date 2022/7/24
 * @author 9chu
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#include <lstg/v2/GamePlay/GameWorld.hpp>

#include <lstg/Core/Logging.hpp>
#include <lstg/Core/Subsystem/ScriptSystem.hpp>
#include <lstg/Core/Subsystem/Script/LuaPush.hpp>
#include <lstg/Core/Subsystem/Script/LuaRead.hpp>
#include <lstg/Core/Subsystem/Render/Drawing2D/SpriteDrawing.hpp>
#include <lstg/v2/GameApp.hpp>
#include <lstg/v2/GamePlay/Components/Collider.hpp>
#include <lstg/v2/GamePlay/Components/LifeTime.hpp>
#include <lstg/v2/GamePlay/Components/Movement.hpp>
#include <lstg/v2/GamePlay/Components/Renderer.hpp>
#include <lstg/v2/GamePlay/Components/Script.hpp>
#include <lstg/v2/GamePlay/Components/Transform.hpp>
#include <lstg/v2/Asset/SpriteAsset.hpp>
#include <lstg/v2/Asset/SpriteSequenceAsset.hpp>
#include <lstg/v2/Asset/HgeParticleAsset.hpp>
#include <ScriptObjectAttributes.gen.hpp>

using namespace std;
using namespace lstg;
using namespace lstg::v2::GamePlay;

using namespace lstg::Subsystem::Script;
using namespace lstg::v2::GamePlay::Components;

LSTG_DEF_LOG_CATEGORY(GameWorld);

namespace
{
    /**
     * 在指定节点前插入节点
     * @tparam T Component类型
     * @param dest 目的节点
     * @param self 要插入的节点
     */
    template <typename T>
    void ListInsertBefore(T* dest, T* self) noexcept
    {
        assert(dest);
        assert(self && !self->PrevInChain && !self->NextInChain);
        self->PrevInChain = dest->PrevInChain;
        self->NextInChain = dest;

        if (dest->PrevInChain)
            dest->PrevInChain->NextInChain = self;
        dest->PrevInChain = self;
    }

    /**
     * 在指定节点后插入节点
     * @tparam T Component类型
     * @param dest 目的节点
     * @param self 要插入的节点
     */
    template <typename T>
    void ListInsertAfter(T* dest, T* self) noexcept
    {
        assert(dest);
        assert(self && !self->PrevInChain && !self->NextInChain);
        self->PrevInChain = dest;
        self->NextInChain = dest->NextInChain;

        if (dest->NextInChain)
            dest->NextInChain->PrevInChain = self;
        dest->NextInChain = self;
    }

    /**
     * 将节点从链表移除
     * @tparam T Component类型
     * @param self 节点
     */
    template <typename T>
    void ListRemove(T* self) noexcept
    {
        if (self->PrevInChain)
            self->PrevInChain->NextInChain = self->NextInChain;
        if (self->NextInChain)
            self->NextInChain->PrevInChain = self->PrevInChain;
        self->NextInChain = nullptr;
        self->PrevInChain = nullptr;
    }

    /**
     * 在有序链表上对节点进行插入排序
     * @tparam T Component类型
     * @tparam TComparer
     * @param self
     * @param comparer
     */
    template <typename T, typename TComparer>
    void ListInsertSort(T* self, TComparer comparer) noexcept
    {
        // 插入排序
        // NOTE: 这里会保证头尾节点不参与排序过程
        assert(self->NextInChain && self->PrevInChain);
        if (self->NextInChain->NextInChain && comparer(self->NextInChain, self))
        {
            // 向后插入
            auto insertBefore = self->NextInChain->NextInChain;
            while (insertBefore->NextInChain && comparer(insertBefore, self))
                insertBefore = insertBefore->NextInChain;
            ListRemove(self);
            ListInsertBefore(insertBefore, self);
        }
        else if (self->PrevInChain->PrevInChain && comparer(self, self->PrevInChain))
        {
            // 向前插入
            auto insertAfter = self->PrevInChain->PrevInChain;
            while (insertAfter->PrevInChain && comparer(self, insertAfter))
                insertAfter = insertAfter->PrevInChain;
            ListRemove(self);
            ListInsertAfter(insertAfter, self);
        }
    }

    bool ColliderSortFunction(Collider* lhs, Collider* rhs) noexcept
    {
        // 总是比较对象ID
        auto lhsScript = lhs->BindingEntity.TryGetComponent<Script>();
        auto rhsScript = rhs->BindingEntity.TryGetComponent<Script>();
        return (lhsScript ? lhsScript->ScriptObjectId : 0) < (rhsScript ? rhsScript->ScriptObjectId : 0);
    }

    bool RendererSortFunction(Renderer* lhs, Renderer* rhs) noexcept
    {
        // Layer 小的靠前
        if (lhs->Layer < rhs->Layer)
            return true;

        // 相同时比较对象ID
        if (lhs->Layer == rhs->Layer)
        {
            auto lhsScript = lhs->BindingEntity.TryGetComponent<Script>();
            auto rhsScript = rhs->BindingEntity.TryGetComponent<Script>();
            return (lhsScript ? lhsScript->ScriptObjectId : 0) < (rhsScript ? rhsScript->ScriptObjectId : 0);
        }
        return false;
    }
}

GameWorld::GameWorld(GameApp& app)
    : m_stApp(app), m_stScriptObjectPool(app.GetSubsystem<Subsystem::ScriptSystem>()->GetState(), this)
{
    // 创建根 Entity
    m_stRootEntity = m_stWorld.CreateEntity<LifeTimeRoot, ColliderRoot, RendererRoot>().ThrowIfError();

    // 保存 Component 的引用，当且仅当 XXXRoot 所在 Chunk 不发生 Expand
    m_pLifeTimeRoot = &m_stRootEntity.GetComponent<LifeTimeRoot>();
    m_pColliderRoot = &m_stRootEntity.GetComponent<ColliderRoot>();
    m_pRendererRoot = &m_stRootEntity.GetComponent<RendererRoot>();
}

Result<LuaStack::AbsIndex> GameWorld::CreateEntity(LuaStack stack, LuaStack::AbsIndex classIndex) noexcept
{
    // 创建实例
    auto entity = m_stWorld.CreateEntity<Transform, Collider, Movement, Renderer, LifeTime, Script>();
    if (!entity)
    {
        LSTG_LOG_ERROR_CAT(GameWorld, "CreateEntity fail, ret={}", entity.GetError());
        return entity.GetError();
    }

    // 创建脚本对象
    auto luaStackTop = stack.GetTop();
    assert(classIndex.Index <= luaStackTop);
    unsigned initCallArgs = luaStackTop - classIndex.Index;
    auto scriptObject = m_stScriptObjectPool.Alloc(stack, classIndex, entity->GetId());
    if (!scriptObject)
    {
        entity->Destroy();
        return scriptObject.GetError();
    }
    assert(luaStackTop + 1 == stack.GetTop());
    assert(std::get<1>(*scriptObject) == stack.GetTop());  // 产生的元素总是在栈顶

    // 初始化组件
    auto& transform = entity->GetComponent<Transform>();
    auto& collider = entity->GetComponent<Collider>();
    auto& renderer = entity->GetComponent<Renderer>();
    auto& lifeTime = entity->GetComponent<LifeTime>();
    auto& script = entity->GetComponent<Script>();
    collider.BindingEntity = *entity;
    ListInsertBefore(&(m_pColliderRoot->ColliderGroupTailers[collider.Group]), &collider);
    ListInsertSort(&collider, ColliderSortFunction);
    renderer.BindingEntity = *entity;
    ListInsertBefore(&m_pRendererRoot->RendererTailer, &renderer);
    ListInsertSort(&renderer, RendererSortFunction);
    lifeTime.BindingEntity = *entity;
    ListInsertBefore(&m_pLifeTimeRoot->LifeTimeTailer, &lifeTime);
    script.Pool = &m_stScriptObjectPool;
    script.ScriptObjectId = std::get<0>(*scriptObject);

    // 调用 Init 事件
    stack.Insert(classIndex.Index + 1);  // ... t(class) t(object) {args...}
    m_stScriptObjectPool.InvokeCallback(stack, script.ScriptObjectId, ScriptCallbackFunctions::OnInit, initCallArgs);
    assert(luaStackTop - initCallArgs + 1 == stack.GetTop());

    // Init 执行后，更新上一帧位置 X, Y
    transform.LastLocation = transform.Location;

    // 返回 Lua 对象
    return LuaStack::AbsIndex { stack.GetTop() };
}

std::optional<ECS::Entity> GameWorld::GetEntityByScriptObjectId(ScriptObjectId id) noexcept
{
    auto entId = m_stScriptObjectPool.GetEntityId(id);
    if (!entId)
        return nullopt;
    auto ret = ECS::Entity { &m_stWorld, *entId };
    assert(ret);  // 对象在 ECS 侧有效
    return ret;
}

bool GameWorld::DeleteEntity(Subsystem::Script::LuaStack stack, ScriptObjectId scriptId, unsigned args) noexcept
{
    // 获取 ECS 实例
    auto entity = GetEntityByScriptObjectId(scriptId);
    if (!entity)
    {
        LSTG_LOG_ERROR_CAT(GameWorld, "Attempt to delete entity already disposed, id={}", scriptId);
        return false;
    }

    // 是否已经无效
    assert(entity->HasComponent<LifeTime>());
    auto& lifeTime = entity->GetComponent<LifeTime>();
    if (lifeTime.Status == LifeTimeStatus::Alive)
    {
        lifeTime.Status = LifeTimeStatus::Deleted;

        // 调用脚本方法
        m_stScriptObjectPool.InvokeCallback(stack, scriptId, ScriptCallbackFunctions::OnDelete, args);
    }
    else
    {
        LSTG_LOG_WARN_CAT(GameWorld, "Unexpected entity state {}, id={}", ToString(lifeTime.Status), scriptId);
    }
    return true;
}

bool GameWorld::KillEntity(Subsystem::Script::LuaStack stack, ScriptObjectId scriptId, unsigned args) noexcept
{
    // 获取 ECS 实例
    auto entity = GetEntityByScriptObjectId(scriptId);
    if (!entity)
    {
        LSTG_LOG_ERROR_CAT(GameWorld, "Attempt to kill entity already disposed, id={}", scriptId);
        return false;
    }

    // 是否已经无效
    assert(entity->HasComponent<LifeTime>());
    auto& lifeTime = entity->GetComponent<LifeTime>();
    if (lifeTime.Status == LifeTimeStatus::Alive)
    {
        lifeTime.Status = LifeTimeStatus::Killed;

        // 调用脚本方法
        m_stScriptObjectPool.InvokeCallback(stack, scriptId, ScriptCallbackFunctions::OnKill, args);
    }
    else
    {
        LSTG_LOG_WARN_CAT(GameWorld, "Unexpected entity state {}, id={}", ToString(lifeTime.Status), scriptId);
    }
    return true;
}

void GameWorld::Frame() noexcept
{
    // 为了保证与 luastg 行为的兼容性，这里通过 LifeTime 上的链表更新所有对象
    // 这并不符合 ECS 的使用规范，无法得到 cache friendly 的优势
    assert(m_pLifeTimeRoot);
    LifeTime* p = m_pLifeTimeRoot->LifeTimeHeader.NextInChain;
    assert(p);
    while (p != &m_pLifeTimeRoot->LifeTimeTailer)
    {
        auto entity = p->BindingEntity;

        // 调用 Frame 方法
        auto scriptComponent = entity.TryGetComponent<Script>();
        if (scriptComponent)
        {
            assert(scriptComponent->Pool == &m_stScriptObjectPool);
            m_stScriptObjectPool.InvokeCallback(m_stScriptObjectPool.GetState(), scriptComponent->ScriptObjectId,
                ScriptCallbackFunctions::OnFrame, 0);
        }

        // 更新对象运动状态
        auto transformComponent = entity.TryGetComponent<Transform>();
        auto movementComponent = entity.TryGetComponent<Movement>();
        if (transformComponent && movementComponent)
        {
            movementComponent->Velocity += movementComponent->AccelVelocity;
            transformComponent->Location += movementComponent->Velocity;
            transformComponent->Rotation += movementComponent->AngularVelocity;
        }

        // 更新粒子系统（若有）
        auto rendererComponent = entity.TryGetComponent<Renderer>();
        if (transformComponent && rendererComponent && rendererComponent->RenderData.index() == 2)
        {
            auto& particleData = std::get<Renderer::ParticleRenderer>(rendererComponent->RenderData);

            // 刷新默认发射器状态
            if (particleData.Emitter)
            {
                assert(particleData.Pool);
                auto emitter = particleData.Emitter;
                if (emitter->IsAlive())
                    emitter->SetAlive();  // 只要粒子保持存活状态，就一直激活粒子
                emitter->SetPosition(transformComponent->Location);
                emitter->SetRotation(static_cast<float>(transformComponent->Rotation));
                emitter->SetScale(rendererComponent->Scale);
            }

            // 更新所有发射器
            if (particleData.Pool)
                particleData.Pool->Update(1.0f / 1.60f);  // 总是使用 60fps 的速度 Tick
        }

        p = p->NextInChain;
    }
}

void GameWorld::Render() noexcept
{
    assert(m_pRendererRoot);
    Renderer* p = m_pRendererRoot->RendererHeader.NextInChain;
    assert(p);
    while (p != &m_pRendererRoot->RendererTailer)
    {
        auto entity = p->BindingEntity;

        if (!p->Invisible)
        {
            // 调用 Render 方法
            auto scriptComponent = entity.TryGetComponent<Script>();
            if (scriptComponent)
            {
                assert(scriptComponent->Pool == &m_stScriptObjectPool);
                if (m_stScriptObjectPool.InvokeCallback(m_stScriptObjectPool.GetState(), scriptComponent->ScriptObjectId,
                    ScriptCallbackFunctions::OnRender, 0) == ScriptCallbackInvokeResult::CallbackNotDefined)
                {
                    // 当没有用户定义渲染方法时，调用默认渲染方法
                    RenderEntityDefault(entity);
                }
            }
        }

        p = p->NextInChain;
    }
}

void GameWorld::UpdateCoordinate() noexcept
{
    m_stWorld.VisitEntities<tuple<Transform, Movement>>(
        [](ECS::Entity ent, Transform& transform, Movement& movement) {
            transform.LocationDelta = transform.Location - transform.LastLocation;
            transform.LastLocation = transform.Location;
            if (movement.RotateToSpeedDirection && (transform.LocationDelta.x != 0 && transform.LocationDelta.y != 0))
                transform.Rotation = ::atan2(transform.LocationDelta.y, transform.LocationDelta.x);
        });
}

void GameWorld::AfterFrame() noexcept
{
    // 更新生命周期
    m_stWorld.VisitEntities<tuple<LifeTime>>([](ECS::Entity ent, LifeTime& lifeTime) {
        ++lifeTime.Timer;
        if (lifeTime.Status != LifeTimeStatus::Alive)
            ent.Destroy();
    });

    // 更新动画计时器
    m_stWorld.VisitEntities<tuple<Renderer>>([](ECS::Entity ent, Renderer& renderer) {
        if (renderer.RenderData.index() == 2)
        {
            auto& spriteSequenceData = std::get<2>(renderer.RenderData);
            ++spriteSequenceData.Timer;
        }
    });
}

void GameWorld::DeleteOutOfBoundaryEntities() noexcept
{
    auto topLeft = m_stBoundary.GetTopLeft();
    auto bottomRight = m_stBoundary.GetBottomRight();

    assert(m_pLifeTimeRoot);
    LifeTime* p = m_pLifeTimeRoot->LifeTimeHeader.NextInChain;
    assert(p);
    while (p != &m_pLifeTimeRoot->LifeTimeTailer)
    {
        auto entity = p->BindingEntity;

        auto transformComponent = entity.TryGetComponent<Transform>();
        if (transformComponent)
        {
            auto lifeTimeComponent = entity.TryGetComponent<LifeTime>();
            if (lifeTimeComponent && lifeTimeComponent->OutOfBoundaryAutoRemove)
            {
                // 检查是否越界
                auto loc = transformComponent->Location;
                if (loc.x < topLeft.x || loc.x > bottomRight.x || loc.y > topLeft.y || loc.y < bottomRight.y)
                {
                    // 越界删除
                    lifeTimeComponent->Status = LifeTimeStatus::Deleted;

                    auto scriptComponent = entity.TryGetComponent<Script>();
                    if (scriptComponent)
                    {
                        assert(scriptComponent->Pool == &m_stScriptObjectPool);
                        m_stScriptObjectPool.InvokeCallback(m_stScriptObjectPool.GetState(), scriptComponent->ScriptObjectId,
                            ScriptCallbackFunctions::OnDelete, 0);
                    }
                }
            }
        }

        p = p->NextInChain;
    }
}

void GameWorld::RenderEntityDefault(ECS::Entity entity) noexcept
{
    auto transformComponent = entity.TryGetComponent<Transform>();
    if (transformComponent)
    {
        auto rendererComponent = entity.TryGetComponent<Renderer>();
        if (rendererComponent)
        {
            auto& cmdBuffer = m_stApp.GetCommandBuffer();

            switch (rendererComponent->RenderData.index())
            {
                case 0:
                    break;
                case 1:
                    {
                        auto& spriteRenderer = std::get<1>(rendererComponent->RenderData);
                        assert(spriteRenderer.Asset);
                        auto draw = spriteRenderer.Asset->GetDrawingSprite().Draw(cmdBuffer);
                        if (!draw)
                        {
                            LSTG_LOG_ERROR_CAT(GameWorld, "Draw asset {} fail: {}", spriteRenderer.Asset->GetName(), draw.GetError());
                            return;
                        }

                        auto loc = transformComponent->Location;
                        draw->Transform(static_cast<float>(transformComponent->Rotation), static_cast<float>(rendererComponent->Scale.x),
                            static_cast<float>(rendererComponent->Scale.y));
                        draw->Translate(static_cast<float>(loc.x), static_cast<float>(loc.y), 0.5f);
                    }
                    break;
                case 2:
                    {
                        auto& spriteSequenceRenderer = std::get<2>(rendererComponent->RenderData);
                        auto& asset = spriteSequenceRenderer.Asset;
                        assert(asset && !asset->GetSequences().empty());
                        auto frame = (spriteSequenceRenderer.Timer / spriteSequenceRenderer.Asset->GetInterval()) %
                            asset->GetSequences().size();
                        auto draw = spriteSequenceRenderer.Asset->GetSequences()[frame].Draw(cmdBuffer);
                        if (!draw)
                        {
                            LSTG_LOG_ERROR_CAT(GameWorld, "Draw asset {} fail: {}", asset->GetName(), draw.GetError());
                            return;
                        }

                        auto loc = transformComponent->Location;
                        draw->Transform(static_cast<float>(transformComponent->Rotation), static_cast<float>(rendererComponent->Scale.x),
                            static_cast<float>(rendererComponent->Scale.y));
                        draw->Translate(static_cast<float>(loc.x), static_cast<float>(loc.y), 0.5f);
                    }
                    break;
                case 3:
                    {
                        auto& particleRenderer = std::get<3>(rendererComponent->RenderData);
                        assert(particleRenderer.Emitter);
                        auto ret = particleRenderer.Emitter->Draw(cmdBuffer);
                        if (!ret)
                            LSTG_LOG_ERROR_CAT(GameWorld, "Draw asset {} fail: {}", particleRenderer.Asset->GetName(), ret.GetError());
                    }
                    break;
                default:
                    assert(false);
                    break;
            }
        }
    }
}

void GameWorld::Clear() noexcept
{
    assert(m_pLifeTimeRoot);
    LifeTime* p = m_pLifeTimeRoot->LifeTimeHeader.NextInChain;
    assert(p);
    while (p != &m_pLifeTimeRoot->LifeTimeTailer)
    {
        auto next = p->NextInChain;
        p->BindingEntity.Destroy();
        p = next;
    }
    assert(m_stScriptObjectPool.GetCurrentObjects() == 0);
}

int GameWorld::OnGetAttribute(LuaStack stack, ECS::EntityId id, std::string_view key)
{
    auto attr = TranslateScriptObjectAttributes(key);
    if (!attr)
        return 0;

    ECS::Entity ent {&m_stWorld, id};
    Transform* transformComponent = nullptr;
    Movement* movementComponent = nullptr;
    LifeTime* lifeTimeComponent = nullptr;
    Renderer* rendererComponent = nullptr;
    Collider* colliderComponent = nullptr;
    Script* scriptComponent = nullptr;

    switch (*attr)
    {
        case ScriptObjectAttributes::X:
            if (!(transformComponent = ent.TryGetComponent<Transform>()))
                return 0;
            stack.PushValue(transformComponent->Location.x);
            return 1;
        case ScriptObjectAttributes::Y:
            if (!(transformComponent = ent.TryGetComponent<Transform>()))
                return 0;
            stack.PushValue(transformComponent->Location.y);
            return 1;
        case ScriptObjectAttributes::DeltaX:
            if (!(transformComponent = ent.TryGetComponent<Transform>()))
                return 0;
            stack.PushValue(transformComponent->LocationDelta.x);
            return 1;
        case ScriptObjectAttributes::DeltaY:
            if (!(transformComponent = ent.TryGetComponent<Transform>()))
                return 0;
            stack.PushValue(transformComponent->LocationDelta.y);
            return 1;
        case ScriptObjectAttributes::Rotation:
            if (!(transformComponent = ent.TryGetComponent<Transform>()))
                return 0;
            stack.PushValue(glm::degrees(transformComponent->Rotation));
            return 1;
        case ScriptObjectAttributes::AngularVelocity:
            if (!(movementComponent = ent.TryGetComponent<Movement>()))
                return 0;
            stack.PushValue(glm::degrees(movementComponent->AngularVelocity));
            return 1;
        case ScriptObjectAttributes::Timer:
            if (!(lifeTimeComponent = ent.TryGetComponent<LifeTime>()))
                return 0;
            stack.PushValue(lifeTimeComponent->Timer);
            return 1;
        case ScriptObjectAttributes::VelocityX:
            if (!(movementComponent = ent.TryGetComponent<Movement>()))
                return 0;
            stack.PushValue(movementComponent->Velocity.x);
            return 1;
        case ScriptObjectAttributes::VelocityY:
            if (!(movementComponent = ent.TryGetComponent<Movement>()))
                return 0;
            stack.PushValue(movementComponent->Velocity.y);
            return 1;
        case ScriptObjectAttributes::AccelVelocityX:
            if (!(movementComponent = ent.TryGetComponent<Movement>()))
                return 0;
            stack.PushValue(movementComponent->AccelVelocity.x);
            return 1;
        case ScriptObjectAttributes::AccelVelocityY:
            if (!(movementComponent = ent.TryGetComponent<Movement>()))
                return 0;
            stack.PushValue(movementComponent->AccelVelocity.y);
            return 1;
        case ScriptObjectAttributes::Layer:
            if (!(rendererComponent = ent.TryGetComponent<Renderer>()))
                return 0;
            stack.PushValue(rendererComponent->Layer);
            return 1;
        case ScriptObjectAttributes::Group:
            if (!(colliderComponent = ent.TryGetComponent<Collider>()))
                return 0;
            stack.PushValue(colliderComponent->Group);
            return 1;
        case ScriptObjectAttributes::Invisible:
            if (!(rendererComponent = ent.TryGetComponent<Renderer>()))
                return 0;
            stack.PushValue(rendererComponent->Invisible);
            return 1;
        case ScriptObjectAttributes::BoundaryCheck:
            if (!(lifeTimeComponent = ent.TryGetComponent<LifeTime>()))
                return 0;
            stack.PushValue(lifeTimeComponent->OutOfBoundaryAutoRemove);
            return 1;
        case ScriptObjectAttributes::TrackDirection:
            if (!(movementComponent = ent.TryGetComponent<Movement>()))
                return 0;
            stack.PushValue(movementComponent->RotateToSpeedDirection);
            return 1;
        case ScriptObjectAttributes::CollisionCheck:
            if (!(colliderComponent = ent.TryGetComponent<Collider>()))
                return 0;
            stack.PushValue(colliderComponent->Enabled);
            return 1;
        case ScriptObjectAttributes::Status:
            if (!(lifeTimeComponent = ent.TryGetComponent<LifeTime>()))
                return 0;
            switch (lifeTimeComponent->Status)
            {
                case LifeTimeStatus::Alive:
                    stack.PushValue("normal");
                    return 1;
                case LifeTimeStatus::Deleted:
                    stack.PushValue("del");
                    return 1;
                case LifeTimeStatus::Killed:
                    stack.PushValue("kill");
                    return 1;
                default:
                    assert(false);
                    return 0;
            }
        case ScriptObjectAttributes::ScaleX:
            if (!(rendererComponent = ent.TryGetComponent<Renderer>()))
                return 0;
            stack.PushValue(rendererComponent->Scale.x);
            return 1;
        case ScriptObjectAttributes::ScaleY:
            if (!(rendererComponent = ent.TryGetComponent<Renderer>()))
                return 0;
            stack.PushValue(rendererComponent->Scale.y);
            return 1;
        case ScriptObjectAttributes::Class:
            if (!(scriptComponent = ent.TryGetComponent<Script>()))
                return 0;
            assert(scriptComponent->Pool == &m_stScriptObjectPool);
            m_stScriptObjectPool.PushScriptObject(stack, scriptComponent->ScriptObjectId);
            if (stack.TypeOf(-1) == LUA_TNIL)
            {
                assert(false);
                return 1;
            }
            stack.RawGet(-1, kIndexOfClassInObject);
            stack.Remove(-2);
            return 1;
        case ScriptObjectAttributes::ColliderX:
            if (!(colliderComponent = ent.TryGetComponent<Collider>()))
                return 0;
            switch (colliderComponent->Shape.index())
            {
                case 0:
                    stack.PushValue(std::get<0>(colliderComponent->Shape).HalfSize.x);
                    return 1;
                case 1:
                    stack.PushValue(std::get<1>(colliderComponent->Shape).Radius);
                    return 1;
                case 2:
                    stack.PushValue(std::get<2>(colliderComponent->Shape).A);
                    return 1;
                default:
                    assert(false);
                    return 0;
            }
        case ScriptObjectAttributes::ColliderY:
            if (!(colliderComponent = ent.TryGetComponent<Collider>()))
                return 0;
            switch (colliderComponent->Shape.index())
            {
                case 0:
                    stack.PushValue(std::get<0>(colliderComponent->Shape).HalfSize.y);
                    return 1;
                case 1:
                    stack.PushValue(std::get<1>(colliderComponent->Shape).Radius);
                    return 1;
                case 2:
                    stack.PushValue(std::get<2>(colliderComponent->Shape).B);
                    return 1;
                default:
                    assert(false);
                    return 0;
            }
        case ScriptObjectAttributes::RectangleCollider:
            if (!(colliderComponent = ent.TryGetComponent<Collider>()))
                return 0;
            stack.PushValue(colliderComponent->Shape.index() == 0);
            return 1;
        case ScriptObjectAttributes::Image:
            if (!(rendererComponent = ent.TryGetComponent<Renderer>()))
            {
                return 0;
            }
            else
            {
                string_view name = rendererComponent->GetAssetName();
                if (name.empty())
                    return 0;
                stack.PushValue(ExtractAssetName(name));  // 兼容 lstg：返回无前缀资源名
                return 1;
            }
        case ScriptObjectAttributes::Animation:
            if (!(rendererComponent = ent.TryGetComponent<Renderer>()))
                return 0;
            if (rendererComponent->RenderData.index() == 2)
            {
                auto& ani = std::get<2>(rendererComponent->RenderData);
                stack.PushValue(ani.Timer);
                return 1;
            }
            stack.PushValue(0);
            return 1;
        default:
            return 0;
    }
}

bool GameWorld::OnSetAttribute(LuaStack stack, ECS::EntityId id, std::string_view key, LuaStack::AbsIndex value)
{
    auto attr = TranslateScriptObjectAttributes(key);
    if (!attr)
        return false;

    ECS::Entity ent {&m_stWorld, id};
    Transform* transformComponent = nullptr;
    Movement* movementComponent = nullptr;
    LifeTime* lifeTimeComponent = nullptr;
    Renderer* rendererComponent = nullptr;
    Collider* colliderComponent = nullptr;
    Script* scriptComponent = nullptr;

    switch (*attr)
    {
        case ScriptObjectAttributes::X:
            if (!(transformComponent = ent.TryGetComponent<Transform>()))
                return false;
            transformComponent->Location.x = stack.ReadValue<double>(value);
            return true;
        case ScriptObjectAttributes::Y:
            if (!(transformComponent = ent.TryGetComponent<Transform>()))
                return false;
            transformComponent->Location.y = stack.ReadValue<double>(value);
            return true;
        case ScriptObjectAttributes::DeltaX:
            stack.Error("property 'dx' is readonly");
            return false;
        case ScriptObjectAttributes::DeltaY:
            stack.Error("property 'dy' is readonly");
            return false;
        case ScriptObjectAttributes::Rotation:
            if (!(transformComponent = ent.TryGetComponent<Transform>()))
                return false;
            transformComponent->Rotation = glm::radians(stack.ReadValue<double>(value));
            return true;
        case ScriptObjectAttributes::AngularVelocity:
            if (!(movementComponent = ent.TryGetComponent<Movement>()))
                return false;
            movementComponent->AngularVelocity = glm::radians(stack.ReadValue<double>(value));
            return true;
        case ScriptObjectAttributes::Timer:
            if (!(lifeTimeComponent = ent.TryGetComponent<LifeTime>()))
                return false;
            lifeTimeComponent->Timer = static_cast<uint32_t>(max(0, stack.ReadValue<int32_t>(value)));
            return true;
        case ScriptObjectAttributes::VelocityX:
            if (!(movementComponent = ent.TryGetComponent<Movement>()))
                return false;
            movementComponent->Velocity.x = stack.ReadValue<double>(value);
            return true;
        case ScriptObjectAttributes::VelocityY:
            if (!(movementComponent = ent.TryGetComponent<Movement>()))
                return false;
            movementComponent->Velocity.y = stack.ReadValue<double>(value);
            return true;
        case ScriptObjectAttributes::AccelVelocityX:
            if (!(movementComponent = ent.TryGetComponent<Movement>()))
                return false;
            movementComponent->AccelVelocity.x = stack.ReadValue<double>(value);
            return true;
        case ScriptObjectAttributes::AccelVelocityY:
            if (!(movementComponent = ent.TryGetComponent<Movement>()))
                return false;
            movementComponent->AccelVelocity.y = stack.ReadValue<double>(value);
            return true;
        case ScriptObjectAttributes::Layer:
            if (!(rendererComponent = ent.TryGetComponent<Renderer>()))
                return false;
            rendererComponent->Layer = stack.ReadValue<double>(value);
            ListInsertSort(rendererComponent, RendererSortFunction);  // 刷新渲染顺序
            return true;
        case ScriptObjectAttributes::Group:
            if (!(colliderComponent = ent.TryGetComponent<Collider>()))
            {
                return false;
            }
            else
            {
                auto group = static_cast<uint32_t>(max(0, stack.ReadValue<int32_t>(value)));
                if (group >= kColliderGroupCount)
                    group = 0;
                assert(0 <= group && group < kColliderGroupCount);
                if (group != colliderComponent->Group)
                {
                    ListRemove(colliderComponent);  // 从原先的组脱离
                    colliderComponent->Group = group;
                    ListInsertBefore(&m_pColliderRoot->ColliderGroupTailers[group], colliderComponent);  // 插入新的组
                    ListInsertSort(colliderComponent, ColliderSortFunction);  // 排序
                }
            }
            return true;
        case ScriptObjectAttributes::Invisible:
            if (!(rendererComponent = ent.TryGetComponent<Renderer>()))
                return false;
            rendererComponent->Invisible = stack.ReadValue<bool>(value);
            return true;
        case ScriptObjectAttributes::BoundaryCheck:
            if (!(lifeTimeComponent = ent.TryGetComponent<LifeTime>()))
                return false;
            lifeTimeComponent->OutOfBoundaryAutoRemove = stack.ReadValue<bool>(value);
            return true;
        case ScriptObjectAttributes::TrackDirection:
            if (!(movementComponent = ent.TryGetComponent<Movement>()))
                return false;
            movementComponent->RotateToSpeedDirection = stack.ReadValue<bool>(value);
            return true;
        case ScriptObjectAttributes::CollisionCheck:
            if (!(colliderComponent = ent.TryGetComponent<Collider>()))
                return false;
            colliderComponent->Enabled = stack.ReadValue<bool>(value);
            return true;
        case ScriptObjectAttributes::Status:
            if (!(lifeTimeComponent = ent.TryGetComponent<LifeTime>()))
            {
                return false;
            }
            else
            {
                auto val = stack.ReadValue<const char*>(value);
                if (::strcmp(val, "normal") == 0)
                    lifeTimeComponent->Status = LifeTimeStatus::Alive;
                else if (::strcmp(val, "del") == 0)
                    lifeTimeComponent->Status = LifeTimeStatus::Deleted;
                else if (::strcmp(val, "kill") == 0)
                    lifeTimeComponent->Status = LifeTimeStatus::Killed;
                else
                    stack.Error("invalid argument for property 'status'.");
            }
            return true;
        case ScriptObjectAttributes::ScaleX:
            if (!(rendererComponent = ent.TryGetComponent<Renderer>()))
                return false;
            rendererComponent->Scale.x = stack.ReadValue<double>(value);
            return true;
        case ScriptObjectAttributes::ScaleY:
            if (!(rendererComponent = ent.TryGetComponent<Renderer>()))
                return false;
            rendererComponent->Scale.y = stack.ReadValue<double>(value);
            return true;
        case ScriptObjectAttributes::Class:
            if (!(scriptComponent = ent.TryGetComponent<Script>()))
                return 0;
            assert(scriptComponent->Pool == &m_stScriptObjectPool);
            m_stScriptObjectPool.PushScriptObject(stack, scriptComponent->ScriptObjectId);
            if (stack.TypeOf(-1) == LUA_TNIL)
            {
                assert(false);
                stack.Pop(1);
                return false;
            }
            stack.PushValue(value);
            stack.RawSet(-2, kIndexOfClassInObject);
            stack.Pop(1);
            return true;
        case ScriptObjectAttributes::ColliderX:
            if (!(colliderComponent = ent.TryGetComponent<Collider>()))
                return false;
            switch (colliderComponent->Shape.index())
            {
                case 0:
                    std::get<0>(colliderComponent->Shape).HalfSize.x = stack.ReadValue<double>(value);
                    break;
                case 1:
                    {
                        auto& circleShape = std::get<1>(colliderComponent->Shape);
                        auto v = stack.ReadValue<double>(value);
                        if (v != circleShape.Radius)  // 提升为椭圆
                        {
                            auto ellipseShape = Math::Collider2D::EllipseShape<double>();
                            ellipseShape.A = v;
                            ellipseShape.B = circleShape.Radius;
                            colliderComponent->Shape = ellipseShape;
                        }
                    }
                    break;
                case 2:
                    {
                        auto& ellipseShape = std::get<2>(colliderComponent->Shape);
                        ellipseShape.A = stack.ReadValue<double>(value);
                        if (ellipseShape.A == ellipseShape.B)  // 降级为圆
                        {
                            auto circleShape = Math::Collider2D::CircleShape<double>();
                            circleShape.Radius = ellipseShape.A;
                            colliderComponent->Shape = circleShape;
                        }
                    }
                    break;
                default:
                    assert(false);
                    return false;
            }
            colliderComponent->RefreshAABB();
            return true;
        case ScriptObjectAttributes::ColliderY:
            if (!(colliderComponent = ent.TryGetComponent<Collider>()))
                return false;
            switch (colliderComponent->Shape.index())
            {
                case 0:
                    std::get<0>(colliderComponent->Shape).HalfSize.y = stack.ReadValue<double>(value);
                    break;
                case 1:
                    {
                        auto& circleShape = std::get<1>(colliderComponent->Shape);
                        auto v = stack.ReadValue<double>(value);
                        if (v != circleShape.Radius)  // 提升为椭圆
                        {
                            auto ellipseShape = Math::Collider2D::EllipseShape<double>();
                            ellipseShape.A = circleShape.Radius;
                            ellipseShape.B = v;
                            colliderComponent->Shape = ellipseShape;
                        }
                    }
                    break;
                case 2:
                    {
                        auto& ellipseShape = std::get<2>(colliderComponent->Shape);
                        ellipseShape.B = stack.ReadValue<double>(value);
                        if (ellipseShape.A == ellipseShape.B)  // 降级为圆
                        {
                            auto circleShape = Math::Collider2D::CircleShape<double>();
                            circleShape.Radius = ellipseShape.A;
                            colliderComponent->Shape = circleShape;
                        }
                    }
                    break;
                default:
                    assert(false);
                    return false;
            }
            colliderComponent->RefreshAABB();
            return true;
        case ScriptObjectAttributes::RectangleCollider:
            if (!(colliderComponent = ent.TryGetComponent<Collider>()))
            {
                return false;
            }
            else
            {
                auto v = stack.ReadValue<bool>(value);
                switch (colliderComponent->Shape.index())
                {
                    case 0:
                        if (!v)
                        {
                            auto& obbShape = std::get<0>(colliderComponent->Shape);
                            if (obbShape.HalfSize.x == obbShape.HalfSize.y)
                            {
                                // 提升为圆形碰撞
                                auto circleShape = Math::Collider2D::CircleShape<double>();
                                circleShape.Radius = obbShape.HalfSize.x;
                                colliderComponent->Shape = circleShape;
                            }
                            else
                            {
                                // 提升为椭圆碰撞
                                auto ellipseShape = Math::Collider2D::EllipseShape<double>();
                                ellipseShape.A = obbShape.HalfSize.x;
                                ellipseShape.B = obbShape.HalfSize.y;
                                colliderComponent->Shape = ellipseShape;
                            }
                        }
                        break;
                    case 1:
                        if (v)
                        {
                            auto r = std::get<1>(colliderComponent->Shape).Radius;
                            auto obbShape = Math::Collider2D::OBBShape<double>();
                            obbShape.HalfSize.x = obbShape.HalfSize.y = r;
                            colliderComponent->Shape = obbShape;
                        }
                        break;
                    case 2:
                        if (v)
                        {
                            auto obbShape = Math::Collider2D::OBBShape<double>();
                            obbShape.HalfSize.x = std::get<2>(colliderComponent->Shape).A;
                            obbShape.HalfSize.y = std::get<2>(colliderComponent->Shape).B;
                            colliderComponent->Shape = obbShape;
                        }
                        break;
                    default:
                        assert(false);
                        return false;
                }
                colliderComponent->RefreshAABB();
                return true;
            }
        case ScriptObjectAttributes::Image:
            if (!(rendererComponent = ent.TryGetComponent<Renderer>()))
            {
                return false;
            }
            else
            {
                if (stack.TypeOf(value) == LUA_TNIL)
                {
                    // 释放资源
                    rendererComponent->RenderData = {};
                    return true;
                }
                else
                {
                    auto newName = stack.ReadValue<const char*>(value);
                    string_view name = rendererComponent->GetAssetName();
                    if (!name.empty() && ExtractAssetName(name) == newName)  // 如果名字一样，不做任何处理
                        return true;

                    // 设置资源的时候会自动设置资源上绑定的碰撞信息
                    colliderComponent = ent.TryGetComponent<Collider>();

                    // 发起资源查找流程: Sprite -> Animation -> Particle
                    auto asset = m_stApp.GetAssetPools()->FindAsset(AssetTypes::Image, newName);
                    if (asset)
                    {
                        Renderer::SpriteRenderer spriteRenderer;
                        spriteRenderer.Asset = static_pointer_cast<v2::Asset::SpriteAsset>(asset);
                        if (colliderComponent)
                        {
                            colliderComponent->Shape = spriteRenderer.Asset->GetColliderShape();
                            colliderComponent->RefreshAABB();
                        }
                        rendererComponent->RenderData = std::move(spriteRenderer);
                        return true;
                    }

                    asset = m_stApp.GetAssetPools()->FindAsset(AssetTypes::Animation, newName);
                    if (asset)
                    {
                        Renderer::SpriteSequenceRenderer spriteSequenceRenderer;
                        spriteSequenceRenderer.Asset = static_pointer_cast<v2::Asset::SpriteSequenceAsset>(asset);
                        spriteSequenceRenderer.Timer = 0;
                        if (colliderComponent)
                        {
                            colliderComponent->Shape = spriteSequenceRenderer.Asset->GetColliderShape();
                            colliderComponent->RefreshAABB();
                        }
                        rendererComponent->RenderData = std::move(spriteSequenceRenderer);
                        return true;
                    }

                    asset = m_stApp.GetAssetPools()->FindAsset(AssetTypes::Particle, newName);
                    if (asset)
                    {
                        Renderer::ParticleRenderer particleRenderer;
                        particleRenderer.Asset = static_pointer_cast<v2::Asset::HgeParticleAsset>(asset);
                        particleRenderer.Pool = make_shared<Subsystem::Render::Drawing2D::ParticlePool>();
                        auto ret = particleRenderer.Pool->AddEmitter(&particleRenderer.Asset->GetParticleConfig());
                        if (!ret)
                        {
                            particleRenderer = {};
                            stack.Error("AddEmitter error: %s", ret.GetError().message().c_str());
                            return false;
                        }
                        particleRenderer.Emitter = &particleRenderer.Pool->GetEmitter(0);
                        assert(particleRenderer.Emitter);

                        if (colliderComponent)
                        {
                            colliderComponent->Shape = particleRenderer.Asset->GetColliderShape();
                            colliderComponent->RefreshAABB();
                        }
                        rendererComponent->RenderData = std::move(particleRenderer);
                        return true;
                    }

                    stack.Error("can't find resource '%s' in image/animation/particle pool.", newName);
                    return false;
                }
            }
        case ScriptObjectAttributes::Animation:
            stack.Error("property 'ani' is readonly");
            return false;
        default:
            return false;
    }
}
