/**
 * @file
 * @date 2022/3/14
 * @author 9chu
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#include "ConstantBufferBuilder.hpp"

#include <lstg/Core/Subsystem/Render/EffectFactory.hpp>
#include "BuilderModule.hpp"

using namespace std;
using namespace lstg;
using namespace lstg::Subsystem::Render::Effect::detail::LuaBridge;

using AbsIndex = Subsystem::Script::LuaStack::AbsIndex;

#define RETURN_SELF \
    return Script::LuaStack::AbsIndex {1u}

#define NAME_CHECK \
    do {                                                                                \
        if (!BuilderModule::IsValidIdentifier(name))                                    \
        {                                                                               \
            stack.Error("Field \"%s\" is not a valid identifier", name);                \
            RETURN_SELF;                                                                \
        }                                                                               \
        if (m_stDefinition.GetName() == name)                                           \
        {                                                                               \
            stack.Error("Field \"%s\" cannot have the name same as the cbuffer", name); \
            RETURN_SELF;                                                                \
        }                                                                               \
        if (m_stDefinition.ContainsField(name))                                         \
        {                                                                               \
            stack.Error("Field \"%s\" already defined", name);                          \
            RETURN_SELF;                                                                \
        }                                                                               \
    } while (false)

ConstantBufferBuilder::ConstantBufferBuilder(const char* name)
    : m_stDefinition(name, ConstantBufferDefinition::Scope::Local)
{
}

AbsIndex ConstantBufferBuilder::DefineScalarField(Script::LuaStack& stack, const char* name, ShaderValueType::ScalarTypes type)
{
    NAME_CHECK;
    m_stDefinition.DefineField(name, ShaderValueType{type});
    RETURN_SELF;
}

AbsIndex ConstantBufferBuilder::DefineVectorField(Script::LuaStack& stack, const char* name, ShaderValueType::ScalarTypes type,
    unsigned dimensions)
{
    NAME_CHECK;
    if (dimensions < 1 || dimensions > 4)
    {
        stack.Error("Vector should contains 1-4 components, %d is invalid", dimensions);
        RETURN_SELF;
    }
    m_stDefinition.DefineField(name, ShaderValueType{type, dimensions});
    RETURN_SELF;
}

AbsIndex ConstantBufferBuilder::DefineMatrixField(Script::LuaStack& stack, const char* name, ShaderValueType::ScalarTypes type,
    unsigned row, unsigned column)
{
    NAME_CHECK;
    if (row < 1 || row > 4)
    {
        stack.Error("Row number of the matrix should be in range 1-4, %d is invalid", row);
        RETURN_SELF;
    }
    if (column < 1 || column > 4)
    {
        stack.Error("Column number of the matrix should be in range 1-4, %d is invalid", column);
        RETURN_SELF;
    }
    m_stDefinition.DefineField(name, ShaderValueType{type, row, column, false});
    RETURN_SELF;
}

AbsIndex ConstantBufferBuilder::DefineRowMajorMatrixField(Script::LuaStack& stack, const char* name, ShaderValueType::ScalarTypes type,
    unsigned row, unsigned column)
{
    NAME_CHECK;
    if (row < 1 || row > 4)
    {
        stack.Error("Row number of the matrix should be in range 1-4, %d is invalid", row);
        RETURN_SELF;
    }
    if (column < 1 || column > 4)
    {
        stack.Error("Column number of the matrix should be in range 1-4, %d is invalid", column);
        RETURN_SELF;
    }
    m_stDefinition.DefineField(name, ShaderValueType{type, row, column, true});
    RETURN_SELF;
}

ConstantBufferWrapper ConstantBufferBuilder::Build(Script::LuaStack& stack)
{
    auto state = BuilderModule::GetScriptState(stack);

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

    return make_shared<const ConstantBufferDefinition>(m_stDefinition);
}
