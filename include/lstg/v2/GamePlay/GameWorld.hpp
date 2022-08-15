/**
 * @file
 * @date 2022/7/24
 * @author 9chu
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#pragma once
#include <lstg/Core/Subsystem/SubsystemContainer.hpp>
#include <lstg/Core/ECS/World.hpp>
#include "ScriptObjectPool.hpp"
#include "../MathAlias.hpp"

namespace lstg::v2
{
    class GameApp;
}

namespace lstg::v2::GamePlay::Components
{
    struct LifeTimeRoot;
    struct ColliderRoot;
    struct RendererRoot;
}

namespace lstg::v2::GamePlay
{
    /**
     * 游戏世界
     */
    class GameWorld :
        public IScriptObjectBridge
    {
    public:
        GameWorld(GameApp& app);
        GameWorld(const GameWorld&) = delete;
        GameWorld(GameWorld&&) = delete;

        GameWorld& operator=(const GameWorld&) = delete;
        GameWorld& operator=(GameWorld&&) = delete;

    public:
        /**
         * 获取对象脚本池
         */
        ScriptObjectPool& GetScriptObjectPool() noexcept { return m_stScriptObjectPool; }

        /**
         * 获取根实例
         */
        ECS::Entity& GetRootEntity() noexcept { return m_stRootEntity; }

        /**
         * 获取根生命周期对象
         */
        Components::LifeTimeRoot& GetLifeTimeRoot() noexcept { return *m_pLifeTimeRoot; }

        /**
         * 获取根碰撞对象
         */
        Components::ColliderRoot& GetColliderRoot() noexcept { return *m_pColliderRoot; }

        /**
         * 获取边界
         */
        const WorldRectangle& GetBoundary() const noexcept { return m_stBoundary; }

        /**
         * 设置边界
         * @param rect 边界
         */
        void SetBoundary(const WorldRectangle& rect) noexcept { m_stBoundary = rect; }

        /**
         * 在 Lua 栈上创建实例
         * 在 classIndex + 1 到栈顶元素被作为参数传递给 OnInit 方法
         * [-args, +1]
         * @param stack 栈
         * @param classIndex Class 索引
         * @return 实例在栈上的索引
         */
        Result<Subsystem::Script::LuaStack::AbsIndex> CreateEntity(Subsystem::Script::LuaStack stack,
            Subsystem::Script::LuaStack::AbsIndex classIndex) noexcept;

        /**
         * 通过脚本 ID 查找 Entity
         * @param id 对象ID
         * @return Entity
         */
        std::optional<ECS::Entity> GetEntityByScriptObjectId(ScriptObjectId id) noexcept;

        /**
         * 在 Lua 栈上删除实例
         * 在 objectIndex + 1 到栈顶元素被作为参数传递给 OnDelete 方法
         * [-args, +0]
         * @param stack 栈
         * @param scriptId 脚本侧对象ID
         * @param args 栈顶参数个数
         */
        bool DeleteEntity(Subsystem::Script::LuaStack stack, ScriptObjectId scriptId, unsigned args) noexcept;

        /**
         * 在 Lua 栈上击杀实例
         * 在 objectIndex + 1 到栈顶元素被作为参数传递给 OnKill 方法
         * [-args, +0]
         * @param stack 栈
         * @param scriptId 脚本侧对象ID
         * @param args 栈顶参数个数
         */
        bool KillEntity(Subsystem::Script::LuaStack stack, ScriptObjectId scriptId, unsigned args) noexcept;

        /**
         * 执行一帧
         */
        void Frame() noexcept;

        /**
         * 执行渲染方法
         */
        void Render() noexcept;

        /**
         * 更新坐标
         */
        void UpdateCoordinate() noexcept;

        /**
         * 帧后更新
         */
        void AfterFrame() noexcept;

        /**
         * 删除越界对象
         */
        void DeleteOutOfBoundaryEntities() noexcept;

        /**
         * 默认渲染行为
         * @param entity 实例
         */
        void RenderEntityDefault(ECS::Entity entity) noexcept;

        /**
         * 执行碰撞检测
         * @param groupA 碰撞组A
         * @param groupB 碰撞组B
         */
        void CollisionCheck(uint32_t groupA, uint32_t groupB) noexcept;

        /**
         * 回收所有对象
         */
        void Clear() noexcept;

    public:
        /**
         * 框架内部的 Update 方法
         * 无论 Frame 是否调用，Update 总是每帧执行一次
         * @param elapsedTime 流逝时间
         */
        void Update(double elapsedTime) noexcept;

    protected:  // IScriptObjectBridge
        int OnGetAttribute(Subsystem::Script::LuaStack stack, ECS::EntityId id, std::string_view key) override;
        bool OnSetAttribute(Subsystem::Script::LuaStack stack, ECS::EntityId id, std::string_view key,
            Subsystem::Script::LuaStack::AbsIndex value) override;

    private:
        GameApp& m_stApp;
        ECS::World m_stWorld;
        ScriptObjectPool m_stScriptObjectPool;

        // 世界属性
        WorldRectangle m_stBoundary;

        // 根对象
        ECS::Entity m_stRootEntity;
        Components::LifeTimeRoot* m_pLifeTimeRoot = nullptr;
        Components::ColliderRoot* m_pColliderRoot = nullptr;
        Components::RendererRoot* m_pRendererRoot = nullptr;
    };
}
