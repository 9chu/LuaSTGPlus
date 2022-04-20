/**
 * @file
 * @author 9chu
 * @date 2022/4/20
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#pragma once
#include <lstg/Core/Subsystem/Script/LuaRead.hpp>
#include <lstg/Core/Subsystem/Script/AutoBridgeHint.hpp>

namespace lstg::v2::Bridge
{
    /**
     * 系统模块
     */
    LSTG_MODULE(lstg, GLOBAL)
    class SystemModule
    {
    public:
        using LuaStack = Subsystem::Script::LuaStack;

    public:
        /**
         * 设置是否窗口化
         * @param windowed 是否窗口化
         */
        static void SetWindowed(bool windowed);

        /**
         * 设置目标 FPS
         * @param fps FPS
         */
        static void SetFPS(int32_t fps);

        /**
         * 获取实时FPS计数
         */
        static int32_t GetFPS();

        /**
         * 设置是否垂直同步
         * @param vsync 垂直同步
         */
        static void SetVsync(bool vsync);

        /**
         * 设置目标分辨率
         * @param width 宽度
         * @param height 高度
         */
        static void SetResolution(int32_t width, int32_t height);

        /**
         * 尝试更改显示模式
         * @param width 宽度
         * @param height 高度
         * @param windowed 窗口化
         * @param vsync 垂直同步
         * @return 是否成功
         */
        static bool ChangeVideoMode(int32_t width, int32_t height, bool windowed, bool vsync);

        /**
         * 设置是否显示光标
         * @param shown 是否显示
         */
        static void SetSplash(bool shown);

        /**
         * 设置窗口标题
         * @param title 标题
         */
        static void SetTitle(const char* title);

        /**
         * 写日志
         * @param stack Lua栈
         * @param what 日志内容
         */
        static void SystemLog(LuaStack& stack, std::string_view what);

        /**
         * 可变参版本的 SystemLog
         * @param stack Lua栈
         */
        static void Print(LuaStack& stack);

        /**
         * 加载压缩包
         * @note 后加载的资源包有较高的查找优先级，这意味着可以通过该机制加载资源包来覆盖基础资源包中的文件
         * @param path 本地路径
         * @param password 密钥
         */
        static void LoadPack(const char* path, std::optional<std::string_view> password);

        /**
         * 卸载指定位置的资源包
         * @param path 本地路径
         */
        static void UnloadPack(const char* path);

        /**
         * 解压资源包
         * @deprecated 已弃用方法
         * @param path 本地路径
         * @param target 目标
         */
        static void ExtractRes(const char* path, const char* target);

        /**
         * 执行指定路径的脚本
         * @note 已执行过的脚本会再次执行
         * @param path
         */
        static void DoFile(const char* path);

        /**
         * 装载载入窗口
         * @deprecated 已弃用方法
         * @param path 图片资源路径
         */
        static void ShowSplashWindow(const char* path);
    };
}
