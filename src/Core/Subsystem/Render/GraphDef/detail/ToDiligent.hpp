/**
 * @file
 * @date 2022/4/5
 * @author 9chu
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#pragma once
#include <lstg/Core/Subsystem/Render/GraphDef/EffectDefinition.hpp>
#include <lstg/Core/Subsystem/Render/GraphDef/MeshDefinition.hpp>

#include <PipelineState.h>
#include <GraphicsTypes.h>
#include <Sampler.h>

namespace lstg::Subsystem::Render::GraphDef::detail
{
    // <editor-fold desc="EffectPassDefinition ToDiligent">

    inline Diligent::BLEND_FACTOR ToDiligent(GraphDef::BlendFactors f) noexcept
    {
        switch (f)
        {
            case BlendFactors::Zero:
                return Diligent::BLEND_FACTOR_ZERO;
            case BlendFactors::One:
                return Diligent::BLEND_FACTOR_ONE;
            case BlendFactors::SourceColor:
                return Diligent::BLEND_FACTOR_SRC_COLOR;
            case BlendFactors::InvertSourceColor:
                return Diligent::BLEND_FACTOR_INV_SRC_COLOR;
            case BlendFactors::SourceAlpha:
                return Diligent::BLEND_FACTOR_SRC_ALPHA;
            case BlendFactors::InvertSourceAlpha:
                return Diligent::BLEND_FACTOR_INV_SRC_ALPHA;
            case BlendFactors::DestAlpha:
                return Diligent::BLEND_FACTOR_DEST_ALPHA;
            case BlendFactors::InvertDestAlpha:
                return Diligent::BLEND_FACTOR_INV_DEST_ALPHA;
            case BlendFactors::DestColor:
                return Diligent::BLEND_FACTOR_DEST_COLOR;
            case BlendFactors::InvertDestColor:
                return Diligent::BLEND_FACTOR_INV_DEST_COLOR;
            case BlendFactors::SourceAlphaSaturate:
                return Diligent::BLEND_FACTOR_SRC_ALPHA_SAT;
            case BlendFactors::Source1Color:
                return Diligent::BLEND_FACTOR_SRC1_COLOR;
            case BlendFactors::InvertSource1Color:
                return Diligent::BLEND_FACTOR_INV_SRC1_COLOR;
            case BlendFactors::Source1Alpha:
                return Diligent::BLEND_FACTOR_SRC1_ALPHA;
            case BlendFactors::InvertSource1Alpha:
                return Diligent::BLEND_FACTOR_INV_SRC1_ALPHA;
            default:
                assert(false);
                return Diligent::BLEND_FACTOR_ONE;
        }
    }

    inline Diligent::BLEND_OPERATION ToDiligent(BlendOperations o) noexcept
    {
        switch (o)
        {
            case BlendOperations::Add:
                return Diligent::BLEND_OPERATION_ADD;
            case BlendOperations::Subtract:
                return Diligent::BLEND_OPERATION_SUBTRACT;
            case BlendOperations::RevertSubtract:
                return Diligent::BLEND_OPERATION_REV_SUBTRACT;
            case BlendOperations::Min:
                return Diligent::BLEND_OPERATION_MIN;
            case BlendOperations::Max:
                return Diligent::BLEND_OPERATION_MAX;
            default:
                assert(false);
                return Diligent::BLEND_OPERATION_ADD;
        }
    }

    inline Diligent::COLOR_MASK ToDiligent(ColorWriteMask m) noexcept
    {
        if (m == ColorWriteMask::All)
            return Diligent::COLOR_MASK_ALL;

        int r = Diligent::COLOR_MASK_NONE;
        if (m & ColorWriteMask::Red)
            r |= Diligent::COLOR_MASK_RED;
        if (m & ColorWriteMask::Green)
            r |= Diligent::COLOR_MASK_GREEN;
        if (m & ColorWriteMask::Blue)
            r |= Diligent::COLOR_MASK_BLUE;
        if (m & ColorWriteMask::Alpha)
            r |= Diligent::COLOR_MASK_ALPHA;
        return static_cast<Diligent::COLOR_MASK>(r);
    }

    inline Diligent::FILL_MODE ToDiligent(FillModes m) noexcept
    {
        switch (m)
        {
            case FillModes::WireFrame:
                return Diligent::FILL_MODE_WIREFRAME;
            case FillModes::Solid:
                return Diligent::FILL_MODE_SOLID;
            default:
                assert(false);
                return Diligent::FILL_MODE_SOLID;
        }
    }

    inline Diligent::CULL_MODE ToDiligent(CullModes m) noexcept
    {
        switch (m)
        {
            case CullModes::None:
                return Diligent::CULL_MODE_NONE;
            case CullModes::Front:
                return Diligent::CULL_MODE_FRONT;
            case CullModes::Back:
                return Diligent::CULL_MODE_BACK;
            default:
                assert(false);
                return Diligent::CULL_MODE_BACK;
        }
    }

    inline Diligent::COMPARISON_FUNCTION ToDiligent(ComparisionFunctions f) noexcept
    {
        switch (f)
        {
            case ComparisionFunctions::Never:
                return Diligent::COMPARISON_FUNC_NEVER;
            case ComparisionFunctions::Less:
                return Diligent::COMPARISON_FUNC_LESS;
            case ComparisionFunctions::Equal:
                return Diligent::COMPARISON_FUNC_EQUAL;
            case ComparisionFunctions::LessEqual:
                return Diligent::COMPARISON_FUNC_LESS_EQUAL;
            case ComparisionFunctions::Greater:
                return Diligent::COMPARISON_FUNC_GREATER;
            case ComparisionFunctions::NotEqual:
                return Diligent::COMPARISON_FUNC_NOT_EQUAL;
            case ComparisionFunctions::GreaterEqual:
                return Diligent::COMPARISON_FUNC_GREATER_EQUAL;
            case ComparisionFunctions::Always:
                return Diligent::COMPARISON_FUNC_ALWAYS;
            default:
                assert(false);
                return Diligent::COMPARISON_FUNC_LESS;
        }
    }

    inline Diligent::RenderTargetBlendDesc ToDiligent(const EffectPassDefinition::BlendStateDesc& desc) noexcept
    {
        Diligent::RenderTargetBlendDesc ret;
        ret.BlendEnable = desc.Enable;
        ret.SrcBlend = ToDiligent(desc.SourceBlend);
        ret.DestBlend = ToDiligent(desc.DestBlend);
        ret.BlendOp = ToDiligent(desc.BlendOperation);
        ret.SrcBlendAlpha = ToDiligent(desc.SourceAlphaBlend);
        ret.DestBlendAlpha = ToDiligent(desc.DestAlphaBlend);
        ret.BlendOpAlpha = ToDiligent(desc.AlphaBlendOperation);
        ret.RenderTargetWriteMask = ToDiligent(desc.WriteMask);
        return ret;
    }

    inline Diligent::RasterizerStateDesc ToDiligent(const EffectPassDefinition::RasterizerStateDesc& desc) noexcept
    {
        Diligent::RasterizerStateDesc ret;
        ret.FillMode = ToDiligent(desc.FillMode);
        ret.CullMode = ToDiligent(desc.CullMode);
        return ret;
    }

    inline Diligent::DepthStencilStateDesc ToDiligent(const EffectPassDefinition::DepthStencilStateDesc& desc) noexcept
    {
        Diligent::DepthStencilStateDesc ret;
        ret.DepthEnable = desc.DepthEnable;
        ret.DepthWriteEnable = desc.DepthWriteEnable;
        ret.DepthFunc = ToDiligent(desc.DepthFunction);
        return ret;
    }

    // </editor-fold>
    // <editor-fold desc="ShaderTextureDefinition ToDiligent">

    inline Diligent::FILTER_TYPE ToDiligent(const FilterTypes t) noexcept
    {
        switch (t)
        {
            case FilterTypes::Point:
                return Diligent::FILTER_TYPE_POINT;
            case FilterTypes::Linear:
                return Diligent::FILTER_TYPE_LINEAR;
            case FilterTypes::Anisotropic:
                return Diligent::FILTER_TYPE_ANISOTROPIC;
            default:
                assert(false);
                return Diligent::FILTER_TYPE_LINEAR;
        }
    }

    inline Diligent::TEXTURE_ADDRESS_MODE ToDiligent(const TextureAddressModes m) noexcept
    {
        switch (m)
        {
            case TextureAddressModes::Wrap:
                return Diligent::TEXTURE_ADDRESS_WRAP;
            case TextureAddressModes::Mirror:
                return Diligent::TEXTURE_ADDRESS_MIRROR;
            case TextureAddressModes::Clamp:
                return Diligent::TEXTURE_ADDRESS_CLAMP;
            case TextureAddressModes::Border:
                return Diligent::TEXTURE_ADDRESS_BORDER;
            default:
                assert(false);
                return Diligent::TEXTURE_ADDRESS_CLAMP;
        }
    }

    inline Diligent::SamplerDesc ToDiligent(const ShaderTextureDefinition::SamplerDesc& desc) noexcept
    {
        Diligent::SamplerDesc ret;
        ret.MinFilter = ToDiligent(desc.MinFilter);
        ret.MagFilter = ToDiligent(desc.MagFilter);
        ret.MipFilter = ToDiligent(desc.MipFilter);
        ret.AddressU = ToDiligent(desc.AddressU);
        ret.AddressV = ToDiligent(desc.AddressV);
        ret.AddressW = ToDiligent(desc.AddressW);
        ret.MaxAnisotropy = desc.MaxAnisotropy;
        ret.BorderColor[0] = desc.BorderColor[0];
        ret.BorderColor[1] = desc.BorderColor[1];
        ret.BorderColor[2] = desc.BorderColor[2];
        ret.BorderColor[3] = desc.BorderColor[3];
        return ret;
    }

    // </editor-fold>
    // <editor-fold desc="MeshDefinition ToDiligent">

    inline Diligent::PRIMITIVE_TOPOLOGY ToDilignet(MeshDefinition::PrimitiveTopologyTypes t) noexcept
    {
        switch (t)
        {
            case MeshDefinition::PrimitiveTopologyTypes::TriangleList:
                return Diligent::PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
            case MeshDefinition::PrimitiveTopologyTypes::TriangleStrip:
                return Diligent::PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
            default:
                assert(false);
                return Diligent::PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        }
    }

    inline Diligent::VALUE_TYPE ToDiligent(MeshDefinition::VertexElementScalarTypes t) noexcept
    {
        switch (t)
        {
            case MeshDefinition::VertexElementScalarTypes::Int8:
                return Diligent::VT_INT8;
            case MeshDefinition::VertexElementScalarTypes::Int16:
                return Diligent::VT_INT16;
            case MeshDefinition::VertexElementScalarTypes::Int32:
                return Diligent::VT_INT32;
            case MeshDefinition::VertexElementScalarTypes::UInt8:
                return Diligent::VT_UINT8;
            case MeshDefinition::VertexElementScalarTypes::UInt16:
                return Diligent::VT_UINT16;
            case MeshDefinition::VertexElementScalarTypes::UInt32:
                return Diligent::VT_UINT32;
            case MeshDefinition::VertexElementScalarTypes::Half:
                return Diligent::VT_FLOAT16;
            case MeshDefinition::VertexElementScalarTypes::Float:
                return Diligent::VT_FLOAT32;
            default:
                assert(false);
                return Diligent::VT_FLOAT32;
        }
    }

    // </editor-fold>
}
