/**
 * @file
 * @date 2022/7/26
 * @author 9chu
 * 此文件为 LuaSTGPlus 项目的一部分，版权与许可声明详见 COPYRIGHT.txt。
 */
#pragma once
#include <cstdint>
#include <cstdlib>

namespace lstg::v2::GamePlay
{
    class ScriptObjectPool;
}

namespace lstg::v2::GamePlay::Components
{
    /**
     * 脚本组件
     */
    struct Script
    {
        /**
         * 在脚本系统中关联对象的 ID
         */
        uint32_t ScriptObjectId = 0;

        /**
         * 脚本对象池
         */
        ScriptObjectPool* Pool = nullptr;

        void Reset() noexcept;
    };

    constexpr uint32_t GetComponentId(Script*) noexcept
    {
        return 8u;
    }
}
