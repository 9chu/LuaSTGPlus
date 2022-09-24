/**
 * @file
 * @date 2022/3/17
 * @author 9chu
 * 此文件为 LuaSTGPlus 项目的一部分，版权与许可声明详见 COPYRIGHT.txt。
 */
#pragma once
#include "Helper.hpp"
#include <lstg/Core/Subsystem/Render/GraphDef/EffectDefinition.hpp>
#include "EffectPassGroupBuilder.hpp"

namespace lstg::Subsystem::Render::detail::LuaEffectBuilder
{
    /**
     * Effect 包装类
     */
    LSTG_CLASS()
    using EffectWrapper = ScriptObjectWrapper<GraphDef::EffectDefinition>;

    /**
     * Effect 构造器
     */
    LSTG_CLASS()
    class EffectBuilder
    {
    public:
        EffectBuilder() = default;

    public:
        LSTG_METHOD(passGroup)
        Script::LuaStack::AbsIndex AddPassGroup(Script::LuaStack& stack, EffectPassGroupWrapper* group);

        LSTG_METHOD(build)
        EffectWrapper Build(Script::LuaStack& stack);

    private:
        GraphDef::EffectDefinition m_stDefinition;
    };
}
