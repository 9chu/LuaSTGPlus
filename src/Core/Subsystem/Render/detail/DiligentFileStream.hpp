/**
 * @file
 * @date 2022/4/7
 * @author 9chu
 * 此文件为 LuaSTGPlus 项目的一部分，版权与许可声明详见 COPYRIGHT.txt。
 */
#pragma once
#include <ObjectBase.hpp>
#include <RefCntAutoPtr.hpp>
#include <FileStream.h>
#include <lstg/Core/Subsystem/VFS/IStream.hpp>

namespace lstg::Subsystem::Render::detail
{
    /**
     * Diligent FileStream -> IStream 桥接
     */
    class DiligentFileStream :
        public Diligent::ObjectBase<Diligent::IFileStream>
    {
    public:
        static Diligent::RefCntAutoPtr<DiligentFileStream> Create(Subsystem::VFS::StreamPtr stream);

    public:
        DiligentFileStream(Diligent::IReferenceCounters* refCounters, Subsystem::VFS::StreamPtr stream);

    public:
        void DILIGENT_CALL_TYPE QueryInterface(const Diligent::INTERFACE_ID& iid, IObject** interface) override;
        bool DILIGENT_CALL_TYPE Read(void* data, size_t bufferSize) override;
        void DILIGENT_CALL_TYPE ReadBlob(Diligent::IDataBlob* data) override;
        bool DILIGENT_CALL_TYPE Write(const void* data, size_t size) override;
        size_t DILIGENT_CALL_TYPE GetSize() override;
        bool DILIGENT_CALL_TYPE IsValid() override;

    private:
        Subsystem::VFS::StreamPtr m_pStream;
    };
}
