/**
 * @file
 * @date 2022/3/16
 * @author 9chu
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#include "VertexLayoutBuilder.hpp"

using namespace std;
using namespace lstg;
using namespace lstg::Subsystem::Render::detail::LuaEffectBuilder;

using AbsIndex = Subsystem::Script::LuaStack::AbsIndex;

AbsIndex VertexLayoutBuilder::AddSlot(Script::LuaStack& stack, uint32_t index,
    GraphDef::ShaderVertexLayoutDefinition::ElementSemanticNames name, uint8_t semanticId, std::optional<bool> normalized)
{
    auto ret = m_stDefinition.AddSlot(index, {name, semanticId}, normalized ? *normalized : true);
    stack.ThrowIfError(ret);
    LSTG_SCRIPT_RETURN_SELF;
}

VertexLayoutWrapper VertexLayoutBuilder::Build()
{
    return { make_shared<GraphDef::ShaderVertexLayoutDefinition>(m_stDefinition) };
}
