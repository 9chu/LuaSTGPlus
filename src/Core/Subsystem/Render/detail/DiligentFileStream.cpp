/**
 * @file
 * @date 2022/4/7
 * @author 9chu
 * 此文件为 LuaSTGPlus 项目的一部分，版权与许可声明详见 COPYRIGHT.txt。
 */
#include "DiligentFileStream.hpp"

using namespace std;
using namespace lstg;
using namespace lstg::Subsystem::Render::detail;

Diligent::RefCntAutoPtr<DiligentFileStream> DiligentFileStream::Create(Subsystem::VFS::StreamPtr stream)
{
    return Diligent::RefCntAutoPtr<DiligentFileStream> { Diligent::MakeNewRCObj<DiligentFileStream>()(std::move(stream)) };
}

DiligentFileStream::DiligentFileStream(Diligent::IReferenceCounters* refCounters, Subsystem::VFS::StreamPtr stream)
    : Diligent::ObjectBase<Diligent::IFileStream>(refCounters), m_pStream(std::move(stream))
{}

void DiligentFileStream::QueryInterface(const Diligent::INTERFACE_ID& iid, IObject** interface)
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

bool DiligentFileStream::Read(void* data, size_t bufferSize)
{
    assert(m_pStream);
    auto ret = m_pStream->Read(reinterpret_cast<uint8_t*>(data), bufferSize);
    return !!ret;
}

void DiligentFileStream::ReadBlob(Diligent::IDataBlob* data)
{
    assert(m_pStream);
    assert(data);
    data->Resize(m_pStream->GetLength());
    m_pStream->Read(reinterpret_cast<uint8_t*>(data->GetDataPtr()), data->GetSize());
}

bool DiligentFileStream::Write(const void* data, size_t size)
{
    assert(m_pStream);
    auto ret = m_pStream->Write(reinterpret_cast<const uint8_t*>(data), size);
    return !!ret;
}

size_t DiligentFileStream::GetSize()
{
    assert(m_pStream);
    return m_pStream->GetLength();
}

bool DiligentFileStream::IsValid()
{
    return !!m_pStream;
}
