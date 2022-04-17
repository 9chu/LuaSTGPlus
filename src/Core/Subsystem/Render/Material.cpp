/**
 * @file
 * @date 2022/3/20
 * @author 9chu
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#include <lstg/Core/Subsystem/Render/Material.hpp>

#include <Buffer.h>
#include <Texture.h>
#include <ShaderResourceBinding.h>
#include <PipelineResourceSignature.h>
#include <DeviceContext.h>
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

Material::Material(RenderDevice& device, GraphDef::ImmutableEffectDefinitionPtr definition, const TexturePtr& defaultTex2D)
    : m_stRenderDevice(device)
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
                auto shaderStage = shader->GetType() == GraphDef::ShaderDefinition::ShaderTypes::VertexShader ?
                    Diligent::SHADER_TYPE_VERTEX : Diligent::SHADER_TYPE_PIXEL;

                // 全局 CBuffer 在 EffectFactory 完成静态绑定
                // 这里只绑定 Material 级别的 CBuffer
                for (const auto& cb: shader->GetConstantBuffers())
                {
                    Diligent::IShaderResourceVariable* shaderVariable = binding->GetVariableByName(shaderStage, cb->GetName().c_str());
                    assert(shaderVariable);

                    auto cBufferInstance = m_stCBufferInstances.find(cb.get());
                    assert(cBufferInstance != m_stCBufferInstances.end());

                    shaderVariable->Set(cBufferInstance->second->m_pNativeHandler);
                }
            }

            // 创建实例
            PassInstance inst;
            inst.ResourceBinding = binding;
            inst.ResourceBinding->AddRef();
            m_stPassInstances.emplace(pass.get(), std::move(inst));
        }
    }

    // 创建 Texture 绑定，需要 PassInstance 内存稳定后创建
    for (const auto& passGroup: definition->GetGroups())
    {
        for (const auto& pass: passGroup->GetPasses())
        {
            assert(m_stPassInstances.find(pass.get()) != m_stPassInstances.end());
            PassInstance* passInstance = &m_stPassInstances[pass.get()];

            const GraphDef::ShaderDefinition* shaders[]{ pass->GetVertexShader().get(), pass->GetPixelShader().get() };
            for (auto shader: shaders)
            {
                auto shaderStage = shader->GetType() == GraphDef::ShaderDefinition::ShaderTypes::VertexShader ?
                    Diligent::SHADER_TYPE_VERTEX : Diligent::SHADER_TYPE_PIXEL;

                for (const auto& v : shader->GetTextures())
                {
                    // FIXME: 暂时不支持 2D 纹理以外的类型
                    if (v->GetType() != GraphDef::ShaderTextureDefinition::TextureTypes::Texture2D)
                    {
                        LSTG_LOG_ERROR_CAT(Material, "Texture {} with type {} is not supported yet", v->GetName(),
                            static_cast<int>(v->GetType()));
                        throw system_error(make_error_code(errc::not_supported));
                    }

                    // 查找纹理是否存在
                    auto it = m_stTextureVariableInstances.find(v.get());
                    if (it == m_stTextureVariableInstances.end())
                    {
                        auto e = m_stTextureVariableInstances.emplace(v.get(), TextureVariableState {});
                        it = e.first;
                    }
                    assert(it != m_stTextureVariableInstances.end());
                    auto jt = std::find_if(it->second.References.begin(), it->second.References.end(),
                        [passInstance](const TextureVariableRef& ref) {
                            return ref.Pass == passInstance;
                        });
                    if (jt == it->second.References.end())
                    {
                        it->second.References.emplace_back(TextureVariableRef{
                            passInstance, nullptr, nullptr
                        });
                        jt = it->second.References.end() - 1;
                    }
                    assert(jt != it->second.References.end());
                    auto var = passInstance->ResourceBinding->GetVariableByName(shaderStage, v->GetName().c_str());
                    assert(var);
                    if (shaderStage == Diligent::SHADER_TYPE_VERTEX)
                    {
                        jt->VertexShaderResource = var;
                    }
                    else
                    {
                        assert(shaderStage == Diligent::SHADER_TYPE_PIXEL);
                        jt->PixelShaderResource = var;
                    }
                }
            }
        }
    }

    // 给所有 Texture 绑定默认的纹理
    auto defaultTex2DView = defaultTex2D->m_pNativeHandler->GetDefaultView(Diligent::TEXTURE_VIEW_SHADER_RESOURCE);
    assert(defaultTex2DView);
    for (auto& p : m_stTextureVariableInstances)
    {
        assert(p.first->GetType() == GraphDef::ShaderTextureDefinition::TextureTypes::Texture2D);
        p.second.BindingTexture = defaultTex2D;
        for (auto& r : p.second.References)
        {
            if (r.VertexShaderResource)
                r.VertexShaderResource->Set(defaultTex2DView);
            if (r.PixelShaderResource)
                r.PixelShaderResource->Set(defaultTex2DView);
            r.Pass->SRBDirty = true;
        }
    }
    m_bSRBDirty = true;
    m_pDefinition = std::move(definition);
}

Result<void> Material::SetTexture(std::string_view symbol, const TexturePtr& texture) noexcept
{
    if (!texture)
        return make_error_code(errc::invalid_argument);
    auto* nativeHandler = texture->m_pNativeHandler;

    // 获取符号定义
    auto info = m_pDefinition->GetSymbol(symbol);
    if (!info)
        return make_error_code(GraphDef::DefinitionError::SymbolNotFound);
    if (info->Type != GraphDef::ShaderDefinition::SymbolTypes::Texture)
        return make_error_code(GraphDef::DefinitionError::SymbolTypeMismatched);

    // 检查类型
    const auto& assoc = std::get<GraphDef::EffectDefinition::TextureOrSamplerSymbolInfo>(info->AssocInfo);
    switch (assoc.Definition->GetType())
    {
        case GraphDef::ShaderTextureDefinition::TextureTypes::Texture1D:
            if (nativeHandler->GetDesc().Type != Diligent::RESOURCE_DIM_TEX_1D)
                return make_error_code(GraphDef::DefinitionError::SymbolTypeMismatched);
            break;
        case GraphDef::ShaderTextureDefinition::TextureTypes::Texture2D:
            if (nativeHandler->GetDesc().Type != Diligent::RESOURCE_DIM_TEX_2D)
                return make_error_code(GraphDef::DefinitionError::SymbolTypeMismatched);
            break;
        case GraphDef::ShaderTextureDefinition::TextureTypes::Texture3D:
            if (nativeHandler->GetDesc().Type != Diligent::RESOURCE_DIM_TEX_3D)
                return make_error_code(GraphDef::DefinitionError::SymbolTypeMismatched);
            break;
        case GraphDef::ShaderTextureDefinition::TextureTypes::TextureCube:
            if (nativeHandler->GetDesc().Type != Diligent::RESOURCE_DIM_TEX_CUBE)
                return make_error_code(GraphDef::DefinitionError::SymbolTypeMismatched);
            break;
        default:
            assert(false);
            return make_error_code(GraphDef::DefinitionError::SymbolTypeMismatched);
    }

    // 赋值
    auto view = nativeHandler->GetDefaultView(Diligent::TEXTURE_VIEW_SHADER_RESOURCE);
    if (!view)
    {
        LSTG_LOG_ERROR_CAT(Material, "GetDefaultView from texture {} fail", symbol);
        return make_error_code(errc::io_error);
    }
    auto it = m_stTextureVariableInstances.find(assoc.Definition.get());
    assert(it != m_stTextureVariableInstances.end());
    it->second.BindingTexture = texture;
    for (auto& r : it->second.References)
    {
        assert(r.VertexShaderResource || r.PixelShaderResource);
        if (r.VertexShaderResource)
            r.VertexShaderResource->Set(view);
        if (r.PixelShaderResource)
            r.PixelShaderResource->Set(view);
        r.Pass->SRBDirty = true;
    }
    m_bSRBDirty = true;
    return {};
}

Result<void> Material::Commit() noexcept
{
    if (m_bCBufferDirty)
    {
        for (auto& buf : m_stCBufferInstances)
        {
            auto ret = buf.second->Commit();
            if (!ret)
                return ret.GetError();
        }
        m_bCBufferDirty = false;
    }

    if (m_bSRBDirty)
    {
        for (auto& pass : m_stPassInstances)
        {
            if (pass.second.SRBDirty)
            {
                m_stRenderDevice.GetImmediateContext()->CommitShaderResources(pass.second.ResourceBinding,
                    Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
                pass.second.SRBDirty = false;
            }
        }
        m_bSRBDirty = false;
    }
    return {};
}

// </editor-fold>
