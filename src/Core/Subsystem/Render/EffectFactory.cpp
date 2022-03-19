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
#include <lstg/Core/Subsystem/VFS/IStream.hpp>
#include "Effect/detail/LuaBridge/BuilderModule.hpp"

using namespace std;
using namespace lstg;
using namespace lstg::Subsystem::Render;

LSTG_DEF_LOG_CATEGORY(EffectFactory);

namespace lstg::Subsystem::Render::detail
{
    /**
     * Diligent::IFileStream 实现
     * 对接到 VirtualFileSystem 上。
     */
    class DiligentFileStream :
        public Diligent::ObjectBase<Diligent::IFileStream>
    {
    public:
        static Diligent::RefCntAutoPtr<DiligentFileStream> Create(Subsystem::VFS::StreamPtr&& stream)
        {
            return Diligent::RefCntAutoPtr<DiligentFileStream> {Diligent::MakeNewRCObj<DiligentFileStream>()(stream)};
        }

    public:
        DiligentFileStream(Diligent::IReferenceCounters* refCounters, Subsystem::VFS::StreamPtr stream)
            : Diligent::ObjectBase<Diligent::IFileStream>(refCounters), m_pStream(std::move(stream)) {}

    public:
        void QueryInterface(const Diligent::INTERFACE_ID& iid, IObject** interface) override
        {
            if (!interface)
                return;

            if (iid == Diligent::IID_FileStream)
            {
                *interface = this;
                (*interface)->AddRef();
            }
            else
            {
                Diligent::ObjectBase<Diligent::IFileStream>::QueryInterface(iid, interface);
            }
        }

        bool Read(void* data, size_t bufferSize) override
        {
            assert(m_pStream);
            auto ret = m_pStream->Read(reinterpret_cast<uint8_t*>(data), bufferSize);
            return !!ret;
        }

        void ReadBlob(Diligent::IDataBlob* data) override
        {
            assert(m_pStream);
            assert(data);
            data->Resize(m_pStream->GetLength());
            m_pStream->Read(reinterpret_cast<uint8_t*>(data->GetDataPtr()), data->GetSize());
        }

        bool Write(const void* data, size_t size) override
        {
            assert(m_pStream);
            auto ret = m_pStream->Write(reinterpret_cast<const uint8_t*>(data), size);
            return !!ret;
        }

        size_t GetSize() override
        {
            assert(m_pStream);
            return m_pStream->GetLength();
        }

        bool IsValid() override
        {
            return !!m_pStream;
        }

    private:
        Subsystem::VFS::StreamPtr m_pStream;
    };

    /**
     * Diligent::IShaderSourceInputStreamFactory 实现
     * 用于在 VirtualFileSystem 上打开流。
     */
    class ShaderSourceInputStreamFactory :
        public Diligent::ObjectBase<Diligent::IShaderSourceInputStreamFactory>
    {
    public:
        static Diligent::RefCntAutoPtr<ShaderSourceInputStreamFactory> Create(Subsystem::VirtualFileSystem* vfs, std::string& baseDirectory)
        {
            return Diligent::RefCntAutoPtr<ShaderSourceInputStreamFactory> {
                Diligent::MakeNewRCObj<ShaderSourceInputStreamFactory>()(vfs, baseDirectory)
            };
        }

    public:
        ShaderSourceInputStreamFactory(Diligent::IReferenceCounters* refCounters, Subsystem::VirtualFileSystem* vfs,
            std::string& baseDirectory)
            : Diligent::ObjectBase<Diligent::IShaderSourceInputStreamFactory>(refCounters), m_pFileSystem(vfs),
            m_stBaseDirectory(baseDirectory) {}

    public:
        void QueryInterface(const Diligent::INTERFACE_ID& iid, IObject** interface) override
        {
            if (!interface)
                return;

            if (iid == Diligent::IID_IShaderSourceInputStreamFactory)
            {
                *interface = this;
                (*interface)->AddRef();
            }
            else
            {
                Diligent::ObjectBase<Diligent::IShaderSourceInputStreamFactory>::QueryInterface(iid, interface);
            }
        }

        void CreateInputStream(const char* name, Diligent::IFileStream** stream) override
        {
            CreateInputStream2(name, Diligent::CREATE_SHADER_SOURCE_INPUT_STREAM_FLAG_NONE, stream);
        }

        void CreateInputStream2(const char* name, Diligent::CREATE_SHADER_SOURCE_INPUT_STREAM_FLAGS flags,
            Diligent::IFileStream** stream) override
        {
            *stream = nullptr;

            auto fullPath = fmt::format("{}/{}", m_stBaseDirectory, name);
            auto ret = m_pFileSystem->OpenFile(fullPath, VFS::FileAccessMode::Read);
            if (!ret)
            {
                if (flags != Diligent::CREATE_SHADER_SOURCE_INPUT_STREAM_FLAG_SILENT)
                    LSTG_LOG_ERROR_CAT(EffectFactory, "Cannot open file \"{}\": {}", name, ret.GetError());
            }
            else
            {
                auto refStream = DiligentFileStream::Create(std::move(*ret));
                refStream->QueryInterface(Diligent::IID_FileStream, reinterpret_cast<IObject**>(stream));
            }
        }

    private:
        Subsystem::VirtualFileSystem* m_pFileSystem = nullptr;
        std::string& m_stBaseDirectory;
    };
}

