/**
 * @file
 * @date 2022/3/20
 * @author 9chu
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#include <lstg/Core/Subsystem/Render/Material.hpp>

#include <Buffer.h>
#include <ShaderResourceBinding.h>
#include <PipelineResourceSignature.h>
#include <RefCntAutoPtr.hpp>
#include <lstg/Core/Logging.hpp>

using namespace std;
using namespace lstg;
using namespace lstg::Subsystem::Render;

LSTG_DEF_LOG_CATEGORY(Material);

// <editor-fold desc="Material::PassInstance">

Material::PassInstance::PassInstance(const PassInstance& org)
    : ResourceBinding(org.ResourceBinding)
{
    if (ResourceBinding)
        ResourceBinding->AddRef();
}

Material::PassInstance::PassInstance(PassInstance&& org) noexcept
{
    std::swap(ResourceBinding, org.ResourceBinding);
}

Material::PassInstance::~PassInstance()
{
    if (ResourceBinding)
    {
        ResourceBinding->Release();
        ResourceBinding = nullptr;
    }
}

// </editor-fold>

// <editor-fold desc="Material::Material">

Material::Material(RenderDevice& device, GraphDef::ImmutableEffectDefinitionPtr definition)
{
    assert(definition);

    // 创建 CBuffer 实例
    for (const auto& symbol : definition->GetSymbolList())
    {
        if (symbol.second.Type == GraphDef::ShaderDefinition::SymbolTypes::Uniform)
        {
            auto cBufferDef = std::get<GraphDef::EffectDefinition::UniformSymbolInfo>(symbol.second.AssocInfo).Definition;
            if (m_stCBufferInstances.find(cBufferDef.get()) == m_stCBufferInstances.end())
            {
                auto cBuffer = make_shared<ConstantBuffer>(device, cBufferDef, ConstantBuffer::Usage::Default);
                m_stCBufferInstances.emplace(cBufferDef.get(), std::move(cBuffer));
            }
        }
    }

    // TODO: 创建 Texture 绑定

    // 创建 Pass 实例
    for (const auto& passGroup : definition->GetGroups())
    {
        for (const auto& pass : passGroup->GetPasses())
        {
            assert(m_stPassInstances.find(pass.get()) == m_stPassInstances.end());

            // 获取 PipelineResourceSignature
            auto prs = pass->m_pResourceSignature;
            assert(prs);

            // 构造 ResourceBinding
            Diligent::RefCntAutoPtr<Diligent::IShaderResourceBinding> binding;
            prs->CreateShaderResourceBinding(&binding, true);
            if (!binding)
            {
                LSTG_LOG_ERROR_CAT(Material, "Create SRB fail, pass {}:{}", passGroup->GetName(), pass->GetName());
                throw system_error(make_error_code(GraphDef::DefinitionError::CreateSRBError));
            }

            // 绑定 CBuffer
            const GraphDef::ShaderDefinition* shaders[]{ pass->GetVertexShader().get(), pass->GetPixelShader().get() };
            for (auto shader: shaders)
            {
                // 全局 CBuffer 在 EffectFactory 完成静态绑定
                // 这里只绑定 Material 级别的 CBuffer
                for (const auto& cb: shader->GetConstantBuffers())
                {
                    auto shaderStage = shader->GetType() == GraphDef::ShaderDefinition::ShaderTypes::VertexShader ?
                        Diligent::SHADER_TYPE_VERTEX : Diligent::SHADER_TYPE_PIXEL;

                    Diligent::IShaderResourceVariable* shaderVariable = binding->GetVariableByName(shaderStage, cb->GetName().c_str());
                    assert(shaderVariable);

                    auto cBufferInstance = m_stCBufferInstances.find(cb.get());
                    assert(cBufferInstance != m_stCBufferInstances.end());

                    shaderVariable->Set(cBufferInstance->second->m_pNativeHandler);
                }
            }

            // TODO: 绑定 Texture

            // 创建实例
            PassInstance inst;
            inst.ResourceBinding = binding;
            inst.ResourceBinding->AddRef();
            m_stPassInstances.emplace(pass.get(), std::move(inst));
        }
    }

    m_pDefinition = std::move(definition);
}

Result<void> Material::Commit(const GraphDef::EffectPassGroupDefinition* group) noexcept
{
    assert(group);

    for (const auto& pass : group->GetPasses())
    {
        const GraphDef::ShaderDefinition* shaders[]{ pass->GetVertexShader().get(), pass->GetPixelShader().get() };
        for (auto shader: shaders)
        {
            // NOTE: GlobalConstantBuffer 由 Renderer 进行提交

            // 提交 ConstantBuffer
            for (const auto& cBufferDef : shader->GetConstantBuffers())
            {
                auto it = m_stCBufferInstances.find(cBufferDef.get());
                assert(it != m_stCBufferInstances.end());
                auto ret = it->second->Commit();
                if (!ret)
                {
                    LSTG_LOG_ERROR_CAT(Material, "Commit constant buffer {} fail: {}", cBufferDef->GetName(), ret.GetError());
                    return ret.GetError();
                }
            }

            // TODO: 提交 Texture ?
        }
    }
    return {};
}

// </editor-fold>
