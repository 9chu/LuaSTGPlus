/**
 * @file
 * @date 2022/3/15
 * @author 9chu
 * 此文件为 LuaSTGPlus 项目的一部分，版权与许可声明详见 COPYRIGHT.txt。
 */
#pragma once
#include "EffectBuilder.hpp"

namespace lstg::Subsystem::Render::detail::LuaEffectBuilder
{
    LSTG_MODULE(Builder, GLOBAL)
    class BuilderModule
    {
    public:  // 枚举
#if LSTG_AUTO_BRIDGE_HINT
        LSTG_ENUM()
        enum class ScalarTypes
        {
            LSTG_FIELD(BOOL)
            Bool,
            LSTG_FIELD(INT)
            Int,
            LSTG_FIELD(UINT)
            UInt,
            LSTG_FIELD(FLOAT)
            Float,
            LSTG_FIELD(DOUBLE)
            Double,
        };
        LSTG_ENUM()
        enum class FilterTypes
        {
            LSTG_FIELD(POINT)
            Point,
            LSTG_FIELD(LINEAR)
            Linear,
            LSTG_FIELD(ANISOTROPIC)
            Anisotropic,
        };
        LSTG_ENUM()
        enum class TextureAddressModes
        {
            LSTG_FIELD(WRAP)
            Wrap,
            LSTG_FIELD(MIRROR)
            Mirror,
            LSTG_FIELD(CLAMP)
            Clamp,
            LSTG_FIELD(BORDER)
            Border,
        };
        LSTG_ENUM()
        enum class SemanticNames
        {
            LSTG_FIELD(BINORMAL)
            Binormal,
            LSTG_FIELD(BLEND_INDICES)
            BlendIndices,
            LSTG_FIELD(BLEND_WEIGHTS)
            BlendWeights,
            LSTG_FIELD(COLOR)
            Color,
            LSTG_FIELD(NORMAL)
            Normal,
            LSTG_FIELD(POSITION)
            Position,
            LSTG_FIELD(TRANSFORMED_POSITION)
            TransformedPosition,
            LSTG_FIELD(POINT_SIZE)
            PointSize,
            LSTG_FIELD(TANGENT)
            Tangent,
            LSTG_FIELD(TEXCOORD)
            TextureCoord,
            LSTG_FIELD(CUSTOM)
            Custom,
        };
        LSTG_ENUM()
        enum class BlendStates
        {
            LSTG_FIELD(ENABLE)
            Enable,
            LSTG_FIELD(SOURCE_BLEND)
            SourceBlend,
            LSTG_FIELD(DEST_BLEND)
            DestBlend,
            LSTG_FIELD(BLEND_OPERATION)
            BlendOperation,
            LSTG_FIELD(SOURCE_ALPHA_BLEND)
            SourceAlphaBlend,
            LSTG_FIELD(DEST_ALPHA_BLEND)
            DestAlphaBlend,
            LSTG_FIELD(ALPHA_BLEND_OPERATION)
            AlphaBlendOperation,
            LSTG_FIELD(WRITE_MASK)
            WriteMask,
        };
        LSTG_ENUM()
        enum class RasterizerStates
        {
            LSTG_FIELD(FILL_MODE)
            FillMode,
            LSTG_FIELD(CULL_MODE)
            CullMode,
        };
        LSTG_ENUM()
        enum class DepthStencilStates
        {
            LSTG_FIELD(DEPTH_ENABLE)
            DepthEnable,
            LSTG_FIELD(DEPTH_WRITE_ENABLE)
            DepthWriteEnable,
            LSTG_FIELD(DEPTH_FUNCTION)
            DepthFunction,
        };
        LSTG_ENUM()
        enum class BlendFactors
        {
            LSTG_FIELD(ZERO)
            Zero,
            LSTG_FIELD(ONE)
            One,
            LSTG_FIELD(SOURCE_COLOR)
            SourceColor,
            LSTG_FIELD(INVERT_SOURCE_COLOR)
            InvertSourceColor,
            LSTG_FIELD(SOURCE_ALPHA)
            SourceAlpha,
            LSTG_FIELD(INVERT_SOURCE_ALPHA)
            InvertSourceAlpha,
            LSTG_FIELD(DEST_ALPHA)
            DestAlpha,
            LSTG_FIELD(INVERT_DEST_ALPHA)
            InvertDestAlpha,
            LSTG_FIELD(DEST_COLOR)
            DestColor,
            LSTG_FIELD(INVERT_DEST_COLOR)
            InvertDestColor,
            LSTG_FIELD(SOURCE_ALPHA_SATURATE)
            SourceAlphaSaturate,
            LSTG_FIELD(SOURCE_1_COLOR)
            Source1Color,
            LSTG_FIELD(INVERT_SOURCE_1_COLOR)
            InvertSource1Color,
            LSTG_FIELD(SOURCE_1_ALPHA)
            Source1Alpha,
            LSTG_FIELD(INVERT_SOURCE_1_ALPHA)
            InvertSource1Alpha,
        };
        LSTG_ENUM()
        enum class BlendOperations
        {
            LSTG_FIELD(ADD)
            Add,
            LSTG_FIELD(SUBTRACT)
            Subtract,
            LSTG_FIELD(REVERT_SUBTRACT)
            RevertSubtract,
            LSTG_FIELD(MIN)
            Min,
            LSTG_FIELD(MAX)
            Max,
        };
        LSTG_ENUM()
        enum class ColorWriteMask
        {
            LSTG_FIELD(NONE)
            None = 0,
            LSTG_FIELD(RED)
            Red = 1,
            LSTG_FIELD(GREEN)
            Green = 2,
            LSTG_FIELD(BLUE)
            Blue = 4,
            LSTG_FIELD(ALPHA)
            Alpha = 8,
            LSTG_FIELD(ALL)
            All = (Red | Green | Blue | Alpha),
        };
        LSTG_ENUM()
        enum class FillModes
        {
            LSTG_FIELD(WIRE_FRAME)
            WireFrame,
            LSTG_FIELD(SOLID)
            Solid,
        };
        LSTG_ENUM()
        enum class CullModes
        {
            LSTG_FIELD(NONE)
            None,
            LSTG_FIELD(FRONT)
            Front,
            LSTG_FIELD(BACK)
            Back,
        };
        LSTG_ENUM()
        enum class ComparisionFunctions
        {
            LSTG_FIELD(NEVER)
            Never,
            LSTG_FIELD(LESS)
            Less,
            LSTG_FIELD(EQUAL)
            Equal,
            LSTG_FIELD(LESS_EQUAL)
            LessEqual,
            LSTG_FIELD(GREATER)
            Greater,
            LSTG_FIELD(NOT_EQUAL)
            NotEqual,
            LSTG_FIELD(GREATER_EQUAL)
            GreaterEqual,
            LSTG_FIELD(ALWAYS)
            Always,
        };
#else
        using ScalarTypes = GraphDef::ConstantBufferValueType::ScalarTypes;
        using FilterTypes = GraphDef::FilterTypes;
        using TextureAddressModes = GraphDef::TextureAddressModes;
        using SemanticNames = GraphDef::ShaderVertexLayoutDefinition::ElementSemanticNames;
        using BlendStates = EffectPassBuilder::BlendStates;
        using RasterizerStates = EffectPassBuilder::RasterizerStates;
        using DepthStencilStates = EffectPassBuilder::DepthStencilStates;
        using BlendFactors = GraphDef::BlendFactors;
        using BlendOperations = GraphDef::BlendOperations;
        using ColorWriteMask = GraphDef::ColorWriteMask;
        using FillModes = GraphDef::FillModes;
        using CullModes = GraphDef::CullModes;
        using ComparisionFunctions = GraphDef::ComparisionFunctions;
#endif

