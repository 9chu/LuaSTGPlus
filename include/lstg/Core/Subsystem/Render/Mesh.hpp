/**
 * @file
 * @date 2022/3/19
 * @author 9chu
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#pragma once
#include "RenderDevice.hpp"
#include "GraphDef/MeshDefinition.hpp"
#include "../../Flag.hpp"
#include "../../Span.hpp"

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
        enum class Usage
        {
            Static,  ///< @brief 静态
            Dynamic,  ///< @brief 动态，每帧都会失效，使用前需要 Commit
        };

    public:
        Mesh(RenderDevice& device, GraphDef::ImmutableMeshDefinitionPtr definition, Diligent::IBuffer* vertexBuffer,
            Diligent::IBuffer* indexBuffer, bool use32BitsIndex, Usage usage);
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
         * 获取用途
         */
        [[nodiscard]] Usage GetUsage() const noexcept { return m_iUsage; }

        /**
         * 获取顶点个数
         */
        [[nodiscard]] size_t GetVertexCount() const noexcept;

        /**
         * 获取索引个数
         */
        [[nodiscard]] size_t GetIndexCount() const noexcept;

        /**
         * 提交数据
         * @param vertexData 顶点数据
         * @param indexData 索引数据
         * @return 结果
         */
        Result<void> Commit(Span<const uint8_t> vertexData, Span<const uint8_t> indexData) noexcept;

    private:
        RenderDevice& m_stDevice;
        GraphDef::ImmutableMeshDefinitionPtr m_pDefinition;
        bool m_bUse32BitsIndex = false;
        Usage m_iUsage = Usage::Static;
        Diligent::IBuffer* m_pVertexBuffer = nullptr;
        Diligent::IBuffer* m_pIndexBuffer = nullptr;
    };

    using MeshPtr = std::shared_ptr<Mesh>;
}
