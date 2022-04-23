/**
 * @file
 * @author 9chu
 * @date 2022/4/21
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#pragma once
#include "../GameApp.hpp"

namespace lstg::v2::Bridge
{
    /**
     * 获取全局 APP 对象
     */
    inline GameApp& GetApp() noexcept
    {
        return *static_cast<GameApp*>(&GameApp::GetInstance());
    }
}