    public:  // 方法
        LSTG_METHOD(constantBuffer)
        static ConstantBufferBuilder DefineConstantBuffer(Script::LuaStack& stack, const char* name);

        LSTG_METHOD(importConstantBuffer)
        static ConstantBufferWrapper ImportGlobalConstantBuffer(Script::LuaStack& stack, const char* name);

        LSTG_METHOD(texture1d)
        static TextureVariableBuilder DefineTexture1D(Script::LuaStack& stack, const char* name);

        LSTG_METHOD(texture2d)
        static TextureVariableBuilder DefineTexture2D(Script::LuaStack& stack, const char* name);

        LSTG_METHOD(texture3d)
        static TextureVariableBuilder DefineTexture3D(Script::LuaStack& stack, const char* name);

        LSTG_METHOD(textureCube)
        static TextureVariableBuilder DefineTextureCube(Script::LuaStack& stack, const char* name);

        LSTG_METHOD(vertexLayout)
        static VertexLayoutBuilder DefineVertexLayout(Script::LuaStack& stack);

        LSTG_METHOD(vertexShader)
        static ShaderBuilder DefineVertexShader(Script::LuaStack& stack, const char* source);

        LSTG_METHOD(pixelShader)
        static ShaderBuilder DefinePixelShader(Script::LuaStack& stack, const char* source);

        LSTG_METHOD(pass)
        static EffectPassBuilder DefinePass(Script::LuaStack& stack, const char* name);

        LSTG_METHOD(passGroup)
        static EffectPassGroupBuilder DefinePassGroup(Script::LuaStack& stack, const char* name);

        LSTG_METHOD(effect)
        static EffectBuilder DefineEffect();
    };

    void InitModule(Script::LuaStack& stack);
}
