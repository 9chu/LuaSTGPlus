/**
 * @file
 * @date 2022/3/19
 * @author 9chu
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#include "EffectBuilder.hpp"

using namespace std;
using namespace lstg;
using namespace lstg::Subsystem::Render::Effect::detail::LuaBridge;

using AbsIndex = Subsystem::Script::LuaStack::AbsIndex;

#define RETURN_SELF \
    return Script::LuaStack::AbsIndex {1u}

AbsIndex EffectBuilder::AddPassGroup(Script::LuaStack& stack, EffectPassGroupWrapper* wrapper)
{
    assert(wrapper);
    if (m_stDefinition.ContainsGroup(wrapper->Get()->GetName()))
        stack.Error("Pass group \"%s\" already defined", wrapper->Get()->GetName().c_str());
    m_stDefinition.AddGroup(wrapper->Get());
    RETURN_SELF;
}

EffectWrapper EffectBuilder::Build(Script::LuaStack& stack)
{
    return { make_shared<const EffectDefinition>(m_stDefinition) };
}
