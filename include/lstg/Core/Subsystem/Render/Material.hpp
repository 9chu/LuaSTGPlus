/**
 * @file
 * @date 2022/3/20
 * @author 9chu
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#pragma once
#include <unordered_map>
#include "EffectFactory.hpp"
#include "ConstantBuffer.hpp"
#include "Texture.hpp"

namespace Diligent
{
    struct IShaderResourceBinding;
    struct IShaderResourceVariable;
}

namespace lstg::Subsystem
{
    class RenderSystem;
}

namespace lstg::Subsystem::Render
{
    /**
     * 材质
     * 材质 = Effect + Data(CBuffer & Texture)
     */
    class Material
    {
        friend class lstg::Subsystem::RenderSystem;

    public:
        Material(RenderDevice& device, GraphDef::ImmutableEffectDefinitionPtr definition, const TexturePtr& defaultTex2D);
        Material(const Material&) = delete;
        Material(Material&&) = delete;

    public:
        /**
         * 获取定义
         */
        [[nodiscard]] const GraphDef::ImmutableEffectDefinitionPtr& GetDefinition() const noexcept { return m_pDefinition; }

        /**
         * 获取变量
         * @tparam T 类型
         * @param symbol 符号名
         * @return 值
         */
        template <typename T>
        Result<T> GetUniform(std::string_view symbol) noexcept
        {
            auto info = m_pDefinition->GetSymbol(symbol);
            if (!info)
                return make_error_code(GraphDef::DefinitionError::SymbolNotFound);

            // 只能设置 Uniform
            // Global Uniform 需要在外部进行设置
            if (info->Type != GraphDef::ShaderDefinition::SymbolTypes::Uniform)
                return make_error_code(std::errc::invalid_argument);

            // 找到对应的 CBuffer
            auto cb = std::get<GraphDef::EffectDefinition::UniformSymbolInfo>(info->AssocInfo).Definition;
            auto it = m_stCBufferInstances.find(cb.get());
            assert(it != m_stCBufferInstances.end());
            return it->second->GetUniform<T>(symbol);
        }

        /**
         * 设置变量
         * @tparam T 类型
         * @param symbol 符号
         * @param value 值
         * @return 是否成功
         */
        template <typename T>
        Result<void> SetUniform(std::string_view symbol, const T& value) noexcept
        {
            auto info = m_pDefinition->GetSymbol(symbol);
            if (!info)
                return make_error_code(GraphDef::DefinitionError::SymbolNotFound);

            // 只能设置 Uniform
            // Global Uniform 需要在外部进行设置
            if (info->Type != GraphDef::ShaderDefinition::SymbolTypes::Uniform)
                return make_error_code(std::errc::invalid_argument);

            // 找到对应的 CBuffer
            auto cb = std::get<GraphDef::EffectDefinition::UniformSymbolInfo>(info->AssocInfo).Definition;
            auto it = m_stCBufferInstances.find(cb.get());
            assert(it != m_stCBufferInstances.end());
            return it->second->SetUniform(symbol, value);
        }

        /**
         * 设置纹理
         * @param symbol 符号
         * @param texture 纹理指针
         * @return 是否成功
         */
        Result<void> SetTexture(std::string_view symbol, const TexturePtr& texture) noexcept;

    private:
        /**
         * 提交数据
         * @param group 关联的组
         */
        Result<void> Commit(const GraphDef::EffectPassGroupDefinition* group) noexcept;

    private:
        struct PassInstance
        {
            bool BindingDirty = true;
            Diligent::IShaderResourceBinding* ResourceBinding = nullptr;

            PassInstance() = default;
            PassInstance(const PassInstance& org);
            PassInstance(PassInstance&& org) noexcept;
            ~PassInstance();
        };

        struct TextureVariableRef
        {
            PassInstance* Pass;
            Diligent::IShaderResourceVariable* VertexShaderResource;
            Diligent::IShaderResourceVariable* PixelShaderResource;
        };

        struct TextureVariableState
        {
            TexturePtr BindingTexture;
            std::vector<TextureVariableRef> References;
        };

        RenderDevice& m_stRenderDevice;
        GraphDef::ImmutableEffectDefinitionPtr m_pDefinition;

        std::unordered_map<const GraphDef::ConstantBufferDefinition*, ConstantBufferPtr> m_stCBufferInstances;  // CBuffer 实例
        std::unordered_map<const GraphDef::ShaderTextureDefinition*, TextureVariableState> m_stTextureVariableInstances;  // TexVar 实例
        std::unordered_map<const GraphDef::EffectPassDefinition*, PassInstance> m_stPassInstances;  // Pass 实例
    };

    using MaterialPtr = std::shared_ptr<Material>;
}
