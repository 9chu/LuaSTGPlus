/**
 * @file
 * @date 2022/9/27
 * @author 9chu
 * 此文件为 LuaSTGPlus 项目的一部分，版权与许可声明详见 COPYRIGHT.txt。
 */
#pragma once
#include <lstg/Core/Subsystem/Script/LuaState.hpp>

namespace lstg::Subsystem::Script::detail
{
    /**
     * Lua 兼容层
     */
    class LuaCompatLayer
    {
    public:
        /**
         * 注册兼容层
         * @param state 状态机
         */
        static void Register(LuaState& state);
    };
}
