/**
 * @file
 * @date 2022/3/13
 * @author 9chu
 * 此文件为 LuaSTGPlus 项目的一部分，版权与许可声明详见 COPYRIGHT.txt。
 */
#include <lstg/Core/Subsystem/Render/GraphDef/ShaderTextureDefinition.hpp>

#include <cassert>
#include <fmt/format.h>

using namespace std;
using namespace lstg;
using namespace lstg::Subsystem::Render::GraphDef;

namespace
{
    /**
     * 写到代码
     * @param out 输出
     * @param type 纹理类型
     */
    void AppendToCode(std::string& out, ShaderTextureDefinition::TextureTypes type)
    {
        // https://docs.microsoft.com/en-us/windows/win32/direct3dhlsl/dx-graphics-hlsl-texture
        // https://docs.microsoft.com/en-us/windows/win32/direct3dhlsl/dx-graphics-hlsl-sampler
        switch (type)
        {
            case ShaderTextureDefinition::TextureTypes::Texture1D:
                out.append("Texture1D");
                break;
            case ShaderTextureDefinition::TextureTypes::Texture2D:
                out.append("Texture2D");
                break;
            case ShaderTextureDefinition::TextureTypes::Texture3D:
                out.append("Texture3D");
                break;
            case ShaderTextureDefinition::TextureTypes::TextureCube:
                out.append("TextureCube");
                break;
            default:
                assert(false);
                break;
        }
    }
}

size_t ShaderTextureDefinition::SamplerDesc::GetHashCode() const noexcept
{
    auto ret = std::hash<string_view>{}("Sampler");
    ret ^= std::hash<FilterTypes>{}(MinFilter);
    ret ^= std::hash<FilterTypes>{}(MagFilter);
    ret ^= std::hash<FilterTypes>{}(MipFilter);
    ret ^= std::hash<TextureAddressModes>{}(AddressU);
    ret ^= std::hash<TextureAddressModes>{}(AddressV);
    ret ^= std::hash<TextureAddressModes>{}(AddressW);
    ret ^= std::hash<uint32_t>{}(MaxAnisotropy);
    ret ^= std::hash<float>{}(BorderColor[0]);
    ret ^= std::hash<float>{}(BorderColor[1]);
    ret ^= std::hash<float>{}(BorderColor[2]);
    ret ^= std::hash<float>{}(BorderColor[3]);
    return ret;
}

ShaderTextureDefinition::ShaderTextureDefinition(TextureTypes type, std::string_view name)
    : m_iType(type), m_stName(name), m_stSuggestedSamplerName(fmt::format("{}Sampler", name))
{
}

bool ShaderTextureDefinition::operator==(const ShaderTextureDefinition& rhs) const noexcept
{
    auto ret = (m_iType == rhs.m_iType && m_stName == rhs.m_stName && m_stSamplerDesc == rhs.m_stSamplerDesc);
    assert(!ret || m_stSuggestedSamplerName == rhs.m_stSuggestedSamplerName);
    return ret;
}

size_t ShaderTextureDefinition::GetHashCode() const noexcept
{
    return std::hash<std::string_view>{}("ShaderTexture") ^ std::hash<std::string>{}(m_stName) ^
        std::hash<TextureTypes>{}(m_iType) ^ m_stSamplerDesc.GetHashCode();
}

void ShaderTextureDefinition::AppendToCode(std::string& out, bool withSampler) const
{
    ::AppendToCode(out, m_iType);
    fmt::format_to(std::back_inserter(out), " {};\n", m_stName);
    if (withSampler)
    {
        // 由于 GLSL 不支持 Texture 和 Sampler 分离，Diligent 要求 Texture 和 Sampler 必须配对出现，否则 HLSL to GLSL 的过程会失败
        // 这里我们总是给 Sampler 加一个 Sampler 后缀
        fmt::format_to(std::back_inserter(out), "SamplerState {};\n", m_stSuggestedSamplerName);
    }
}
