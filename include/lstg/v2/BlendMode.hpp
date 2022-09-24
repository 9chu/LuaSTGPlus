/**
 * @file
 * @date 2022/5/31
 * @author 9chu
 * 此文件为 LuaSTGPlus 项目的一部分，版权与许可声明详见 COPYRIGHT.txt。
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
