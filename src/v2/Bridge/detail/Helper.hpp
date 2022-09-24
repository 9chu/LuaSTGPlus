/**
 * @file
 * @author 9chu
 * @date 2022/5/15
 * 此文件为 LuaSTGPlus 项目的一部分，版权与许可声明详见 COPYRIGHT.txt。
 */
#pragma once
#include <lstg/v2/GameApp.hpp>

namespace lstg::v2::Bridge::detail
{
    /**
     * 获取全局 APP 对象
     */
    inline GameApp& GetGlobalApp() noexcept
    {
        return *static_cast<GameApp*>(&GameApp::GetInstance());
    }

    /**
     * 解析绝对路径或相对路径
     *
     * 为了方便资源组织，且为了考虑兼容性，我们允许采取特殊的方式标记一个相对路径：
     *   data/asset/1.png -> 绝对路径 /data/asset/1.png
     *   ./asset/1.png -> 相对于脚本路径，若脚本位于 /data/1.lua，则解析为 /data/asset/1.png
     *
     * @param path 路径
     * @return 解析结果
     */
    std::string ResolveAbsoluteOrRelativePath(Subsystem::Script::LuaStack& stack, std::string_view path) noexcept;
}

#define LSTG_LOG_DEPRECATED(MODULE, METHOD) \
    do \
    { \
        static bool kPrinted = false; \
        if (!kPrinted) \
        { \
            LSTG_LOG_WARN_CAT(MODULE, #METHOD " is deprecated and has no effect anymore"); \
            kPrinted = true; \
        } \
    } while (false)
