/**
 * @file
 * @date 2022/3/16
 * @author 9chu
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#pragma once
#include <lstg/Core/Subsystem/Script/LuaState.hpp>
#include <lstg/Core/Subsystem/Script/AutoBridgeHint.hpp>
#include <lstg/Core/Subsystem/Render/Effect/ShaderVertexLayoutDefinition.hpp>

namespace lstg::Subsystem::Render::Effect::detail::LuaBridge
{
    /**
     * VertexLayout 包装类
     */
    LSTG_CLASS()
    class VertexLayoutWrapper
    {
    public:
        VertexLayoutWrapper(ImmutableShaderVertexLayoutDefinitionPtr def)
            : m_pDefinition(std::move(def))
        {}

    public:
        [[nodiscard]] const auto& Get() const noexcept { return m_pDefinition; }

    private:
        ImmutableShaderVertexLayoutDefinitionPtr m_pDefinition;
    };

    /**
     * VertexLayout 构造器
     */
    LSTG_CLASS()
    class VertexLayoutBuilder
    {
    public:
        VertexLayoutBuilder() = default;

    public:
        LSTG_METHOD(slot)
        Script::LuaStack::AbsIndex AddSlot(Script::LuaStack& st, uint32_t index, ShaderVertexLayoutDefinition::ElementSemanticNames name,
            uint8_t semanticId, std::optional<bool> normalized);

        LSTG_METHOD(build)
        VertexLayoutWrapper Build();

    private:
        ShaderVertexLayoutDefinition m_stDefinition;
    };
}
