/**
 * @file
 * @date 2022/3/15
 * @author 9chu
 * 此文件为 LuaSTGPlus 项目的一部分，版权与许可声明详见 COPYRIGHT.txt。
 */
#include "BuilderModule.hpp"

#include <lstg/Core/Subsystem/Render/EffectFactory.hpp>
#include "../../GraphDef/detail/NameCheck.hpp"

using namespace std;
using namespace lstg;
using namespace lstg::Subsystem::Render::detail::LuaEffectBuilder;

ConstantBufferBuilder BuilderModule::DefineConstantBuffer(Script::LuaStack& stack, const char* name)
{
    if (!GraphDef::detail::IsValidIdentifier(name))
        stack.Error("\"%s\" is not a valid identifier", name);
    return { name };
}

ConstantBufferWrapper BuilderModule::ImportGlobalConstantBuffer(Script::LuaStack& stack, const char* name)
{
    auto state = BuilderGlobalState::FromLuaStack(stack);
    auto factory = state->Factory;
    auto cb = factory->GetGlobalConstantBuffer(name);
    if (!cb)
        stack.Error("global constant buffer \"%s\" not found", name);
    auto def = cb->GetDefinition();
    assert(def);

    // 预先构造的对象只能在导入的时候进行名字校验
    // 其他对象在 Build 的时候进行校验

    // 检查 Symbol 定义情况
    if (state->ContainsSymbol(def->GetName()))
        stack.Error("Symbol \"%s\" already defined in script", def->GetName().c_str());
    for (const auto& e : def->GetFields())
    {
        if (state->ContainsSymbol(e.Name))
            stack.Error("Symbol \"%s\" already defined in script", e.Name.c_str());
    }

    // 定义 Symbol
    state->SymbolCache.emplace(def->GetName());
    for (const auto& e : def->GetFields())
        state->SymbolCache.emplace(e.Name);

    return { def, true };
}

TextureVariableBuilder BuilderModule::DefineTexture1D(Script::LuaStack& stack, const char* name)
{
    if (!GraphDef::detail::IsValidIdentifier(name))
        stack.Error("\"%s\" is not a valid identifier", name);
    return { GraphDef::ShaderTextureDefinition::TextureTypes::Texture1D, name };
}

TextureVariableBuilder BuilderModule::DefineTexture2D(Script::LuaStack& stack, const char* name)
{
    if (!GraphDef::detail::IsValidIdentifier(name))
        stack.Error("\"%s\" is not a valid identifier", name);
    return { GraphDef::ShaderTextureDefinition::TextureTypes::Texture2D, name };
}

TextureVariableBuilder BuilderModule::DefineTexture3D(Script::LuaStack& stack, const char* name)
{
    if (!GraphDef::detail::IsValidIdentifier(name))
        stack.Error("\"%s\" is not a valid identifier", name);
    return { GraphDef::ShaderTextureDefinition::TextureTypes::Texture3D, name };
}

TextureVariableBuilder BuilderModule::DefineTextureCube(Script::LuaStack& stack, const char* name)
{
    if (!GraphDef::detail::IsValidIdentifier(name))
        stack.Error("\"%s\" is not a valid identifier", name);
    return { GraphDef::ShaderTextureDefinition::TextureTypes::TextureCube, name };
}

VertexLayoutBuilder BuilderModule::DefineVertexLayout(Script::LuaStack& stack)
{
    return {};
}

ShaderBuilder BuilderModule::DefineVertexShader(Script::LuaStack& stack, const char* source)
{
    ShaderBuilder ret(GraphDef::ShaderDefinition::ShaderTypes::VertexShader);
    (*ret).SetSource(source);
    return ret;
}

ShaderBuilder BuilderModule::DefinePixelShader(Script::LuaStack& stack, const char* source)
{
    ShaderBuilder ret(GraphDef::ShaderDefinition::ShaderTypes::PixelShader);
    (*ret).SetSource(source);
    return ret;
}

EffectPassBuilder BuilderModule::DefinePass(Script::LuaStack& stack, const char* name)
{
    if (!GraphDef::detail::IsValidIdentifier(name))
        stack.Error("\"%s\" is not a valid identifier", name);
    return { name };
}

EffectPassGroupBuilder BuilderModule::DefinePassGroup(Script::LuaStack& stack, const char* name)
{
    if (!GraphDef::detail::IsValidIdentifier(name))
        stack.Error("\"%s\" is not a valid identifier", name);
    return { name };
}

EffectBuilder BuilderModule::DefineEffect()
{
    return {};
}
