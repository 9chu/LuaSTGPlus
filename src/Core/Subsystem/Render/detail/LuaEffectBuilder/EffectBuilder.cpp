/**
 * @file
 * @date 2022/3/19
 * @author 9chu
 * 此文件为 LuaSTGPlus 项目的一部分，版权与许可声明详见 COPYRIGHT.txt。
 */
#include "EffectBuilder.hpp"

using namespace std;
using namespace lstg;
using namespace lstg::Subsystem::Render::detail::LuaEffectBuilder;

using AbsIndex = Subsystem::Script::LuaStack::AbsIndex;

AbsIndex EffectBuilder::AddPassGroup(Script::LuaStack& stack, EffectPassGroupWrapper* wrapper)
{
    assert(wrapper);
    auto ret = m_stDefinition.AddGroup(wrapper->Get());
    stack.ThrowIfError(ret);
    LSTG_SCRIPT_RETURN_SELF;
}

EffectWrapper EffectBuilder::Build(Script::LuaStack& stack)
{
    if (m_stDefinition.GetGroups().empty())
        stack.Error("At least one pass group must be defined");
    return { make_shared<const GraphDef::EffectDefinition>(m_stDefinition) };
}
