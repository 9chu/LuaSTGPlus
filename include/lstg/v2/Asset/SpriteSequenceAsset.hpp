/**
 * @file
 * @date 2022/6/7
 * @author 9chu
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#pragma once
#include <array>
#include <optional>
#include <lstg/Core/Subsystem/Render/Drawing2D/CommandBuffer.hpp>
#include "TextureAsset.hpp"
#include "../BlendMode.hpp"
#include "../MathAlias.hpp"

namespace lstg::v2::Asset
{
    class SpriteSequenceAssetLoader;

    /**
     * 精灵序列资源
     */
    class SpriteSequenceAsset :
        public Subsystem::Asset::Asset
    {
        friend class SpriteSequenceAssetLoader;

    public:
        using SequenceContainer = std::vector<UVRectangle>;
        using PrecomputedSequenceContainer = std::vector<std::array<Subsystem::Render::Drawing2D::Vertex, 4>>;

        [[nodiscard]] static Subsystem::Asset::AssetTypeId GetAssetTypeIdStatic() noexcept;

    public:
        SpriteSequenceAsset(std::string name, TextureAssetPtr texture, SequenceContainer frames, ColliderShape colliderShape);

    public:
        /**
         * 获取关联的纹理资产
         */
        [[nodiscard]] const Subsystem::Asset::AssetPtr& GetTexture() const noexcept { return m_pTextureAsset; }

        /**
         * 获取帧
         */
        [[nodiscard]] const SequenceContainer& GetFrames() const noexcept { return m_stFrames; }

        /**
         * 获取锚点
         */
        [[nodiscard]] const Vec2& GetAnchor() const noexcept { return m_stAnchor; }

        /**
         * 设置锚点
         */
        void SetAnchor(Vec2 vec) noexcept;

        /**
         * 获取碰撞形状
         */
        [[nodiscard]] const ColliderShape& GetColliderShape() const noexcept { return m_stColliderShape; }

        /**
         * 设置碰撞形状
         */
        void SetColliderShape(ColliderShape shape) noexcept { m_stColliderShape = shape; }

        /**
         * 获取默认的混合模式
         */
        [[nodiscard]] const BlendMode& GetDefaultBlendMode() const noexcept { return m_stDefaultBlendMode; }

        /**
         * 设置默认的混合模式
         */
        void SetDefaultBlendMode(BlendMode mode) noexcept;

        /**
         * 获取默认的混合颜色
         */
        [[nodiscard]] const std::array<ColorRGBA32, 4>& GetDefaultBlendColor() const noexcept { return m_stDefaultBlendColor; }

        /**
         * 设置默认的混合颜色
         */
        void SetDefaultBlendColor(std::array<ColorRGBA32, 4> color) noexcept;

        /**
         * 获取预先计算的顶点
         * 这里的顶点会预先填充好偏移、纹理坐标、颜色等数据，用于加速渲染。
         */
        const PrecomputedSequenceContainer& GetPrecomputedSequenceVertex() const noexcept { return m_stPrecomputedSequenceVertex; }

    protected:  // Asset
        [[nodiscard]] Subsystem::Asset::AssetTypeId GetAssetTypeId() const noexcept override;

    private:
        void PrecomputedVertex(int what) noexcept;

    private:
        Subsystem::Asset::AssetPtr m_pTextureAsset;
        SequenceContainer m_stFrames;  // 帧，精灵在纹理中的范围
        Vec2 m_stAnchor;  // 锚点，相对于 Frame 左上角
        ColliderShape m_stColliderShape;  // 碰撞外形
        BlendMode m_stDefaultBlendMode;  // 默认混合模式
        std::array<ColorRGBA32, 4> m_stDefaultBlendColor = { 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu };  // 默认混合颜色
        PrecomputedSequenceContainer m_stPrecomputedSequenceVertex;  // 根据参数预先计算的顶点
    };
}
