/**
 * @file
 * @date 2022/3/16
 * @author 9chu
 * 此文件为 LuaSTGPlus 项目的一部分，版权与许可声明详见 COPYRIGHT.txt。
 */
#pragma once
#include "Helper.hpp"
#include <lstg/Core/Subsystem/Render/GraphDef/ShaderDefinition.hpp>
#include "ConstantBufferBuilder.hpp"
#include "TextureVariableBuilder.hpp"
#include "VertexLayoutBuilder.hpp"

namespace lstg::Subsystem::Render::detail::LuaEffectBuilder
{
    /**
     * Shader 包装类
     */
    LSTG_CLASS()
    using ShaderWrapper = ScriptObjectWrapper<GraphDef::ShaderDefinition>;

    /**
     * Shader 构造器
     */
    LSTG_CLASS()
    class ShaderBuilder
    {
    public:
        ShaderBuilder(GraphDef::ShaderDefinition::ShaderTypes t);

        GraphDef::ShaderDefinition& operator*() noexcept { return m_stDefinition; }

    public:
        LSTG_METHOD(name)
        Script::LuaStack::AbsIndex SetName(const char* name);

        LSTG_METHOD(entry)
        Script::LuaStack::AbsIndex SetEntry(const char* name);

        LSTG_METHOD(use)
        Script::LuaStack::AbsIndex Use(Script::LuaStack& stack, std::variant<ConstantBufferWrapper*, TextureVariableWrapper*> resource);

        LSTG_METHOD(vertexLayout)
        Script::LuaStack::AbsIndex SetVertexLayout(Script::LuaStack& stack, VertexLayoutWrapper* vertexLayout);

        LSTG_METHOD(build)
        ShaderWrapper Build(Script::LuaStack& stack);

    private:
        GraphDef::ShaderDefinition m_stDefinition;
    };
}
