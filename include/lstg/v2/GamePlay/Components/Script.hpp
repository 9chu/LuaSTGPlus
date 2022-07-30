/**
 * @file
 * @date 2022/7/26
 * @author 9chu
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
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
