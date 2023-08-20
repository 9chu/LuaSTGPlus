/**
 * @file
 * @date 2023/8/20
 * @author 9chu
 * 此文件为 LuaSTGPlus 项目的一部分，版权与许可声明详见 COPYRIGHT.txt。
 */
#pragma once
#include <string>
#include "RenderDevice.hpp"

namespace lstg::Subsystem::Render
{
    /**
     * 渲染事件类型
     */
    enum class RenderEventTypes
    {
        None,
        SwapBufferResized,
    };

    /**
     * 渲染系统事件
     */
    struct RenderEvent
    {
        struct SwapBufferResizeArgs
        {
            uint32_t Width;
            uint32_t Height;
        };

        RenderEventTypes Type = RenderEventTypes::None;

        union {
            SwapBufferResizeArgs SwapBufferResize;
        };
    };
} // lstg::Subsystem::Render
