/**
 * @file
 * @date 2022/5/31
 * @author 9chu
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#pragma once
#include <string_view>
#include <lstg/Core/Subsystem/Render/Drawing2D/CommandBuffer.hpp>

namespace lstg::v2
{
    /**
     * 颜色混合模式
     */
    using ColorBlendMode = Subsystem::Render::Drawing2D::ColorBlendMode;

    /**
     * 顶点颜色混合模式
     */
    enum VertexColorBlendMode
    {
        Additive,
        Multiply,
    };

    /**
     * 混合模式
     */
    struct BlendMode
    {
        ColorBlendMode ColorBlend = ColorBlendMode::Alpha;
        VertexColorBlendMode VertexColorBlend = VertexColorBlendMode::Multiply;

        BlendMode() = default;
        BlendMode(std::string_view str) noexcept;

        /**
         * 转到字符串
         * @param m 混合模式
         * @return 混合模式字符串
         */
        const char* ToString() noexcept;
    };
}
