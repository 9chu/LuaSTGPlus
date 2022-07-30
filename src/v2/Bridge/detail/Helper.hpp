/**
 * @file
 * @author 9chu
 * @date 2022/5/15
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
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
