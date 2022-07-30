/**
 * @file
 * @date 2022/7/26
 * @author 9chu
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#pragma once
#include <optional>
#include <unordered_map>
#include <lstg/Core/Subsystem/Script/LuaState.hpp>
#include <lstg/Core/Subsystem/Script/LuaReference.hpp>
#include <lstg/Core/ECS/Entity.hpp>

namespace lstg::v2::GamePlay
{
    /**
     * 脚本回调方法
     */
    enum class ScriptCallbackFunctions
    {
        OnInit = 1,
        OnDelete = 2,
        OnFrame = 3,
        OnRender = 4,
        OnCollision = 5,
        OnKill = 6,
    };

    const char* ToString(ScriptCallbackFunctions functions) noexcept;

    /**
     * 脚本桥
     * 用于 Lua -> C++
     */
    class IScriptObjectBridge
    {
    public:
        /**
         * 当访问对象属性时调用
         * @param stack Lua栈
         * @param id 对象ID
         * @param key 属性名
         * @return 返回的值个数（位于Lua栈顶）
         */
        virtual int OnGetAttribute(Subsystem::Script::LuaStack stack, ECS::EntityId id, std::string_view key) = 0;

        /**
         * 当设置对象属性时调用
         * @param stack Lua栈
         * @param id 对象ID
         * @param key 属性名
         * @param value 值
         * @return 是否成功设置，当值不存在时返回 false
         */
        virtual bool OnSetAttribute(Subsystem::Script::LuaStack stack, ECS::EntityId id, std::string_view key,
            Subsystem::Script::LuaStack::AbsIndex value) = 0;
    };

    using ScriptObjectId = uint32_t;

    static constexpr int kIndexOfClassInObject = 1;
    static constexpr int kIndexOfScriptObjectIdInObject = 2;

    enum class ScriptCallbackInvokeResult
    {
        Ok,
        Disposed,
        InvalidClass,
        CallbackNotDefined,
        InvalidCallback,
        CallError,
    };

    /**
     * 脚本对象池
     */
    class ScriptObjectPool
    {
    public:
        ScriptObjectPool(Subsystem::Script::LuaState& state, IScriptObjectBridge* bridge);
        ~ScriptObjectPool();

    public:
        /**
         * 获取虚拟机
         */
        Subsystem::Script::LuaState& GetState() const noexcept { return m_stState; }

        /**
         * 是否在主线程
         * @param stack Lua栈
         * @return 检查结果
         */
        bool IsOnMainThread(Subsystem::Script::LuaStack& stack) const noexcept { return stack == GetState(); }

        /**
         * 获取脚本对象桥
         */
        IScriptObjectBridge* GetScriptObjectBridge() noexcept { return m_pBridge; }

        /**
         * 获取当前的对象个数
         */
        size_t GetCurrentObjects() const noexcept { return m_uCurrentObjects; }

        /**
         * 申请对象
         * [-0, +1]
         * @param stack Lua栈
         * @param classIndex Class对象在栈上的索引
         * @param id 对象的实例ID
         * @return 脚本侧对象ID
         */
        Result<std::tuple<ScriptObjectId, Subsystem::Script::LuaStack::AbsIndex>> Alloc(Subsystem::Script::LuaStack stack,
            Subsystem::Script::LuaStack::AbsIndex classIndex, ECS::EntityId id) noexcept;

        /**
         * 回收对象
         * @param stack Lua栈
         * @param scriptId 脚本侧对象ID
         */
        void Free(Subsystem::Script::LuaStack stack, ScriptObjectId scriptId) noexcept;

        /**
         * 在 Lua 栈上推入对象
         * [-0, +1]
         * @param stack 栈
         * @param scriptId 对象ID
         */
        void PushScriptObject(Subsystem::Script::LuaStack stack, ScriptObjectId scriptId) noexcept;

        /**
         * 调用回调函数
         * [-args, +0]
         * @param stack Lua栈
         * @param scriptId 脚本侧对象ID
         * @param callback 回调方法
         * @param args 参数个数
         */
        ScriptCallbackInvokeResult InvokeCallback(Subsystem::Script::LuaStack stack, ScriptObjectId scriptId, ScriptCallbackFunctions callback,
            unsigned args) noexcept;

        /**
         * 获取对象实例ID
         * @param scriptObjectId 脚本对象ID
         * @return 实例ID
         */
        std::optional<ECS::EntityId> GetEntityId(ScriptObjectId scriptObjectId) noexcept;

    private:
        Subsystem::Script::LuaState& m_stState;
        IScriptObjectBridge* m_pBridge = nullptr;

        Subsystem::Script::LuaReference m_stObjectTableRef;
        Subsystem::Script::LuaReference m_stObjectMetaTableRef;

        // 由于 luajit 不能存储 int64_t，我们需要中间表进行转换
        std::unordered_map<ScriptObjectId, ECS::EntityId> m_stEntityIdMapping;

        ScriptObjectId m_uNextObjectId = 1;
        size_t m_uCurrentObjects = 0;
    };
}
