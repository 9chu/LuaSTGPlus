/**
 * @file
 * @date 2022/3/16
 * @author 9chu
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#include "ShaderBuilder.hpp"

#include <lstg/Core/Subsystem/VFS/Path.hpp>
#include <lstg/Core/Subsystem/Render/EffectFactory.hpp>

using namespace std;
using namespace lstg;
using namespace lstg::Subsystem::Render::detail::LuaEffectBuilder;

using AbsIndex = Subsystem::Script::LuaStack::AbsIndex;

ShaderBuilder::ShaderBuilder(GraphDef::ShaderDefinition::ShaderTypes t)
    : m_stDefinition(t)
{
}

AbsIndex ShaderBuilder::SetName(const char* name)
{
    m_stDefinition.SetName(name);
    LSTG_SCRIPT_RETURN_SELF;
}

AbsIndex ShaderBuilder::SetEntry(const char* name)
{
    m_stDefinition.SetEntry(name);
    LSTG_SCRIPT_RETURN_SELF;
}

AbsIndex ShaderBuilder::Use(Script::LuaStack& stack, std::variant<ConstantBufferWrapper*, TextureVariableWrapper*> resource)
{
    if (resource.index() == 0)
    {
        auto cbw = std::get<0>(resource);
        assert(cbw);
        auto& cb = cbw->Get();

        auto ret = m_stDefinition.AddConstantBuffer(cb, cbw->IsGlobal());
        stack.ThrowIfError(ret);
    }
    else
    {
        assert(resource.index() == 1);
        auto tvw = std::get<1>(resource);
        assert(tvw);
        auto& tv = tvw->Get();

        auto ret = m_stDefinition.AddTexture(tv);
        stack.ThrowIfError(ret);
    }
    LSTG_SCRIPT_RETURN_SELF;
}

AbsIndex ShaderBuilder::SetVertexLayout(Script::LuaStack& stack, VertexLayoutWrapper* vertexLayout)
{
    auto ret = m_stDefinition.SetVertexLayout(vertexLayout->Get());
    stack.ThrowIfError(ret);
    LSTG_SCRIPT_RETURN_SELF;
}

ShaderWrapper ShaderBuilder::Build(Script::LuaStack& stack)
{
    auto state = BuilderGlobalState::FromLuaStack(stack);
    auto searchBase = Subsystem::VFS::Path {stack.GetTopLevelScriptPath()}.GetParent().ToString();
    auto ec = state->Factory->CompileShader(m_stDefinition, searchBase.c_str());
    stack.ThrowIfError(ec);
    return std::move(*ec);
}
