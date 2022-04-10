/**
 * @file
 * @date 2022/3/15
 * @author 9chu
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#include <lstg/Core/Subsystem/Render/EffectFactory.hpp>

#include <RenderDevice.h>
#include <ObjectBase.hpp>
#include <RefCntAutoPtr.hpp>
#include <DataBlobImpl.hpp>
#include <lstg/Core/Logging.hpp>
#include <lstg/Core/Subsystem/Script/LuaRead.hpp>
#include <lstg/Core/Subsystem/Render/GraphicsDefinitionCache.hpp>
#include "detail/DiligentShaderSourceInputStreamFactory.hpp"
#include "detail/LuaEffectBuilder/BuilderModule.hpp"
#include "GraphDef/detail/ToDiligent.hpp"

using namespace std;
using namespace lstg;
using namespace lstg::Subsystem::Render;

LSTG_DEF_LOG_CATEGORY(EffectFactory);

EffectFactory::EffectFactory(VirtualFileSystem& vfs, RenderDevice& device)
    : m_stFileSystem(vfs), m_stRenderDevice(device)
{
    // 构造脚本状态对象
    auto scriptState = make_unique<detail::LuaEffectBuilder::BuilderGlobalState>();

    // 构造 StreamFactory
    auto streamFactory = detail::DiligentShaderSourceInputStreamFactory::Create(m_stFileSystem);

    // 初始化 lua 虚拟机状态
    {
        Script::LuaStack::BalanceChecker checker(m_stState);

        // 启用标准库
        m_stState.OpenStandardLibrary();

        // 注册 State
        scriptState->Factory = this;
        scriptState->RegisterToLua(m_stState);

        // 注册模块
        detail::LuaEffectBuilder::InitModule(m_stState);

        // 设置模块内的定义到全局环境
        lua_getglobal(m_stState, "Builder");  // t
        assert(lua_type(m_stState, -1) == LUA_TTABLE);
        lua_pushnil(m_stState);
        while (lua_next(m_stState, -2))
        {
            const char* name = lua_tostring(m_stState, -2);
            lua_setglobal(m_stState, name);
        }
        lua_pop(m_stState, 1);

        // 删除全局 Builder 表
        lua_pushnil(m_stState);
        lua_setglobal(m_stState, "Builder");
    }

    // 持有指针
    m_pScriptState = scriptState.release();
    m_pStreamFactory = streamFactory;
    m_pStreamFactory->AddRef();
}

EffectFactory::~EffectFactory()
{
    // 释放 ScriptState
    if (m_pScriptState)
    {
        delete m_pScriptState;
        m_pScriptState = nullptr;
    }

    // 释放 StreamFactory
    if (m_pStreamFactory)
    {
        m_pStreamFactory->Release();
        m_pStreamFactory = nullptr;
    }
}

Result<GraphDef::ImmutableEffectDefinitionPtr> EffectFactory::CreateEffect(std::string_view source) noexcept
{
    char nameBuffer[32];
    ::memset(nameBuffer, 0, sizeof(nameBuffer));
    strncpy(nameBuffer, source.data(), std::min(sizeof(nameBuffer) - 1, source.size()));

    return CreateEffect(source, nameBuffer);
}

Result<GraphDef::ImmutableEffectDefinitionPtr> EffectFactory::CreateEffectFromFile(std::string_view path) noexcept
{
    char nameBuffer[64];
    ::memset(nameBuffer, 0, sizeof(nameBuffer));
    nameBuffer[0] = '@';
    strncpy(nameBuffer + 1, path.data(), std::min(sizeof(nameBuffer) - 2, path.size()));

    // 加载文件
    vector<uint8_t> content;
    auto open = m_stFileSystem.ReadFile(content, fmt::format("{}/{}", m_stFileSystem.GetAssetBaseDirectory(), path));
    if (!open)
    {
        LSTG_LOG_ERROR_CAT(EffectFactory, "Cannot read file from \"{}\"", path);
        return open.GetError();
    }

    return CreateEffect({reinterpret_cast<const char*>(content.data()), content.size()}, nameBuffer);
}

Result<void> EffectFactory::RegisterGlobalConstantBuffer(const ConstantBufferPtr& buf) noexcept
{
    if (!buf)
        return make_error_code(errc::invalid_argument);

    const auto& def = buf->GetDefinition();
    assert(def);
    if (m_stGlobalCBuffers.find(def->GetName()) != m_stGlobalCBuffers.end())
        return make_error_code(GraphDef::DefinitionError::SymbolAlreadyDefined);

    try
    {
        m_stGlobalCBuffers.emplace(def->GetName(), buf);
        return {};
    }
    catch (...)
    {
        return make_error_code(errc::not_enough_memory);
    }
}

ConstantBufferPtr EffectFactory::GetGlobalConstantBuffer(std::string_view name) const noexcept
{
    auto it = m_stGlobalCBuffers.find(name);
    if (it == m_stGlobalCBuffers.end())
        return nullptr;
    return it->second;
}

Result<GraphDef::ImmutableEffectDefinitionPtr> EffectFactory::CreateEffect(std::string_view source, const char* chunkName) noexcept
{
    Script::LuaStack::BalanceChecker checker(m_stState);

    // 编译脚本
    auto ret = m_stState.LoadBuffer(Span<const uint8_t>(reinterpret_cast<const uint8_t*>(source.data()), source.size()), chunkName);
    if (!ret)
    {
        LSTG_LOG_ERROR_CAT(EffectFactory, "Compile effect script fail: {}", lua_tostring(m_stState, -1));
        lua_pop(m_stState, 1);
        return ret.GetError();
    }

    // 设置脚本状态
    assert(m_pScriptState);
    m_pScriptState->SymbolCache.clear();

    // 执行
    ret = m_stState.ProtectedCallWithTraceback(0, 1);
    if (!ret)
    {
        LSTG_LOG_ERROR_CAT(EffectFactory, "Execute effect script fail: {}", lua_tostring(m_stState, -1));
        lua_pop(m_stState, 1);
        return ret.GetError();
    }

    // 转换到 Effect 对象
    Result<detail::LuaEffectBuilder::EffectWrapper*> wrapper = nullptr;
    m_stState.ReadValue(-1, wrapper);
    if (!wrapper)
    {
        LSTG_LOG_ERROR_CAT(EffectFactory, "Script not returns effect object");
        lua_pop(m_stState, 1);
        return make_error_code(errc::invalid_argument);
    }
    auto ptr = (*wrapper)->Get();
    lua_pop(m_stState, 1);
    return ptr;
}

Result<GraphDef::ImmutableShaderDefinitionPtr> EffectFactory::CompileShader(const GraphDef::ShaderDefinition& def) noexcept
{
    try
    {
        auto device = m_stRenderDevice.GetDevice();
        auto ret = make_shared<GraphDef::ShaderDefinition>(def);

        // 生成 Shader 代码准备编译
        string source;
        {
            source.append("// External constant buffers\n");
            const auto& globalCBufferList = ret->GetGlobalConstantBuffers();
            for (const auto& cb : globalCBufferList)
                cb->AppendToCode(source);

            source.append("// Constant buffers\n");
            const auto& cBufferList = ret->GetConstantBuffers();
            for (const auto& cb : cBufferList)
                cb->AppendToCode(source);

            source.append("\n// Texture resources\n");
            const auto& texList = ret->GetTextures();
            for (const auto& tex : texList)
                tex->AppendToCode(source);

            source.append("\n// Original code\n#line 1\n");
            source.append(ret->GetSource());
        }

        // 尝试编译 Shader
        Diligent::RefCntAutoPtr<Diligent::IShader> shaderOutput;
        {
            Diligent::RefCntAutoPtr<Diligent::IDataBlob> compilerOutputBlob;

            Diligent::ShaderCreateInfo ci;
            ci.pShaderSourceStreamFactory = m_pStreamFactory;
            ci.Source = source.c_str();
            ci.SourceLength = source.length();
            ci.EntryPoint = ret->GetEntry().c_str();
            ci.UseCombinedTextureSamplers = true;
            ci.CombinedSamplerSuffix = "Sampler";
            ci.Desc.Name = ret->GetName().c_str();
            ci.Desc.ShaderType = (ret->GetType() == GraphDef::ShaderDefinition::ShaderTypes::VertexShader ?
                Diligent::SHADER_TYPE_VERTEX : Diligent::SHADER_TYPE_PIXEL);
            ci.SourceLanguage = Diligent::SHADER_SOURCE_LANGUAGE_HLSL;
            ci.ppCompilerOutput = &compilerOutputBlob;
            device->CreateShader(ci, &shaderOutput);

            string_view compilerOutput = compilerOutputBlob ? reinterpret_cast<const char*>(compilerOutputBlob->GetConstDataPtr()) : "";
            string_view generatedSource = compilerOutputBlob ?
                reinterpret_cast<const char*>(compilerOutputBlob->GetConstDataPtr()) + compilerOutput.size() + 1 : source.c_str();
            LSTG_LOG_TRACE_CAT(EffectFactory, "Shader source code for \"{}\": {}", ci.Desc.Name, generatedSource);
            if (!shaderOutput)
            {
                LSTG_LOG_TRACE_CAT(EffectFactory, "Compile shader \"{}\" fail: {}", ci.Desc.Name, compilerOutput);
                return make_error_code(GraphDef::DefinitionError::ShaderCompileError);
            }
            else if (!compilerOutput.empty())
            {
                LSTG_LOG_WARN_CAT(EffectFactory, "Compile shader \"{}\": {}", ci.Desc.Name, compilerOutput);
            }
        }
        LSTG_LOG_TRACE_CAT(EffectFactory, "Shader \"{}\" created", ret->GetName());

        ret->m_pCompiledShader = shaderOutput;
        ret->m_pCompiledShader->AddRef();
        return ret;
    }
    catch (...)  // bad_alloc
    {
        return make_error_code(errc::not_enough_memory);
    }
}

Result<GraphDef::ImmutableEffectPassDefinitionPtr> EffectFactory::CompilePass(const GraphDef::EffectPassDefinition& def) noexcept
{
    // 参数检查
    if (!def.GetVertexShader())
        return make_error_code(errc::invalid_argument);
    if (!def.GetPixelShader())
        return make_error_code(errc::invalid_argument);

    try
    {
        auto device = m_stRenderDevice.GetDevice();
        auto ret = make_shared<GraphDef::EffectPassDefinition>(def);

        // 根据定义生成 PRS
        Diligent::RefCntAutoPtr<Diligent::IPipelineResourceSignature> prs;
        {
            vector<Diligent::PipelineResourceDesc> resourceList;
            vector<Diligent::ImmutableSamplerDesc> samplerList;

            // 收集所有的 Shader Resources
            const GraphDef::ShaderDefinition* shaders[2] = {ret->GetVertexShader().get(), ret->GetPixelShader().get()};
            for (const auto& shader : shaders)
            {
                auto shaderStage = shader->GetType() == GraphDef::ShaderDefinition::ShaderTypes::VertexShader ?
                    Diligent::SHADER_TYPE_VERTEX : Diligent::SHADER_TYPE_PIXEL;

                // 声明全局 CBuffer
                for (const auto& ref : shader->GetGlobalConstantBuffers())
                {
                    resourceList.emplace_back(Diligent::PipelineResourceDesc {
                        shaderStage,
                        ref->GetName().c_str(),
                        1,
                        Diligent::SHADER_RESOURCE_TYPE_CONSTANT_BUFFER,
                        Diligent::SHADER_RESOURCE_VARIABLE_TYPE_STATIC,
                    });
                }

                // 声明可变 CBuffer
                for (const auto& ref : shader->GetConstantBuffers())
                {
                    resourceList.emplace_back(Diligent::PipelineResourceDesc {
                        shaderStage,
                        ref->GetName().c_str(),
                        1,
                        Diligent::SHADER_RESOURCE_TYPE_CONSTANT_BUFFER,
                        Diligent::SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE,  // 实例化 SRB 时固化
                    });
                }

                // 声明 Texture
                for (const auto& ref : shader->GetTextures())
                {
                    // 定义 Sampler，总是 immutable 的
                    Diligent::SamplerDesc samplerDesc = GraphDef::detail::ToDiligent(ref->GetSamplerDesc());
                    samplerList.emplace_back(Diligent::ImmutableSamplerDesc {
                        shaderStage,
                        ref->GetName().c_str(),
                        samplerDesc
                    });

                    // Texture 总是 dynamic 的，可以反复修改
                    resourceList.emplace_back(Diligent::PipelineResourceDesc {
                        shaderStage,
                        ref->GetName().c_str(),
                        1,
                        Diligent::SHADER_RESOURCE_TYPE_TEXTURE_SRV,
                        Diligent::SHADER_RESOURCE_VARIABLE_TYPE_DYNAMIC,  // 允许反复设置
                    });
                }
            }

            // 构造 PRS
            Diligent::PipelineResourceSignatureDesc desc;
            desc.Resources = resourceList.data();
            desc.NumResources = resourceList.size();
            desc.ImmutableSamplers = samplerList.data();
            desc.NumImmutableSamplers = samplerList.size();
            desc.UseCombinedTextureSamplers = true;
            desc.CombinedSamplerSuffix = "Sampler";
            device->CreatePipelineResourceSignature(desc, &prs);
            if (!prs)
                return make_error_code(GraphDef::DefinitionError::CreatePRSError);

            // 绑定静态资源
            for (const auto& shader : shaders)
            {
                auto shaderStage = shader->GetType() == GraphDef::ShaderDefinition::ShaderTypes::VertexShader ?
                    Diligent::SHADER_TYPE_VERTEX : Diligent::SHADER_TYPE_PIXEL;

                for (const auto& ref : shader->GetGlobalConstantBuffers())
                {
                    auto variable = prs->GetStaticVariableByName(shaderStage, ref->GetName().c_str());
                    assert(variable);

                    auto cBuffer = GetGlobalConstantBuffer(ref->GetName().c_str());
                    assert(cBuffer);
                    variable->Set(cBuffer->m_pNativeHandler);
                }
            }
        }
        LSTG_LOG_TRACE_CAT(EffectFactory, "Pass \"{}\" created", ret->GetName());

        ret->m_pResourceSignature = prs;
        ret->m_pResourceSignature->AddRef();
        return ret;
    }
    catch (...)  // bad_alloc
    {
        return make_error_code(errc::not_enough_memory);
    }
}
