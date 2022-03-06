/**
 * @file
 * @date 2022/3/2
 * @author 9chu
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
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
         * 更新状态
         * @param elapsedTime 流逝时间
         */
        void Update(double elapsedTime) noexcept;

    private:
        Script::LuaState m_stState;
        Script::SandBox m_stSandBox;
    };
}
