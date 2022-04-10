/**
 * @file
 * @date 2022/3/19
 * @author 9chu
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#pragma once
#include "GraphDef/MeshDefinition.hpp"
#include "../../Flag.hpp"

namespace Diligent
{
    struct IBuffer;
}

namespace lstg::Subsystem
{
    class RenderSystem;
}

namespace lstg::Subsystem::Render
{
    /**
     * 网格数据
     */
    class Mesh
    {
        friend class lstg::Subsystem::RenderSystem;

    public:
        Mesh(GraphDef::ImmutableMeshDefinitionPtr definition, Diligent::IBuffer* vertexBuffer, Diligent::IBuffer* indexBuffer,
            bool use32BitsIndex);
        ~Mesh();

    public:
        /**
         * 获取定义
         */
        [[nodiscard]] const GraphDef::ImmutableMeshDefinitionPtr& GetDefinition() const noexcept { return m_pDefinition; }

        /**
         * 是否启用 32 位索引
         */
        [[nodiscard]] bool Is32BitsIndex() const noexcept { return m_bUse32BitsIndex; }

        /**
         * 获取顶点个数
         */
        [[nodiscard]] size_t GetVertexCount() const noexcept;

        /**
         * 获取索引个数
         */
        [[nodiscard]] size_t GetIndexCount() const noexcept;

    private:
        GraphDef::ImmutableMeshDefinitionPtr m_pDefinition;
        bool m_bUse32BitsIndex = false;
        Diligent::IBuffer* m_pVertexBuffer = nullptr;
        Diligent::IBuffer* m_pIndexBuffer = nullptr;
    };

    using MeshPtr = std::shared_ptr<Mesh>;
}
