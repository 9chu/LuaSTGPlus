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
#include "../VirtualFileSystem.hpp"

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
        SandBox(VirtualFileSystem& fs, LuaStack mainThread);

    public:
        /**
         * 获取关联的虚拟文件系统
         */
        [[nodiscard]] VirtualFileSystem& GetVirtualFileSystem() noexcept { return m_stFileSystem; }

        /**
         * 获取关联的主线程
         */
        [[nodiscard]] LuaStack& GetMainThread() noexcept { return m_stMainThread; }
        [[nodiscard]] const LuaStack& GetMainThread() const noexcept { return m_stMainThread; }

        /**
         * 获取检查间隔
         */
        [[nodiscard]] double GetCheckInterval() const noexcept { return m_dCheckInterval; }

        /**
         * 设置检查间隔
         * @param checkInterval 间隔，设置为 0 时关闭自动检查。
         */
        void SetCheckInterval(double checkInterval) noexcept { m_dCheckInterval = checkInterval; }

        /**
         * 加载文件
         * @param path 路径
         * @param force 强制加载，如果文件已经加载过了也进行加载
         * @return 返回环境
         */
        Result<LuaReference> ImportScript(std::string_view path, bool force = false) noexcept;

        /**
         * 更新状态
         * 检查哪些文件需要被热更新。
         * @param elapsedTime 流逝的时间（秒）
         */
        void Update(double elapsedTime) noexcept;

    private:
        struct ImportedFile
        {
            VFS::Path Path;
            std::string LuaChunkName;
            time_t LastModified;
            LuaReference Env;
            LuaReference ModuleLoader;  // for returning ENV
        };

        /**
         * 获取完整路径
         * @param modname 模块名
         * @return 完整路径
         */
        Result<VFS::Path> MakeFullPath(std::string_view modname) noexcept;

        /**
         * 模块定位器
         * @param modname 模块名
         * @param forceReload 强制重新加载
         * @return 返回 ModuleLoader
         */
        Result<ImportedFile*> ModuleLocator(std::string_view modname, bool forceReload = false) noexcept;

        /**
         * 执行文件
         * @param file 导入的文件信息
         * @return 是否成功
         */
        Result<void> ExecFile(ImportedFile& file);

    private:
        VirtualFileSystem& m_stFileSystem;
        LuaStack m_stMainThread;

        double m_dCheckInterval = 0;

        std::map<std::string, ImportedFile, std::less<>> m_stFiles;
        double m_dCheckTimer = 0;
    };
}
