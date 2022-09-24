/**
 * @file
 * @date 2022/3/19
 * @author 9chu
 * 此文件为 LuaSTGPlus 项目的一部分，版权与许可声明详见 COPYRIGHT.txt。
 */
#pragma once
#include <vector>
#include "ShaderVertexLayoutDefinition.hpp"

namespace lstg::Subsystem::Render::GraphDef
{
    /**
     * 网格定义
     */
    class MeshDefinition
    {
    public:
        /**
         * 图元类型
         */
        enum class PrimitiveTopologyTypes
        {
            TriangleList,
            TriangleStrip,
        };

        /**
         * 字段类型
         */
        enum class VertexElementScalarTypes: uint8_t
        {
            Int8,
            Int16,
            Int32,
            UInt8,
            UInt16,
            UInt32,
            Half,
            Float,
        };

        /**
         * 字段元素个数
         */
        enum class VertexElementComponents: uint8_t
        {
            One = 1,
            Two = 2,
            Three = 3,
            Four = 4,
        };

        /**
         * 顶点元素类型
         */
        using VertexElementType = std::tuple<VertexElementScalarTypes, VertexElementComponents>;

        /**
         * 语义名称
         */
        using VertexElementSemanticNames = ShaderVertexLayoutDefinition::ElementSemanticNames;

        /**
         * 顶点元素语义
         */
        using VertexElementSemantic = ShaderVertexLayoutDefinition::ElementSemantic;

        /**
         * 顶点元素
         */
        struct VertexElement
        {
            VertexElementType Type;
            VertexElementSemantic Semantic;  // 用于和 VertexShader 进行关联
            size_t Offset = 0;  // 相对于当前顶点的偏移

            bool operator==(const VertexElement& rhs) const noexcept
            {
                return Type == rhs.Type &&
                    Semantic == rhs.Semantic &&
                    Offset == rhs.Offset;
            }
        };

    public:
        MeshDefinition() = default;

        bool operator==(const MeshDefinition& def) const noexcept;

    public:
        /**
         * 获取图元类型
         */
        [[nodiscard]] PrimitiveTopologyTypes GetPrimitiveTopologyType() const noexcept { return m_iTopologyType; }

        /**
         * 设置图元类型
         */
        void SetPrimitiveTopologyType(PrimitiveTopologyTypes t) noexcept { m_iTopologyType = t; }

        /**
         * 获取单个顶点的大小
         */
        [[nodiscard]] size_t GetVertexStride() const noexcept { return m_iVertexStride; }

        /**
         * 设置单个顶点的大小
         * @param stride 大小
         */
        void SetVertexStride(size_t stride) noexcept { m_iVertexStride = stride; }

        /**
         * 增加顶点元素定义
         * @pre !ContainsSemantic(semantic)
         * @param type 类型
         * @param semantic 语义
         * @param offset 偏移
         * @return 是否成功
         */
        Result<void> AddVertexElement(VertexElementType type, VertexElementSemantic semantic, size_t offset) noexcept;

        /**
         * 是否包含语义
         * @param semantic 语义
         */
        [[nodiscard]] bool ContainsSemantic(VertexElementSemantic semantic) const noexcept;

        /**
         * 获取顶点元素列表
         */
        [[nodiscard]] const auto& GetVertexElements() const noexcept { return m_stVertexElements; }

        /**
         * 获取哈希值
         */
        [[nodiscard]] size_t GetHashCode() const noexcept;

    private:
        PrimitiveTopologyTypes m_iTopologyType = PrimitiveTopologyTypes::TriangleList;
        size_t m_iVertexStride = 0;
        std::vector<VertexElement> m_stVertexElements;
    };

    using MeshDefinitionPtr = std::shared_ptr<MeshDefinition>;
    using ImmutableMeshDefinitionPtr = std::shared_ptr<const MeshDefinition>;
}

namespace std
{
    template <>
    struct hash<lstg::Subsystem::Render::GraphDef::MeshDefinition>
    {
        std::size_t operator()(const lstg::Subsystem::Render::GraphDef::MeshDefinition& value) const noexcept
        {
            return value.GetHashCode();
        }
    };
}
