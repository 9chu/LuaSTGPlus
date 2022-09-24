/**
 * @file
 * @date 2022/4/7
 * @author 9chu
 * 此文件为 LuaSTGPlus 项目的一部分，版权与许可声明详见 COPYRIGHT.txt。
 */
#include "DiligentShaderSourceInputStreamFactory.hpp"

#include <fmt/format.h>
#include <lstg/Core/Logging.hpp>

using namespace std;
using namespace lstg;
using namespace lstg::Subsystem::Render::detail;

LSTG_DEF_LOG_CATEGORY(DiligentShaderSourceInputStreamFactory);

Diligent::RefCntAutoPtr<DiligentShaderSourceInputStreamFactory> DiligentShaderSourceInputStreamFactory::Create(
    Subsystem::VirtualFileSystem& vfs)
{
    return Diligent::RefCntAutoPtr<DiligentShaderSourceInputStreamFactory> {
        Diligent::MakeNewRCObj<DiligentShaderSourceInputStreamFactory>()(vfs)
    };
}

DiligentShaderSourceInputStreamFactory::DiligentShaderSourceInputStreamFactory(Diligent::IReferenceCounters* refCounters,
    Subsystem::VirtualFileSystem& vfs)
    : Diligent::ObjectBase<Diligent::IShaderSourceInputStreamFactory>(refCounters), m_stFileSystem(vfs)
{}

void DiligentShaderSourceInputStreamFactory::QueryInterface(const Diligent::INTERFACE_ID& iid, IObject** interface)
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

void DiligentShaderSourceInputStreamFactory::CreateInputStream(const char* name, Diligent::IFileStream** stream)
{
    CreateInputStream2(name, Diligent::CREATE_SHADER_SOURCE_INPUT_STREAM_FLAG_NONE, stream);
}

void DiligentShaderSourceInputStreamFactory::CreateInputStream2(const char* name, Diligent::CREATE_SHADER_SOURCE_INPUT_STREAM_FLAGS flags,
    Diligent::IFileStream** stream)
{
    *stream = nullptr;

    string fullPath;
    if (name[0] == '/')
        fullPath = fmt::format("{}{}", m_stFileSystem.GetAssetBaseDirectory(), name);
    else
        fullPath = fmt::format("{}/{}/{}", m_stFileSystem.GetAssetBaseDirectory(), m_stFileSearchBase, name);
    auto ret = m_stFileSystem.OpenFile(fullPath, VFS::FileAccessMode::Read);
    if (!ret)
    {
        if (flags != Diligent::CREATE_SHADER_SOURCE_INPUT_STREAM_FLAG_SILENT)
            LSTG_LOG_ERROR_CAT(DiligentShaderSourceInputStreamFactory, "Cannot open file \"{}\": {}", name, ret.GetError());
    }
    else
    {
        auto refStream = DiligentFileStream::Create(std::move(*ret));
        refStream->QueryInterface(Diligent::IID_FileStream, reinterpret_cast<IObject**>(stream));
    }
}
