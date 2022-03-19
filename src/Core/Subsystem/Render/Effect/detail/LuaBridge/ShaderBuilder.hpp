/**
 * @file
 * @date 2022/3/16
 * @author 9chu
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#pragma once
#include <lstg/Core/Subsystem/Script/LuaState.hpp>
#include <lstg/Core/Subsystem/Script/AutoBridgeHint.hpp>
#include <lstg/Core/Subsystem/Render/Effect/ShaderDefinition.hpp>
#include "ConstantBufferBuilder.hpp"
#include "TextureVariableBuilder.hpp"
#include "VertexLayoutBuilder.hpp"

namespace lstg::Subsystem::Render::Effect::detail::LuaBridge
{
    /**
     * Shader 包装类
     */
    LSTG_CLASS()
    class ShaderWrapper
    {
    public:
        ShaderWrapper(ImmutableShaderDefinitionPtr def)
            : m_pDefinition(std::move(def))
        {}

    public:
        [[nodiscard]] const auto& Get() const noexcept { return m_pDefinition; }

    private:
        ImmutableShaderDefinitionPtr m_pDefinition;
    };

    /**
     * Shader 构造器
     */
    LSTG_CLASS()
    class ShaderBuilder
    {
    public:
        ShaderBuilder(ShaderDefinition::ShaderTypes t);

    public:
        [[nodiscard]] auto& Get() noexcept { return m_stDefinition; }

        LSTG_METHOD(name)
        Script::LuaStack::AbsIndex SetName(const char* name);

        LSTG_METHOD(entry)
        Script::LuaStack::AbsIndex SetEntry(const char* name);

        LSTG_METHOD(use)
        Script::LuaStack::AbsIndex Use(Script::LuaStack& st, std::variant<ConstantBufferWrapper*, TextureVariableWrapper*> resource);

        LSTG_METHOD(vertexLayout)
        Script::LuaStack::AbsIndex SetVertexLayout(Script::LuaStack& st, VertexLayoutWrapper* vertexLayout);

        LSTG_METHOD(build)
        ShaderWrapper Build(Script::LuaStack& st);

    private:
        ShaderDefinition m_stDefinition;
    };
}
