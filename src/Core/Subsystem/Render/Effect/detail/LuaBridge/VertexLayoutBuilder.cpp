/**
 * @file
 * @date 2022/3/16
 * @author 9chu
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#include "VertexLayoutBuilder.hpp"

#include <lstg/Core/Subsystem/Render/EffectFactory.hpp>
#include "BuilderModule.hpp"

using namespace std;
using namespace lstg;
using namespace lstg::Subsystem::Render::Effect::detail::LuaBridge;

using AbsIndex = Subsystem::Script::LuaStack::AbsIndex;

#define RETURN_SELF \
    return Script::LuaStack::AbsIndex {1u}

AbsIndex VertexLayoutBuilder::AddSlot(Script::LuaStack& st, uint32_t index, ShaderVertexLayoutDefinition::ElementSemanticNames name,
    uint8_t semanticId, std::optional<bool> normalized)
{
    if (m_stDefinition.ContainsSlot(index))
    {
        st.Error("Slot {} already defined", index);
        RETURN_SELF;
    }

    m_stDefinition.AddSlot(index, {name, semanticId}, normalized ? *normalized : true);
    RETURN_SELF;
}

VertexLayoutWrapper VertexLayoutBuilder::Build()
{
    return { make_shared<ShaderVertexLayoutDefinition>(m_stDefinition) };
}