EffectFactory::EffectFactory(VirtualFileSystem* vfs, RenderDevice* device)
    : m_pFileSystem(vfs), m_pRenderDevice(device)
{
    Script::LuaStack::BalanceChecker checker(m_stState);

    // 启用标准库
    m_stState.OpenStandardLibrary();

    // 注册 ScriptState 指针到全局
    m_stScriptState.Factory = this;
    lua_pushlightuserdata(m_stState, &m_stScriptState);
    lua_setfield(m_stState, LUA_REGISTRYINDEX, "_state");

    // 注册模块
    Effect::detail::LuaBridge::RenderEffectLuaBridge(m_stState);

    // 解压模块内的定义到全局环境
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

    // 构造 StreamFactory
    auto streamFactory = detail::ShaderSourceInputStreamFactory::Create(m_pFileSystem, m_stBaseDirectory);
    m_pStreamFactory = streamFactory;
    m_pStreamFactory->AddRef();
}

EffectFactory::~EffectFactory()
{
    // 释放 Diligent::IShader
    for (auto& p : m_stShaderCache)
        p.second->Release();
    m_stShaderCache.clear();

    // 释放 StreamFactory
    m_pStreamFactory->Release();
    m_pStreamFactory = nullptr;
}

Result<Effect::ImmutableEffectDefinitionPtr> EffectFactory::CreateEffect(std::string_view source) noexcept
{
    char nameBuffer[32];
    ::memset(nameBuffer, 0, sizeof(nameBuffer));
    strncpy(nameBuffer, source.data(), std::min(sizeof(nameBuffer) - 1, source.size()));

    return CreateEffect(source, nameBuffer);
}

Result<Effect::ImmutableEffectDefinitionPtr> EffectFactory::CreateEffectFromFile(std::string_view path) noexcept
{
    char nameBuffer[64];
    ::memset(nameBuffer, 0, sizeof(nameBuffer));
    nameBuffer[0] = '@';
    strncpy(nameBuffer + 1, path.data(), std::min(sizeof(nameBuffer) - 2, path.size()));

    // 加载文件
    vector<uint8_t> content;
    auto open = m_pFileSystem->ReadFile(content, fmt::format("{}/{}", m_stBaseDirectory, path));
    if (!open)
    {
        LSTG_LOG_ERROR_CAT(EffectFactory, "Cannot read file from \"%s\"", path);
        return open.GetError();
    }

    return CreateEffect({reinterpret_cast<const char*>(content.data()), content.size()}, nameBuffer);
}

Effect::ImmutableConstantBufferDefinitionPtr EffectFactory::DefineGlobalConstantBuffer(const Effect::ConstantBufferDefinition& def)
{
    assert(def.GetScope() == Effect::ConstantBufferDefinition::Scope::Global);

    auto it = m_stGlobalCBuffers.find(def.GetName());
    if (it != m_stGlobalCBuffers.end())
    {
        assert(def == (*it->second));
        return it->second;
    }

    auto immutable = make_shared<const Effect::ConstantBufferDefinition>(def);
    auto ret = m_stGlobalCBuffers.emplace(def.GetName(), immutable);
    assert(ret.first->second == immutable);
    return ret.first->second;
}

Effect::ImmutableConstantBufferDefinitionPtr EffectFactory::GetGlobalConstantBuffer(std::string_view name) const noexcept
{
    auto it = m_stGlobalCBuffers.find(name);
    if (it == m_stGlobalCBuffers.end())
        return nullptr;
    return it->second;
}

