/**
 * @file
 * @date 2022/3/14
 * @author 9chu
 * 此文件为 LuaSTGPlus 项目的一部分，版权与许可声明详见 COPYRIGHT.txt。
 */
#include "ConstantBufferBuilder.hpp"

using namespace std;
using namespace lstg;
using namespace lstg::Subsystem::Render::detail::LuaEffectBuilder;

using AbsIndex = Subsystem::Script::LuaStack::AbsIndex;

ConstantBufferBuilder::ConstantBufferBuilder(const char* name)
    : m_stDefinition(name)
{
}

AbsIndex ConstantBufferBuilder::DefineScalarField(Script::LuaStack& stack, const char* name,
    GraphDef::ConstantBufferValueType::ScalarTypes type)
{
    auto ret = m_stDefinition.DefineField(name, GraphDef::ConstantBufferValueType { type });
    stack.ThrowIfError(ret);
    LSTG_SCRIPT_RETURN_SELF;
}

AbsIndex ConstantBufferBuilder::DefineVectorField(Script::LuaStack& stack, const char* name,
    GraphDef::ConstantBufferValueType::ScalarTypes type, unsigned dimensions)
{
    if (dimensions < 1 || dimensions > 4)
    {
        stack.Error("vector should contains 1-4 components, %d is invalid", dimensions);
        LSTG_SCRIPT_RETURN_SELF;
    }
    auto ret = m_stDefinition.DefineField(name, GraphDef::ConstantBufferValueType{ type, dimensions });
    stack.ThrowIfError(ret);
    LSTG_SCRIPT_RETURN_SELF;
}

AbsIndex ConstantBufferBuilder::DefineMatrixField(Script::LuaStack& stack, const char* name,
    GraphDef::ConstantBufferValueType::ScalarTypes type, unsigned row, unsigned column)
{
    if (row < 1 || row > 4)
    {
        stack.Error("Row number of the matrix should be in range 1-4, %d is invalid", row);
        LSTG_SCRIPT_RETURN_SELF;
    }
    if (column < 1 || column > 4)
    {
        stack.Error("Column number of the matrix should be in range 1-4, %d is invalid", column);
        LSTG_SCRIPT_RETURN_SELF;
    }
    auto ret = m_stDefinition.DefineField(name, GraphDef::ConstantBufferValueType{ type, row, column, false });
    stack.ThrowIfError(ret);
    LSTG_SCRIPT_RETURN_SELF;
}

AbsIndex ConstantBufferBuilder::DefineRowMajorMatrixField(Script::LuaStack& stack, const char* name,
    GraphDef::ConstantBufferValueType::ScalarTypes type, unsigned row, unsigned column)
{
    if (row < 1 || row > 4)
    {
        stack.Error("Row number of the matrix should be in range 1-4, %d is invalid", row);
        LSTG_SCRIPT_RETURN_SELF;
    }
    if (column < 1 || column > 4)
    {
        stack.Error("Column number of the matrix should be in range 1-4, %d is invalid", column);
        LSTG_SCRIPT_RETURN_SELF;
    }
    auto ret = m_stDefinition.DefineField(name, GraphDef::ConstantBufferValueType{ type, row, column, true });
    stack.ThrowIfError(ret);
    LSTG_SCRIPT_RETURN_SELF;
}

ConstantBufferWrapper ConstantBufferBuilder::Build(Script::LuaStack& stack)
{
    auto state = BuilderGlobalState::FromLuaStack(stack);

    // 如果没有符号，拒绝
    if (m_stDefinition.GetFields().empty())
        stack.Error("Empty constant buffer is not allowed");

    // 检查 Symbol 定义情况
    if (state->ContainsSymbol(m_stDefinition.GetName()))
        stack.Error("Symbol \"%s\" already defined in script", m_stDefinition.GetName().c_str());
    for (const auto& e : m_stDefinition.GetFields())
    {
        if (state->ContainsSymbol(e.Name))
            stack.Error("Symbol \"%s\" already defined in script", e.Name.c_str());
    }

    // 定义 Symbol
    state->SymbolCache.emplace(m_stDefinition.GetName());
    for (const auto& e : m_stDefinition.GetFields())
        state->SymbolCache.emplace(e.Name);

    return { make_shared<const GraphDef::ConstantBufferDefinition>(m_stDefinition), false };
}
