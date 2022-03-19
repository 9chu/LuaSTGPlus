/**
 * @file
 * @date 2022/3/13
 * @author 9chu
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#pragma once
#include <cstdint>
#include <string>
#include <memory>

namespace lstg::Subsystem::Render::Effect
{
    /**
     * 过滤器类型
     */
    enum class FilterTypes
    {
        Point,
        Linear,
        Anisotropic,
    };

    /**
     * 纹理坐标采样方式
     */
    enum class TextureAddressModes
    {
        Wrap,
        Mirror,
        Clamp,
        Border,
    };

    /**
     * Shader 纹理资源定义
     * 简化实现，每个 Texture 总是绑定一个 Sampler。
     */
    class ShaderTextureDefinition
    {
    public:
        // https://docs.microsoft.com/en-us/windows/win32/direct3dhlsl/dx-graphics-hlsl-texture
        enum class TextureTypes
        {
            Texture1D,
            Texture2D,
            Texture3D,
            TextureCube,
        };

        struct SamplerDesc
        {
            FilterTypes MinFilter = FilterTypes::Linear;
            FilterTypes MagFilter = FilterTypes::Linear;
            FilterTypes MipFilter = FilterTypes::Linear;
            TextureAddressModes AddressU = TextureAddressModes::Clamp;
            TextureAddressModes AddressV = TextureAddressModes::Clamp;
            TextureAddressModes AddressW = TextureAddressModes::Clamp;
            uint32_t MaxAnisotropy = 0;
            float BorderColor[4] = {0, 0, 0, 0};

            bool operator==(const SamplerDesc& rhs) const noexcept
            {
                return MinFilter == rhs.MinFilter && MagFilter == rhs.MagFilter && MipFilter == rhs.MipFilter && AddressU == rhs.AddressU &&
                    AddressV == rhs.AddressV && AddressW == rhs.AddressW && MaxAnisotropy == rhs.MaxAnisotropy &&
                    BorderColor == rhs.BorderColor;
            }

            [[nodiscard]] size_t GetHashCode() const noexcept;
        };

    public:
        ShaderTextureDefinition(TextureTypes type, std::string_view name);

        bool operator==(const ShaderTextureDefinition& rhs) const noexcept;

    public:
        /**
         * 获取纹理类型
         */
        [[nodiscard]] TextureTypes GetType() const noexcept { return m_iType; }

        /**
         * 获取名称
         */
        [[nodiscard]] const std::string& GetName() const noexcept { return m_stName; }

        /**
         * 获取参照的 Sampler 名称
         */
        [[nodiscard]] const std::string& GetSuggestedSamplerName() const noexcept { return m_stSuggestedSamplerName; }

        /**
         * 获取采样器描述
         */
        [[nodiscard]] const SamplerDesc& GetSamplerDesc() const noexcept { return m_stSamplerDesc; }
        [[nodiscard]] SamplerDesc& GetSamplerDesc() noexcept { return m_stSamplerDesc; }

        /**
         * 设置采样器描述
         */
        void SetSamplerDesc(const SamplerDesc& desc) noexcept { m_stSamplerDesc = desc; }

        /**
         * 计算 Hash 值
         */
        [[nodiscard]] size_t GetHashCode() const noexcept;

        /**
         * 转到 HLSL 代码
         * @param withSampler 产生配对 Sampler
         */
        void AppendToCode(std::string& out, bool withSampler = true) const;

    private:
        TextureTypes m_iType;
        std::string m_stName;
        std::string m_stSuggestedSamplerName;
        SamplerDesc m_stSamplerDesc;
    };

    using ShaderTextureDefinitionPtr = std::shared_ptr<ShaderTextureDefinition>;
    using ImmutableShaderTextureDefinitionPtr = std::shared_ptr<const ShaderTextureDefinition>;
}

namespace std
{
    template <>
    struct hash<lstg::Subsystem::Render::Effect::ShaderTextureDefinition>
    {
        std::size_t operator()(const lstg::Subsystem::Render::Effect::ShaderTextureDefinition& value) const
        {
            return value.GetHashCode();
        }
    };
}
