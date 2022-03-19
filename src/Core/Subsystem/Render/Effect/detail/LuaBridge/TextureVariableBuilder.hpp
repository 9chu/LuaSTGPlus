/**
 * @file
 * @date 2022/3/15
 * @author 9chu
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#pragma once
#include <lstg/Core/Subsystem/Script/LuaState.hpp>
#include <lstg/Core/Subsystem/Script/AutoBridgeHint.hpp>
#include <lstg/Core/Subsystem/Render/Effect/ShaderTextureDefinition.hpp>

namespace lstg::Subsystem::Render::Effect::detail::LuaBridge
{
    /**
     * TextureVariable 包装类
     */
    LSTG_CLASS()
    class TextureVariableWrapper
    {
    public:
        TextureVariableWrapper(ImmutableShaderTextureDefinitionPtr def)
            : m_pDefinition(std::move(def))
        {}

    public:
        [[nodiscard]] const auto& Get() const noexcept { return m_pDefinition; }

    private:
        ImmutableShaderTextureDefinitionPtr m_pDefinition;
    };

    /**
     * TextureVariable 构造器
     */
    LSTG_CLASS()
    class TextureVariableBuilder
    {
    public:
        TextureVariableBuilder(ShaderTextureDefinition::TextureTypes type, const char* name);

    public:
        LSTG_METHOD(minFilter)
        Script::LuaStack::AbsIndex SetMinFilter(FilterTypes t);

        LSTG_METHOD(magFilter)
        Script::LuaStack::AbsIndex SetMagFilter(FilterTypes t);

        LSTG_METHOD(mipFilter)
        Script::LuaStack::AbsIndex SetMipFilter(FilterTypes t);

        LSTG_METHOD(addressU)
        Script::LuaStack::AbsIndex SetAddressU(TextureAddressModes u);

        LSTG_METHOD(addressV)
        Script::LuaStack::AbsIndex SetAddressV(TextureAddressModes v);

        LSTG_METHOD(addressW)
        Script::LuaStack::AbsIndex SetAddressW(TextureAddressModes w);

        LSTG_METHOD(maxAnisotropy)
        Script::LuaStack::AbsIndex SetMaxAnisotropy(uint32_t v);

        LSTG_METHOD(borderColor)
        Script::LuaStack::AbsIndex SetBorderColor(float r, float g, float b, float a);

        LSTG_METHOD(build)
        TextureVariableWrapper Build(Script::LuaStack& stack);

    private:
        ShaderTextureDefinition m_stDefinition;
    };
}
