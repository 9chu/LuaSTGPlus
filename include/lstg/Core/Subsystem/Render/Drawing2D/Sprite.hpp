/**
 * @file
 * @date 2022/7/9
 * @author 9chu
 * 此文件为 LuaSTGPlus 项目的一部分，版权与许可声明详见 COPYRIGHT.txt。
 */
#pragma once
#include "../../../Math/Rectangle.hpp"
#include "Texture2D.hpp"
#include "SpriteDrawing.hpp"

namespace lstg::Subsystem::Render::Drawing2D
{
    /**
     * 精灵
     */
    class Sprite
    {
    public:
        /**
         * 获取关联的纹理
         */
        [[nodiscard]] const Texture2D* GetTexture2D() const noexcept;

        /**
         * 设置关联的纹理
         * @note 不持有对象
         * @param tex 纹理指针
         * @param updateVertex 是否更新顶点
         */
        void SetTexture2D(const Texture2D* tex, bool updateVertex = true) noexcept;

        /**
         * 获取帧
         */
        [[nodiscard]] const Math::ImageRectangleFloat& GetFrame() const noexcept;

        /**
         * 设置帧
         * @param rect 图像范围
         * @param updateVertex 是否更新顶点
         */
        void SetFrame(const Math::ImageRectangleFloat& rect, bool updateVertex = true) noexcept;

        /**
         * 获取锚点
         */
        [[nodiscard]] const glm::vec2& GetAnchor() const noexcept;

        /**
         * 设置锚点
         * @param vec 锚点，相对于 Frame 左上角
         * @param updateVertex 是否更新顶点
         */
        void SetAnchor(glm::vec2 vec, bool updateVertex = true) noexcept;

        /**
         * 获取默认的混合模式
         */
        [[nodiscard]] ColorBlendMode GetColorBlendMode() const noexcept;

        /**
         * 设置默认的混合模式
         * @param m 混合模式
         */
        void SetColorBlendMode(ColorBlendMode m) noexcept;

        /**
         * 获取加算颜色
         */
        [[nodiscard]] const SpriteColorComponents& GetAdditiveBlendColor() const noexcept;

        /**
         * 设置加算颜色
         * @param color 颜色
         * @param updateVertex 是否更新顶点
         */
        void SetAdditiveBlendColor(const SpriteColorComponents& color, bool updateVertex = true) noexcept;

        /**
         * 获取乘算颜色
         */
        [[nodiscard]] const SpriteColorComponents& GetMultiplyBlendColor() const noexcept;

        /**
         * 设置乘算颜色
         * @param color 颜色
         * @param updateVertex 是否更新顶点
         */
        void SetMultiplyBlendColor(const SpriteColorComponents& color, bool updateVertex = true) noexcept;

        /**
         * 获取预先计算的顶点
         * 这里的顶点会预先填充好偏移、纹理坐标、颜色等数据，用于加速渲染。
         */
        const std::array<Vertex, 4>& GetPrecomputedVertex() const noexcept;

        /**
         * 更新预计算顶点
         * @note 当依赖的纹理变化时，可以使用该方法强制触发预计算顶点的刷新
         */
        void UpdatePrecomputedVertex() noexcept;

        /**
         * 绘制
         * 使用预计算顶点绘制基本形状。
         * @param buffer 绘制缓冲区
         * @param blendModeOverride 混合模式覆盖
         * @return 绘制工具
         */
        Result<SpriteDrawing> Draw(CommandBuffer& buffer, std::optional<ColorBlendMode> blendModeOverride = {}) const noexcept;

    private:
        void PrecomputedVertex(int what) noexcept;

    private:
        const Texture2D* m_pTexture = nullptr;
        Math::ImageRectangleFloat m_stFrame;
        glm::vec2 m_stAnchor;
        ColorBlendMode m_iBlendMode = ColorBlendMode::Alpha;
        SpriteColorComponents m_stAdditiveBlendColor = { 0x000000FFu, 0x000000FFu, 0x000000FFu, 0x000000FFu };
        SpriteColorComponents m_stMultiplyBlendColor = { 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu };
        std::array<Vertex, 4> m_stPrecomputedVertex;
    };
}
