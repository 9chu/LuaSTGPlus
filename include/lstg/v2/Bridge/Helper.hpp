/**
 * @file
 * @author 9chu
 * @date 2022/4/21
 * 此文件为 LuaSTGPlus 项目的一部分，版权与许可声明详见 COPYRIGHT.txt。
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
