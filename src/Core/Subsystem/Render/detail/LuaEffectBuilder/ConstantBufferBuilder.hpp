/**
 * @file
 * @date 2022/3/14
 * @author 9chu
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#pragma once
#include "Helper.hpp"
#include <lstg/Core/Subsystem/Render/GraphDef/ConstantBufferDefinition.hpp>
#include <lstg/Core/Subsystem/Render/GraphDef/ConstantBufferValueType.hpp>

namespace lstg::Subsystem::Render::detail::LuaEffectBuilder
{
    /**
     * ConstantBuffer 包装器
     */
    LSTG_CLASS()
    class ConstantBufferWrapper :
        public ScriptObjectWrapper<GraphDef::ConstantBufferDefinition>
    {
    public:
        ConstantBufferWrapper(std::shared_ptr<const GraphDef::ConstantBufferDefinition> def, bool global)
            : ScriptObjectWrapper(std::move(def)), m_bGlobal(global)
        {}

    public:
        [[nodiscard]] bool IsGlobal() const noexcept { return m_bGlobal; }

    private:
        bool m_bGlobal = false;
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
        Script::LuaStack::AbsIndex DefineScalarField(Script::LuaStack& stack, const char* name,
            GraphDef::ConstantBufferValueType::ScalarTypes type);

        LSTG_METHOD(vector)
        Script::LuaStack::AbsIndex DefineVectorField(Script::LuaStack& stack, const char* name,
            GraphDef::ConstantBufferValueType::ScalarTypes type, unsigned dimensions);

        LSTG_METHOD(matrix)
        Script::LuaStack::AbsIndex DefineMatrixField(Script::LuaStack& stack, const char* name,
            GraphDef::ConstantBufferValueType::ScalarTypes type, unsigned row, unsigned column);

        LSTG_METHOD(rowMajorMatrix)
        Script::LuaStack::AbsIndex DefineRowMajorMatrixField(Script::LuaStack& stack, const char* name,
            GraphDef::ConstantBufferValueType::ScalarTypes type, unsigned row, unsigned column);

        LSTG_METHOD(build)
        ConstantBufferWrapper Build(Script::LuaStack& stack);

    private:
        GraphDef::ConstantBufferDefinition m_stDefinition;
    };
}
