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
     * 游戏对象模块
     */
    LSTG_MODULE(lstg, GLOBAL)
    class GameObjectModule
    {
    public:
        /**
         * 获取对象表
         * @warning 慎用
         */
        LSTG_METHOD(ObjTable)
        static Subsystem::Script::LuaStack::AbsIndex GetObjectTable();
    };
}
