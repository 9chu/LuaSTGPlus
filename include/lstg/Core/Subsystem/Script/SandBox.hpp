/**
 * @file
 * @date 2022/3/3
 * @author 9chu
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#pragma once
#include <map>
#include "LuaStack.hpp"
#include "LuaReference.hpp"
#include "../VFS/Path.hpp"

namespace lstg::Subsystem::Script
{
    /**
     * 沙箱机制
     *
     * 沙箱机制使得只有当用户显式使用 _G 进行定义，否则所有文件内的符号不会污染全局。
     * 同时沙箱自带自动重载机制（SHIPPING 模式默认关闭）。
     */
    class SandBox
    {
    public:
        SandBox(LuaStack mainThread, std::string_view baseDirectory);

    public:
        /**
         * 获取检查间隔
         */
        double GetCheckInterval() const noexcept { return m_dCheckInterval; }

        /**
         * 设置检查间隔
         * @param checkInterval 间隔，设置为 0 时关闭自动检查。
         */
        void SetCheckInterval(double checkInterval) noexcept { m_dCheckInterval = checkInterval; }

        /**
         * 获取基准目录
         */
        const VFS::Path& GetBaseDirectory() const noexcept { return m_stBaseDirectory; }

        /**
         * 加载文件
         * @param path 路径
         */
        Result<void> ImportScript(std::string_view path) noexcept;

        /**
         * 更新状态
         * 检查哪些文件需要被热更新。
         * @param elapsedTime 流逝的时间（秒）
         */
        void Update(double elapsedTime) noexcept;

    private:
        /**
         * 模块定位器
         * @param modname 模块名
         * @return 返回 ModuleLoader
         */
        Result<LuaReference> ModuleLocator(const char* modname) noexcept;

    private:
        struct ImportedFile
        {
            std::string Path;
            time_t LastModifiedTime;
            LuaReference Env;
            LuaReference ModuleLoader;  // for returning ENV
        };

        LuaStack m_stMainThread;

        const VFS::Path m_stBaseDirectory;
        double m_dCheckInterval = 0;

        std::map<std::string, ImportedFile> m_stFiles;
        double m_dCheckTimer = 0;
    };
}
