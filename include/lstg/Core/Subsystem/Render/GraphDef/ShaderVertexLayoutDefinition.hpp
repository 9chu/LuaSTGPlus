/**
 * @file
 * @date 2022/3/13
 * @author 9chu
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#pragma once
#include <cstdint>
#include <tuple>
#include <memory>
#include <map>
#include "../../../Result.hpp"

namespace lstg::Subsystem::Render::GraphDef
{
    /**
     * 顶点布局定义
     */
    class ShaderVertexLayoutDefinition
    {
    public:
        /**
         * 元素语义
         * @see https://docs.microsoft.com/en-us/windows/win32/direct3dhlsl/dx-graphics-hlsl-semantics
         */
        enum class ElementSemanticNames: uint8_t
        {
            Binormal,
            BlendIndices,
            BlendWeights,
            Color,
            Normal,
            Position,
            TransformedPosition,
            PointSize,
            Tangent,
            TextureCoord,
            Custom,
        };

        using ElementSemantic = std::tuple<ElementSemanticNames, uint8_t /* Index */>;

        struct VertexElementDesc
        {
            uint32_t SlotIndex;
            ElementSemantic Semantic;
            bool Normalized;

            bool operator==(const VertexElementDesc& rhs) const noexcept
            {
                return SlotIndex == rhs.SlotIndex && Semantic == rhs.Semantic && Normalized == rhs.Normalized;
            }
        };

    public:
        ShaderVertexLayoutDefinition() = default;

        bool operator==(const ShaderVertexLayoutDefinition& rhs) const noexcept;

    public:
        /**
         * 定义 Slot
         * @pre !ContainsSlot(index)
         * @param index Slot 索引
         * @param semantic 语义
         * @param normalized 是否规格化
         */
        Result<void> AddSlot(uint32_t index, ElementSemantic semantic, bool normalized) noexcept;

        /**
         * 检查是否存在 Slot
         */
        [[nodiscard]] bool ContainsSlot(uint32_t index) const noexcept;

        /**
         * 获取所有 Slot
         */
        [[nodiscard]] const auto& GetSlots() const noexcept { return m_stSlots; }

        /**
         * 计算 Hash 值
         */
        [[nodiscard]] size_t GetHashCode() const noexcept;

    private:
        std::map<uint32_t, VertexElementDesc> m_stSlots;
    };

    using ShaderVertexLayoutDefinitionPtr = std::shared_ptr<ShaderVertexLayoutDefinition>;
    using ImmutableShaderVertexLayoutDefinitionPtr = std::shared_ptr<const ShaderVertexLayoutDefinition>;
}

namespace std
{
    template <>
    struct hash<lstg::Subsystem::Render::GraphDef::ShaderVertexLayoutDefinition>
    {
        std::size_t operator()(const lstg::Subsystem::Render::GraphDef::ShaderVertexLayoutDefinition& value) const noexcept
        {
            return value.GetHashCode();
        }
    };
}