std::tuple<Effect::ImmutableShaderDefinitionPtr, std::string> EffectFactory::CompileShader(const Effect::ShaderDefinition& def)
{
    auto clone = make_shared<Effect::ShaderDefinition>(def);

    // 代换 Vertex Layout，保证相同的 Vertex Layout 全局唯一
    if (clone->GetType() == Effect::ShaderDefinition::ShaderTypes::VertexShader)
    {
        if (clone->GetVertexLayout() == nullptr)
            return { nullptr, "vertex layout is not defined" };

        auto it = m_stVertexLayoutDefCache.find(clone->GetVertexLayout());
        if (it == m_stVertexLayoutDefCache.end())
        {
            LSTG_LOG_TRACE_CAT(EffectFactory, "Cache new vertex layout #{}", clone->GetVertexLayout()->GetHashCode());
            m_stVertexLayoutDefCache.emplace(clone->GetVertexLayout());
        }
        else
        {
            LSTG_LOG_TRACE_CAT(EffectFactory, "Vertex layout replaced: {} -> {}", static_cast<const void*>(clone->GetVertexLayout().get()),
                static_cast<const void*>(it->get()));
            clone->SetVertexLayout(*it);
        }
    }

    // 检查是否已经定义了，如果定义了就直接返回
    // 当然，因为缓存只跟源码相关，如果依赖的文件发生变化我们并不能及时发现
    {
        auto it = m_stShaderDefCache.find(clone);
        if (it != m_stShaderDefCache.end())
            return { *it, "" };
    }

    // 生成 Shader 代码准备编译
    string source;
    {
        source.append("// Constant buffers\n");
        const auto& cbList = clone->GetConstantBufferReferences();
        for (const auto& cb : cbList)
            cb->AppendToCode(source);

        source.append("\n// Texture resources\n");
        const auto& texList = clone->GetTextureReferences();
        for (const auto& tex : texList)
            tex->AppendToCode(source);

        source.append("\n// Original code\n#line 1\n");
        source.append(clone->GetSource());
    }

    // 尝试编译 Shader
    auto device = m_pRenderDevice->GetDevice();
    Diligent::RefCntAutoPtr<Diligent::IShader> shaderOutput;
    {
        Diligent::RefCntAutoPtr<Diligent::IDataBlob> compilerOutputBlob;

        Diligent::ShaderCreateInfo ci;
        ci.pShaderSourceStreamFactory = m_pStreamFactory;
        ci.Source = source.c_str();
        ci.SourceLength = source.length();
        ci.EntryPoint = clone->GetEntry().c_str();
        ci.UseCombinedTextureSamplers = true;
        ci.CombinedSamplerSuffix = "Sampler";
        ci.Desc.Name = clone->GetName().c_str();
        ci.Desc.ShaderType = (clone->GetType() == Effect::ShaderDefinition::ShaderTypes::VertexShader ?
            Diligent::SHADER_TYPE_VERTEX : Diligent::SHADER_TYPE_PIXEL);
        ci.SourceLanguage = Diligent::SHADER_SOURCE_LANGUAGE_HLSL;
        ci.ppCompilerOutput = &compilerOutputBlob;
        device->CreateShader(ci, &shaderOutput);

        string_view compilerOutput = compilerOutputBlob ?
            reinterpret_cast<const char*>(compilerOutputBlob->GetConstDataPtr()) : "";
        string_view generatedSource = compilerOutputBlob ?
            reinterpret_cast<const char*>(compilerOutputBlob->GetConstDataPtr()) + compilerOutput.size() + 1 : source.c_str();
        LSTG_LOG_TRACE_CAT(EffectFactory, "Shader source code for \"{}\": {}", ci.Desc.Name, generatedSource);
        if (!shaderOutput)
        {
            LSTG_LOG_TRACE_CAT(EffectFactory, "Compile shader \"{}\" fail: {}", ci.Desc.Name, compilerOutput);
            return { nullptr, string{compilerOutput.empty() ? "unknown error, see log for more details" : compilerOutput} };
        }
        else if (!compilerOutput.empty())
        {
            LSTG_LOG_WARN_CAT(EffectFactory, "Compile shader \"{}\": {}", ci.Desc.Name, compilerOutput);
        }
    }

    LSTG_LOG_TRACE_CAT(EffectFactory, "Shader \"{}\" created", clone->GetName());

    m_stShaderDefCache.emplace(clone);
    try
    {
        m_stShaderCache.emplace(clone, shaderOutput);
        shaderOutput->AddRef();
    }
    catch (...)  // 保持内存分配失败情况下的一致性
    {
        m_stShaderDefCache.erase(clone);
        throw;
    }
    return { clone, "" };
}

Diligent::IShader* EffectFactory::GetShaderCache(const Effect::ImmutableShaderDefinitionPtr& def) const noexcept
{
    auto it = m_stShaderCache.find(def);
    if (it == m_stShaderCache.end())
        return nullptr;
    return it->second;
}

Result<Effect::ImmutableEffectDefinitionPtr> EffectFactory::CreateEffect(std::string_view source, const char* chunkName) noexcept
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
    m_stScriptState.SymbolCache.clear();

    // 执行
    ret = m_stState.ProtectedCallWithTraceback(0, 1);
    if (!ret)
    {
        LSTG_LOG_ERROR_CAT(EffectFactory, "Execute effect script fail: {}", lua_tostring(m_stState, -1));
        lua_pop(m_stState, 1);
        return ret.GetError();
    }

    // 转换到 Effect 对象
    Result<Effect::detail::LuaBridge::EffectWrapper*> wrapper = nullptr;
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
