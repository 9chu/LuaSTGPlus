/**
 * @file
 * @date 2022/3/15
 * @author 9chu
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#include "BuilderModule.hpp"

#include <lstg/Core/Subsystem/Render/EffectFactory.hpp>

using namespace std;
using namespace lstg;
using namespace lstg::Subsystem::Render::Effect::detail::LuaBridge;

bool BuilderModule::IsValidIdentifier(std::string_view name) noexcept
{
    enum {
        STATE_IDENTIFIER_START,
        STATE_IDENTIFIER,
    } state = STATE_IDENTIFIER_START;

    for (size_t i = 0; i <= name.size(); ++i)
    {
        char ch = (i >= name.size()) ? '\0' : name[i];
        switch (state)
        {
            case STATE_IDENTIFIER_START:
                if (('a' <= ch && ch <= 'z') || ('A' <= ch && ch <= 'Z') || ch == '_')
                    state = STATE_IDENTIFIER;
                else
                    return false;
                break;
            case STATE_IDENTIFIER:
                if (ch == '\0')
                    break;
                if (!(('a' <= ch && ch <= 'z') || ('A' <= ch && ch <= 'Z') || ('0' <= ch && ch <= '9') || ch == '_'))
                    return false;
                break;
        }
    }
    return true;
}

lstg::Subsystem::Render::detail::EffectScriptState* BuilderModule::GetScriptState(Script::LuaStack& st)
{
    lua_getfield(st, LUA_REGISTRYINDEX, "_state");
    auto state = static_cast<Render::detail::EffectScriptState*>(lua_touserdata(st, -1));
    assert(state);
    lua_pop(st, 1);
    return state;
}

ConstantBufferBuilder BuilderModule::DefineConstantBuffer(Script::LuaStack& st, const char* name)
{
    if (!IsValidIdentifier(name))
        st.Error("\"%s\" is not a valid identifier", name);
    return { name };
}

ConstantBufferWrapper BuilderModule::ImportGlobalConstantBuffer(Script::LuaStack& st, const char* name)
{
    auto state = GetScriptState(st);
    auto factory = state->Factory;
    auto cbuffer = factory->GetGlobalConstantBuffer(name);
    if (!cbuffer)
        st.Error("Global constant buffer \"%s\" not found", name);

    // 预先构造的对象只能在导入的时候进行名字校验
    // 其他对象在 Build 的时候进行校验

    // 检查 Symbol 定义情况
    if (state->ContainsSymbol(cbuffer->GetName()))
        st.Error("Symbol \"%s\" already defined in script", cbuffer->GetName().c_str());
    for (const auto& e : cbuffer->GetFields())
    {
        if (state->ContainsSymbol(e.Name))
            st.Error("Symbol \"%s\" already defined in script", e.Name.c_str());
    }

    // 定义 Symbol
    state->SymbolCache.emplace(cbuffer->GetName());
    for (const auto& e : cbuffer->GetFields())
        state->SymbolCache.emplace(e.Name);

    return { cbuffer };
}

TextureVariableBuilder BuilderModule::DefineTexture1D(Script::LuaStack& st, const char* name)
{
    if (!IsValidIdentifier(name))
        st.Error("\"%s\" is not a valid identifier", name);
    return { ShaderTextureDefinition::TextureTypes::Texture1D, name };
}

TextureVariableBuilder BuilderModule::DefineTexture2D(Script::LuaStack& st, const char* name)
{
    if (!IsValidIdentifier(name))
        st.Error("\"%s\" is not a valid identifier", name);
    return { ShaderTextureDefinition::TextureTypes::Texture2D, name };
}

TextureVariableBuilder BuilderModule::DefineTexture3D(Script::LuaStack& st, const char* name)
{
    if (!IsValidIdentifier(name))
        st.Error("\"%s\" is not a valid identifier", name);
    return { ShaderTextureDefinition::TextureTypes::Texture3D, name };
}

TextureVariableBuilder BuilderModule::DefineTextureCube(Script::LuaStack& st, const char* name)
{
    if (!IsValidIdentifier(name))
        st.Error("\"%s\" is not a valid identifier", name);
    return { ShaderTextureDefinition::TextureTypes::TextureCube, name };
}

VertexLayoutBuilder BuilderModule::DefineVertexLayout(Script::LuaStack& st)
{
    return {};
}

ShaderBuilder BuilderModule::DefineVertexShader(Script::LuaStack& st, const char* source)
{
    ShaderBuilder ret(ShaderDefinition::ShaderTypes::VertexShader);
    ret.Get().SetSource(source);
    return ret;
}

ShaderBuilder BuilderModule::DefinePixelShader(Script::LuaStack& st, const char* source)
{
    ShaderBuilder ret(ShaderDefinition::ShaderTypes::PixelShader);
    ret.Get().SetSource(source);
    return ret;
}

EffectPassBuilder BuilderModule::DefinePass(Script::LuaStack& st, const char* name)
{
    if (!IsValidIdentifier(name))
        st.Error("\"%s\" is not a valid identifier", name);
    EffectPassBuilder ret(name);
    return ret;
}

EffectPassGroupBuilder BuilderModule::DefinePassGroup(Script::LuaStack& st, const char* name)
{
    if (!IsValidIdentifier(name))
        st.Error("\"%s\" is not a valid identifier", name);
    EffectPassGroupBuilder ret(name);
    return ret;
}

EffectBuilder BuilderModule::DefineEffect()
{
    EffectBuilder ret;
    return ret;
}
