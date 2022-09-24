/**
 * @file
 * @date 2022/4/7
 * @author 9chu
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#pragma once
#include <Shader.h>
#include <lstg/Core/Subsystem/VirtualFileSystem.hpp>
#include "DiligentFileStream.hpp"

namespace lstg::Subsystem::Render::detail
{
    /**
     * Diligent::ShaderSourceInputStreamFactory 实现
     */
    class DiligentShaderSourceInputStreamFactory :
        public Diligent::ObjectBase<Diligent::IShaderSourceInputStreamFactory>
    {
    public:
        static Diligent::RefCntAutoPtr<DiligentShaderSourceInputStreamFactory> Create(Subsystem::VirtualFileSystem& vfs);

    public:
        DiligentShaderSourceInputStreamFactory(Diligent::IReferenceCounters* refCounters, Subsystem::VirtualFileSystem& vfs);

    public:
        /**
         * 设置包含文件搜索基准路径
         */
        void SetFileSearchBase(std::string_view base) { m_stFileSearchBase = base; }

    public:  // Diligent::IShaderSourceInputStreamFactory
        void QueryInterface(const Diligent::INTERFACE_ID& iid, IObject** interface) override;
        void CreateInputStream(const char* name, Diligent::IFileStream** stream) override;
        void CreateInputStream2(const char* name, Diligent::CREATE_SHADER_SOURCE_INPUT_STREAM_FLAGS flags,
            Diligent::IFileStream** stream) override;

    private:
        Subsystem::VirtualFileSystem& m_stFileSystem;
        std::string m_stFileSearchBase;
    };
}
