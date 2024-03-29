/**
 * @file
 * @date 2022/3/19
 * @author 9chu
 * 此文件为 LuaSTGPlus 项目的一部分，版权与许可声明详见 COPYRIGHT.txt。
 */
#pragma once
#include "Helper.hpp"
#include <lstg/Core/Subsystem/Render/GraphDef/EffectPassGroupDefinition.hpp>
#include "EffectPassBuilder.hpp"

namespace lstg::Subsystem::Render::detail::LuaEffectBuilder
{
    /**
     * EffectPassGroup 包装类
     */
    LSTG_CLASS()
    using EffectPassGroupWrapper = ScriptObjectWrapper<GraphDef::EffectPassGroupDefinition>;

    /**
     * EffectPassGroup 构造器
     */
    LSTG_CLASS()
    class EffectPassGroupBuilder
    {
    public:
        EffectPassGroupBuilder(std::string_view name);

    public:
        LSTG_METHOD(tag)
        Script::LuaStack::AbsIndex SetTag(std::string_view key, std::string_view value);

        LSTG_METHOD(pass)
        Script::LuaStack::AbsIndex AddPass(Script::LuaStack& stack, EffectPassWrapper* wrapper);

        LSTG_METHOD(build)
        EffectPassGroupWrapper Build(Script::LuaStack& stack);

    private:
        GraphDef::EffectPassGroupDefinition m_stDefinition;
    };
}
