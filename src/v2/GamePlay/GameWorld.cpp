/**
 * @file
 * @date 2022/7/24
 * @author 9chu
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#include <lstg/v2/GamePlay/GameWorld.hpp>

#include <lstg/Core/Logging.hpp>
#include <lstg/Core/Subsystem/ScriptSystem.hpp>
#include <lstg/Core/Subsystem/Render/Drawing2D/SpriteDrawing.hpp>
#include <lstg/v2/GameApp.hpp>
#include <lstg/v2/GamePlay/Components/Collider.hpp>
#include <lstg/v2/GamePlay/Components/LifeTime.hpp>
#include <lstg/v2/GamePlay/Components/Movement.hpp>
#include <lstg/v2/GamePlay/Components/Renderer.hpp>
#include <lstg/v2/GamePlay/Components/Script.hpp>
#include <lstg/v2/GamePlay/Components/Transform.hpp>

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
        if (renderer.RenderData.index() == 1)
        {
            auto& spriteSequenceData = std::get<Renderer::SpriteSequenceRenderer>(renderer.RenderData);
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
    // TODO
    return 0;
}

bool GameWorld::OnSetAttribute(LuaStack stack, ECS::EntityId id, std::string_view key, LuaStack::AbsIndex value)
{
    // TODO
    return false;
}
