/**
 * @file
 * @date 2022/3/19
 * @author 9chu
 * 此文件为 LuaSTGPlus 项目的一部分，版权与许可声明详见 COPYRIGHT.txt。
 */
#include "EffectPassGroupBuilder.hpp"

using namespace std;
using namespace lstg;
using namespace lstg::Subsystem::Render::detail::LuaEffectBuilder;

using AbsIndex = Subsystem::Script::LuaStack::AbsIndex;

EffectPassGroupBuilder::EffectPassGroupBuilder(std::string_view name)
{
    m_stDefinition.SetName(string{name});
}

AbsIndex EffectPassGroupBuilder::SetTag(std::string_view key, std::string_view value)
{
    m_stDefinition.SetTag(key, value);
    LSTG_SCRIPT_RETURN_SELF;
}

AbsIndex EffectPassGroupBuilder::AddPass(Script::LuaStack& stack, EffectPassWrapper* wrapper)
{
    assert(wrapper);
    auto ret = m_stDefinition.AddPass(wrapper->Get());
    stack.ThrowIfError(ret);
    LSTG_SCRIPT_RETURN_SELF;
}

EffectPassGroupWrapper EffectPassGroupBuilder::Build(Script::LuaStack& stack)
{
    if (m_stDefinition.GetPasses().empty())
        stack.Error("At least one pass must be defined");
    return { make_shared<const GraphDef::EffectPassGroupDefinition>(m_stDefinition) };
}
