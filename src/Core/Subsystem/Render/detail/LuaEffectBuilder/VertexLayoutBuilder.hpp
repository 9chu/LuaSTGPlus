/**
 * @file
 * @date 2022/3/16
 * @author 9chu
 * 此文件为 LuaSTGPlus 项目的一部分，版权与许可声明详见 COPYRIGHT.txt。
 */
#pragma once
#include "Helper.hpp"
#include <lstg/Core/Subsystem/Render/GraphDef/ShaderVertexLayoutDefinition.hpp>

namespace lstg::Subsystem::Render::detail::LuaEffectBuilder
{
    /**
     * ConstantBuffer 包装器
     */
    LSTG_CLASS()
    using VertexLayoutWrapper = ScriptObjectWrapper<GraphDef::ShaderVertexLayoutDefinition>;

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
        Script::LuaStack::AbsIndex AddSlot(Script::LuaStack& stack, uint32_t index,
            GraphDef::ShaderVertexLayoutDefinition::ElementSemanticNames name, uint8_t semanticId, std::optional<bool> normalized);

        LSTG_METHOD(build)
        VertexLayoutWrapper Build();

    private:
        GraphDef::ShaderVertexLayoutDefinition m_stDefinition;
    };
}
