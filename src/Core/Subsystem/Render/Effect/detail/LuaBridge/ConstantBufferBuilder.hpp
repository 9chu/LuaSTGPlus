/**
 * @file
 * @date 2022/3/14
 * @author 9chu
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#pragma once
#include <lstg/Core/Subsystem/Script/LuaState.hpp>
#include <lstg/Core/Subsystem/Script/AutoBridgeHint.hpp>
#include <lstg/Core/Subsystem/Render/Effect/ConstantBufferDefinition.hpp>
#include <lstg/Core/Subsystem/Render/Effect/ShaderValueType.hpp>

namespace lstg::Subsystem::Render::Effect::detail::LuaBridge
{
    /**
     * ConstantBuffer 包装类
     */
    LSTG_CLASS()
    class ConstantBufferWrapper
    {
    public:
        ConstantBufferWrapper(ImmutableConstantBufferDefinitionPtr def)
            : m_pDefinition(std::move(def))
        {}

    public:
        [[nodiscard]] const auto& Get() const noexcept { return m_pDefinition; }

    private:
        ImmutableConstantBufferDefinitionPtr m_pDefinition;
    };

    /**
     * ConstantBuffer 构造器
     */
    LSTG_CLASS()
    class ConstantBufferBuilder
    {
    public:
        ConstantBufferBuilder(const char* name);

    public:
        LSTG_METHOD(scalar)
        Script::LuaStack::AbsIndex DefineScalarField(Script::LuaStack& stack, const char* name, ShaderValueType::ScalarTypes type);

        LSTG_METHOD(vector)
        Script::LuaStack::AbsIndex DefineVectorField(Script::LuaStack& stack, const char* name, ShaderValueType::ScalarTypes type,
            unsigned dimensions);

        LSTG_METHOD(matrix)
        Script::LuaStack::AbsIndex DefineMatrixField(Script::LuaStack& stack, const char* name, ShaderValueType::ScalarTypes type,
            unsigned row, unsigned column);

        LSTG_METHOD(rowMajorMatrix)
        Script::LuaStack::AbsIndex DefineRowMajorMatrixField(Script::LuaStack& stack, const char* name, ShaderValueType::ScalarTypes type,
            unsigned row, unsigned column);

        LSTG_METHOD(build)
        ConstantBufferWrapper Build(Script::LuaStack& stack);

    private:
        ConstantBufferDefinition m_stDefinition;
    };
}
