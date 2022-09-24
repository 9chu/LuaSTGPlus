/**
 * @file
 * @date 2022/3/15
 * @author 9chu
 * 此文件为 LuaSTGPlus 项目的一部分，版权与许可声明详见 COPYRIGHT.txt。
 */
#pragma once
#include "Helper.hpp"
#include <lstg/Core/Subsystem/Render/GraphDef/ShaderTextureDefinition.hpp>

namespace lstg::Subsystem::Render::detail::LuaEffectBuilder
{
    /**
     * TextureVariable 包装类
     */
    LSTG_CLASS()
    using TextureVariableWrapper = ScriptObjectWrapper<GraphDef::ShaderTextureDefinition>;

    /**
     * TextureVariable 构造器
     */
    LSTG_CLASS()
    class TextureVariableBuilder
    {
    public:
        TextureVariableBuilder(GraphDef::ShaderTextureDefinition::TextureTypes type, const char* name);

    public:
        LSTG_METHOD(minFilter)
        Script::LuaStack::AbsIndex SetMinFilter(GraphDef::FilterTypes t);

        LSTG_METHOD(magFilter)
        Script::LuaStack::AbsIndex SetMagFilter(GraphDef::FilterTypes t);

        LSTG_METHOD(mipFilter)
        Script::LuaStack::AbsIndex SetMipFilter(GraphDef::FilterTypes t);

        LSTG_METHOD(addressU)
        Script::LuaStack::AbsIndex SetAddressU(GraphDef::TextureAddressModes u);

        LSTG_METHOD(addressV)
        Script::LuaStack::AbsIndex SetAddressV(GraphDef::TextureAddressModes v);

        LSTG_METHOD(addressW)
        Script::LuaStack::AbsIndex SetAddressW(GraphDef::TextureAddressModes w);

        LSTG_METHOD(maxAnisotropy)
        Script::LuaStack::AbsIndex SetMaxAnisotropy(uint32_t v);

        LSTG_METHOD(borderColor)
        Script::LuaStack::AbsIndex SetBorderColor(float r, float g, float b, float a);

        LSTG_METHOD(build)
        TextureVariableWrapper Build(Script::LuaStack& stack);

    private:
        GraphDef::ShaderTextureDefinition m_stDefinition;
    };
}
