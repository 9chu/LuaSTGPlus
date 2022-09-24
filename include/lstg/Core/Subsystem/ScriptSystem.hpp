/**
 * @file
 * @date 2022/3/2
 * @author 9chu
 * 此文件为 LuaSTGPlus 项目的一部分，版权与许可声明详见 COPYRIGHT.txt。
 */
#pragma once
#include "ISubsystem.hpp"
#include "Script/LuaState.hpp"
#include "Script/SandBox.hpp"

namespace lstg::Subsystem
{
    /**
     * 脚本子系统
     */
    class ScriptSystem :
        public ISubsystem
    {
    public:
        explicit ScriptSystem(SubsystemContainer& container);
        ScriptSystem(const ScriptSystem&) = delete;
        ScriptSystem(ScriptSystem&&) noexcept = delete;

    public:
        /**
         * 获取虚拟机状态
         */
        [[nodiscard]] Script::LuaState& GetState() noexcept { return m_stState; }
        [[nodiscard]] const Script::LuaState& GetState() const noexcept { return m_stState; }

        /**
         * 获取沙箱对象
         */
        [[nodiscard]] Script::SandBox& GetSandBox() noexcept { return m_stSandBox; }
        [[nodiscard]] const Script::SandBox& GetSandBox() const noexcept { return m_stSandBox; }

        /**
         * 加载脚本
         * 注意：不检查文件是否已经加载，沙箱模式会强制重新加载。
         * @param path 路径，注意非沙箱情况会参考沙箱中设置的 BaseDirectory
         * @param sandbox 沙箱模式加载
         */
        Result<void> LoadScript(std::string_view path, bool sandbox = false) noexcept;

        /**
         * 调用全局方法
         * @param TRet 返回值类型
         * @tparam TArgs 参数类型
         * @param func 方法名
         * @param args 参数列表
         * @return 是否成功
         */
        template <typename TRet, typename... TArgs>
        Result<TRet> CallGlobal(const char* func, TArgs&&... args) noexcept
        {
            Script::LuaStack::BalanceChecker stackChecker(m_stState);

            // 准备足够栈大小
            if (!::lua_checkstack(m_stState, sizeof...(args) + 1))
                return std::make_error_code(std::errc::not_enough_memory);

            // 获取方法
            // nil 的情况交给 lua 处理，统一报错
            m_stState.GetGlobal(func);

            // 推入参数
            auto values = m_stState.PushValues(std::forward<TArgs>(args)...);

            if constexpr (std::is_same_v<TRet, void>)
            {
                // 执行
                auto call = m_stState.ProtectedCallWithTraceback(values, 0);
                if (!call)
                {
                    LogCallFail(func, lua_tostring(m_stState, -1));
                    lua_pop(m_stState, 1);
                    return call.GetError();
                }
                return {};
            }
            else
            {
                // 执行
                auto call = m_stState.ProtectedCallWithTraceback(values, Script::detail::CountArgs<TRet>::Value);
                if (!call)
                {
                    LogCallFail(func, lua_tostring(m_stState, -1));
                    lua_pop(m_stState, 1);
                    return call.GetError();
                }
                auto ret = m_stState.ReadValue<TRet>(lua_gettop(m_stState) - Script::detail::CountArgs<TRet>::Value + 1);
                lua_pop(m_stState, Script::detail::CountArgs<TRet>::Value);
                return ret;
            }
        }

    protected:  // ISubsystem
        /**
         * 更新状态
         * @param elapsedTime 流逝时间
         */
        void OnUpdate(double elapsedTime) noexcept override;

    private:
        void LogCallFail(const char* func, const char* what) noexcept;

    private:
        Script::LuaState m_stState;
        Script::SandBox m_stSandBox;
    };
}
