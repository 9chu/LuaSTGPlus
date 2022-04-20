/**
 * @file
 * @author 9chu
 * @date 2022/4/17
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#pragma once
#include "LSTGColor.hpp"
#include "LSTGRandomizer.hpp"
#include "LSTGBentLaserData.hpp"

namespace lstg::v2::Bridge
{
    /**
     * 杂项模块
     */
    LSTG_MODULE(lstg, GLOBAL)
    class MiscModule
    {
    public:
        /**
         * 返回注册表
         * @deprecated 已弃用方法
         */
        LSTG_METHOD(Registry)
        static void Registry();

        /**
         * 截图
         * @param path 保存路径
         */
        LSTG_METHOD(Snapshot)
        static void CaptureSnapshot(const char* path);

        /**
         * 执行外部程序
         * @param path 路径
         * @param arguments 参数
         * @param directory 工作目录
         * @param wait 是否等待
         * @return 是否成功
         */
        LSTG_METHOD()
        static bool Execute(const char* path, std::optional<std::string_view> arguments, std::optional<const char*> directory,
            std::optional<bool> wait /* =true */);

        /**
         * 构造随机数发生器
         * 默认会以当前系统时间作为 Seed
         * @return 随机数发生器
         */
        LSTG_METHOD(Rand)
        static LSTGRandomizer NewRandomizer();

        /**
         * 构造颜色
         * @param stack Lua栈
         * @return 颜色对象
         */
        LSTG_METHOD(Color)
        static LSTGColor NewColor(Subsystem::Script::LuaStack& stack);

        /**
         * 构造曲线激光数据
         * @return 曲线激光数据
         */
        LSTG_METHOD(BentLaserData)
        static LSTGBentLaserData NewBentLaserData();
    };
}
