/**
 * @file
 * @date 2022/3/14
 * @author 9chu
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#pragma once
#include "../../../Flag.hpp"
#include "ShaderDefinition.hpp"

namespace lstg::Subsystem::Render::Effect
{
    /**
     * 混合元素
     */
    enum class BlendFactors
    {
        Zero,
        One,
        SourceColor,
        InvertSourceColor,
        SourceAlpha,
        InvertSourceAlpha,
        DestAlpha,
        InvertDestAlpha,
        DestColor,
        InvertDestColor,
        SourceAlphaSaturate,
        // BlendFactor,
        // InvertBlendFactor,
        Source1Color,
        InvertSource1Color,
        Source1Alpha,
        InvertSource1Alpha,
    };

    /**
     * 混合操作符
     */
    enum class BlendOperations
    {
        Add,
        Subtract,
        RevertSubtract,
        Min,
        Max,
    };

    /**
     * 颜色掩码
     */
    LSTG_FLAG_BEGIN(ColorWriteMask)
        None = 0,
        Red = 1,
        Green = 2,
        Blue = 4,
        Alpha = 8,
        All = (Red | Green | Blue | Alpha),
    LSTG_FLAG_END(ColorWriteMask)

    /**
     * 填充模式
     */
    enum class FillModes
    {
        WireFrame,
        Solid,
    };

    /**
     * 三角形裁切模式
     */
    enum class CullModes
    {
        None,
        Front,
        Back,
    };

    /**
     * 比较函数
     */
    enum class ComparisionFunctions
    {
        Never,
        Less,
        Equal,
        LessEqual,
        Greater,
        NotEqual,
        GreaterEqual,
        Always,
    };

    /**
     * 渲染过程
     */
    class EffectPassDefinition
    {
    public:
        /**
         * 混合模式描述
         */
        struct BlendStateDesc
        {
            bool Enable = false;
            BlendFactors SourceBlend = BlendFactors::One;
            BlendFactors DestBlend = BlendFactors::Zero;
            BlendOperations BlendOperation = BlendOperations::Add;
            BlendFactors SourceAlphaBlend = BlendFactors::One;
            BlendFactors DestAlphaBlend = BlendFactors::Zero;
            BlendOperations AlphaBlendOperation = BlendOperations::Add;
            ColorWriteMask WriteMask = ColorWriteMask::All;

            bool operator==(const BlendStateDesc& rhs) const noexcept;

            /**
             * 计算哈希值
             */
            [[nodiscard]] size_t GetHashCode() const noexcept;
        };

        /**
         * 光栅化描述
         */
        struct RasterizerStateDesc
        {
            FillModes FillMode = FillModes::Solid;
            CullModes CullMode = CullModes::Back;

            bool operator==(const RasterizerStateDesc& rhs) const noexcept;

            /**
             * 计算哈希值
             */
            [[nodiscard]] size_t GetHashCode() const noexcept;
        };

        /**
         * 深度模板缓冲区描述
         */
        struct DepthStencilStateDesc
        {
            bool DepthEnable = true;
            bool DepthWriteEnable = true;
            ComparisionFunctions DepthFunction = ComparisionFunctions::Less;

            bool operator==(const DepthStencilStateDesc& rhs) const noexcept;

            /**
             * 计算哈希值
             */
            [[nodiscard]] size_t GetHashCode() const noexcept;
        };

    public:
        EffectPassDefinition() = default;

        bool operator==(const EffectPassDefinition& rhs) const noexcept;

    public:
        /**
         * 获取 Pass 名称
         */
        [[nodiscard]] const std::string& GetName() const noexcept { return m_stName; }

        /**
         * 设置 Pass 名称
         */
        void SetName(std::string name) noexcept { m_stName = std::move(name); }

        /**
         * 获取混合状态
         */
        [[nodiscard]] const BlendStateDesc& GetBlendState() const noexcept { return m_stBlendState; }
        [[nodiscard]] BlendStateDesc& GetBlendState() noexcept { return m_stBlendState; }

        /**
         * 获取光栅化状态
         */
        [[nodiscard]] const RasterizerStateDesc& GetRasterizerState() const noexcept { return m_stRasterizerState; }
        [[nodiscard]] RasterizerStateDesc& GetRasterizerState() noexcept { return m_stRasterizerState; }

        /**
         * 获取深度模板缓冲状态
         */
        [[nodiscard]] const DepthStencilStateDesc& GetDepthStencilState() const noexcept { return m_stDepthStencilState; }
        [[nodiscard]] DepthStencilStateDesc& GetDepthStencilState() noexcept { return m_stDepthStencilState; }

        /**
         * 获取顶点着色器
         */
        [[nodiscard]] const ImmutableShaderDefinitionPtr& GetVertexShader() const noexcept { return m_pVertexShader; }
        void SetVertexShader(ImmutableShaderDefinitionPtr ptr) noexcept { m_pVertexShader = std::move(ptr); }

        /**
         * 获取像素着色器
         */
        [[nodiscard]] const ImmutableShaderDefinitionPtr& GetPixelShader() const noexcept { return m_pPixelShader; }
        void SetPixelShader(ImmutableShaderDefinitionPtr ptr) noexcept { m_pPixelShader = std::move(ptr); }

        /**
         * 获取哈希值
         */
        [[nodiscard]] size_t GetHashCode() const noexcept;

    private:
        std::string m_stName;
        BlendStateDesc m_stBlendState;
        RasterizerStateDesc m_stRasterizerState;
        DepthStencilStateDesc m_stDepthStencilState;
        ImmutableShaderDefinitionPtr m_pVertexShader;
        ImmutableShaderDefinitionPtr m_pPixelShader;
    };

    using EffectPassDefinitionPtr = std::shared_ptr<EffectPassDefinition>;
    using ImmutableEffectPassDefinitionPtr = std::shared_ptr<const EffectPassDefinition>;
}

namespace std
{
    template <>
    struct hash<lstg::Subsystem::Render::Effect::EffectPassDefinition>
    {
        std::size_t operator()(const lstg::Subsystem::Render::Effect::EffectPassDefinition& value) const
        {
            return value.GetHashCode();
        }
    };
}
