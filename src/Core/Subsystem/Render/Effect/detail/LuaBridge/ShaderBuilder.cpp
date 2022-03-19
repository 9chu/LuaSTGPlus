/**
 * @file
 * @date 2022/3/16
 * @author 9chu
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#include "ShaderBuilder.hpp"

#include <lstg/Core/Subsystem/Render/EffectFactory.hpp>
#include "BuilderModule.hpp"

using namespace std;
using namespace lstg;
using namespace lstg::Subsystem::Render::Effect::detail::LuaBridge;

using AbsIndex = Subsystem::Script::LuaStack::AbsIndex;

#define RETURN_SELF \
    return Script::LuaStack::AbsIndex {1u}

ShaderBuilder::ShaderBuilder(ShaderDefinition::ShaderTypes t)
    : m_stDefinition(t)
{
}

AbsIndex ShaderBuilder::SetName(const char* name)
{
    m_stDefinition.SetName(name);
    RETURN_SELF;
}

AbsIndex ShaderBuilder::SetEntry(const char* name)
{
    m_stDefinition.SetEntry(name);
    RETURN_SELF;
}

AbsIndex ShaderBuilder::Use(Script::LuaStack& st, std::variant<ConstantBufferWrapper*, TextureVariableWrapper*> resource)
{
    if (resource.index() == 0)
    {
        auto cbw = std::get<0>(resource);
        assert(cbw);
        auto& cb = cbw->Get();

        // 检查 ConstantBuffer 是否定义
        // 只用检查 ConstantBuffer 的名字是否引入即可
        if (m_stDefinition.ContainsSymbol(cb->GetName()))
            st.Error("Symbol \"%s\" already defined", cb->GetName().c_str());

        m_stDefinition.AddConstantBufferReference(cb);
    }
    else
    {
        assert(resource.index() == 1);
        auto tvw = std::get<1>(resource);
        assert(tvw);
        auto& tv = tvw->Get();

        // 检查纹理是否定义
        // 只用检查 Texture 的名字是否引入即可
        if (m_stDefinition.ContainsSymbol(tv->GetName()))
            st.Error("Symbol \"%s\" already defined", tv->GetName().c_str());

        m_stDefinition.AddTextureReference(tv);
    }
    RETURN_SELF;
}

AbsIndex ShaderBuilder::SetVertexLayout(Script::LuaStack& st, VertexLayoutWrapper* vertexLayout)
{
    assert(vertexLayout);
    if (m_stDefinition.GetType() == ShaderDefinition::ShaderTypes::PixelShader)
    {
        st.Error("Cannot set vertex layout for pixel shader");
        RETURN_SELF;
    }
    m_stDefinition.SetVertexLayout(vertexLayout->Get());
    RETURN_SELF;
}

ShaderWrapper ShaderBuilder::Build(Script::LuaStack& st)
{
    auto state = BuilderModule::GetScriptState(st);
    auto [ret, err] = state->Factory->CompileShader(m_stDefinition);
    if (!ret)
        st.Error("Compile shader fail: %s", err.c_str());
    return { ret };
}
