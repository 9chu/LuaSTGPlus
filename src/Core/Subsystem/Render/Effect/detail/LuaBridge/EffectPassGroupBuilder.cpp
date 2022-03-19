/**
 * @file
 * @date 2022/3/19
 * @author 9chu
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#include "EffectPassGroupBuilder.hpp"

using namespace std;
using namespace lstg;
using namespace lstg::Subsystem::Render::Effect::detail::LuaBridge;

using AbsIndex = Subsystem::Script::LuaStack::AbsIndex;

#define RETURN_SELF \
    return Script::LuaStack::AbsIndex {1u}

EffectPassGroupBuilder::EffectPassGroupBuilder(std::string_view name)
{
    m_stDefinition.SetName(string{name});
}

AbsIndex EffectPassGroupBuilder::SetTag(std::string_view key, std::string_view value)
{
    m_stDefinition.SetTag(key, value);
    RETURN_SELF;
}

AbsIndex EffectPassGroupBuilder::AddPass(Script::LuaStack& stack, EffectPassWrapper* wrapper)
{
    assert(wrapper);
    if (m_stDefinition.ContainsPass(wrapper->Get()->GetName()))
        stack.Error("Pass \"%s\" already defined", wrapper->Get()->GetName().c_str());
    m_stDefinition.AddPass(wrapper->Get());
    RETURN_SELF;
}

EffectPassGroupWrapper EffectPassGroupBuilder::Build(Script::LuaStack& stack)
{
    return { make_shared<const EffectPassGroupDefinition>(m_stDefinition) };
}
