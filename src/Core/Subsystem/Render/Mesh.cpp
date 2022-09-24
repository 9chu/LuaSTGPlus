/**
 * @file
 * @date 2022/3/19
 * @author 9chu
 * 此文件为 LuaSTGPlus 项目的一部分，版权与许可声明详见 COPYRIGHT.txt。
 */
#include <lstg/Core/Subsystem/Render/Mesh.hpp>

#include <Buffer.h>
#include <MapHelper.hpp>
#include <RenderDevice.h>

using namespace std;
using namespace lstg;
using namespace lstg::Subsystem::Render;

namespace
{
    /**
     * 计算不小于 n 的最近二次幂
     */
    constexpr unsigned int NextPowerOf2(unsigned int n) noexcept
    {
        n--;
        n |= n >> 1;
        n |= n >> 2;
        n |= n >> 4;
        n |= n >> 8;
        n |= n >> 16;
        n++;
        return n;
    }
}

Mesh::Mesh(Render::RenderDevice& device, GraphDef::ImmutableMeshDefinitionPtr definition, Diligent::IBuffer* vertexBuffer,
    Diligent::IBuffer* indexBuffer, bool use32BitsIndex, Usage usage)
    : m_stDevice(device), m_pDefinition(std::move(definition)), m_bUse32BitsIndex(use32BitsIndex), m_iUsage(usage),
    m_pVertexBuffer(vertexBuffer), m_pIndexBuffer(indexBuffer)
{
    assert(m_pVertexBuffer);
    assert(m_pIndexBuffer);
    m_pVertexBuffer->AddRef();
    m_pIndexBuffer->AddRef();
}

Mesh::~Mesh()
{
    if (m_pVertexBuffer)
    {
        m_pVertexBuffer->Release();
        m_pVertexBuffer = nullptr;
    }
    if (m_pIndexBuffer)
    {
        m_pIndexBuffer->Release();
        m_pIndexBuffer = nullptr;
    }
}

size_t Mesh::GetVertexCount() const noexcept
{
    // Dynamic Mesh 顶点个数可以和 Buffer 大小无关（总是2的幂次），此时取整，含义为可以存放的最大顶点个数
    assert(m_iUsage == Usage::Dynamic || m_pVertexBuffer->GetDesc().Size % m_pDefinition->GetVertexStride() == 0);
    return m_pVertexBuffer->GetDesc().Size / m_pDefinition->GetVertexStride();
}

size_t Mesh::GetIndexCount() const noexcept
{
    // Dynamic Mesh 索引个数可以和单个索引大小无关（总是2的幂次），此时取整，含义为可以存放的最大索引个数
    assert(m_iUsage == Usage::Dynamic || m_pIndexBuffer->GetDesc().Size % (m_bUse32BitsIndex ? 4 : 2) == 0);
    return m_pIndexBuffer->GetDesc().Size / (m_bUse32BitsIndex ? 4 : 2);
}

Result<void> Mesh::Commit(Span<const uint8_t> vertexData, Span<const uint8_t> indexData) noexcept
{
    // 参数检查
    if (m_iUsage != Usage::Dynamic)
        return make_error_code(errc::invalid_argument);
    if (vertexData.size() % m_pDefinition->GetVertexStride() != 0)
        return make_error_code(errc::invalid_argument);
    if (indexData.size() % (m_bUse32BitsIndex ? sizeof(uint32_t) : sizeof(uint16_t)) != 0)
        return make_error_code(errc::invalid_argument);

    // 检查是否需要申请更大的空间
    if (!m_pVertexBuffer || m_pVertexBuffer->GetDesc().Size < vertexData.size())
    {
        Diligent::RefCntAutoPtr<Diligent::IBuffer> vertexBuffer;

        // 计算需要的大小，总是取 2 的幂次
        auto desiredSize = ::max(16u, ::NextPowerOf2(vertexData.size()));
        assert(!m_pVertexBuffer || desiredSize > m_pVertexBuffer->GetDesc().Size);

        Diligent::BufferDesc vertexBufferDesc;
        vertexBufferDesc.Name = m_pVertexBuffer ? m_pVertexBuffer->GetDesc().Name : "";
        vertexBufferDesc.BindFlags = Diligent::BIND_VERTEX_BUFFER;
        vertexBufferDesc.Size = desiredSize;
        vertexBufferDesc.Usage = Diligent::USAGE_DYNAMIC;
        vertexBufferDesc.CPUAccessFlags = Diligent::CPU_ACCESS_WRITE;
        m_stDevice.GetDevice()->CreateBuffer(vertexBufferDesc, nullptr, &vertexBuffer);
        if (!vertexBuffer)
            return make_error_code(errc::io_error);

        if (m_pVertexBuffer)
            m_pVertexBuffer->Release();
        m_pVertexBuffer = vertexBuffer;
        m_pVertexBuffer->AddRef();
    }
    if (!m_pIndexBuffer || m_pIndexBuffer->GetDesc().Size < indexData.size())
    {
        Diligent::RefCntAutoPtr<Diligent::IBuffer> indexBuffer;

        // 计算需要的大小，总是取 2 的幂次
        auto desiredSize = ::max(16u, ::NextPowerOf2(indexData.size()));
        assert(!m_pIndexBuffer || desiredSize > m_pIndexBuffer->GetDesc().Size);

        Diligent::BufferDesc indexBufferDesc;
        indexBufferDesc.Name = m_pIndexBuffer ? m_pIndexBuffer->GetDesc().Name : "";
        indexBufferDesc.BindFlags = Diligent::BIND_INDEX_BUFFER;
        indexBufferDesc.Size = desiredSize;
        indexBufferDesc.Usage = Diligent::USAGE_DYNAMIC;
        indexBufferDesc.CPUAccessFlags = Diligent::CPU_ACCESS_WRITE;
        m_stDevice.GetDevice()->CreateBuffer(indexBufferDesc, nullptr, &indexBuffer);
        if (!indexBuffer)
            return make_error_code(errc::io_error);

        if (m_pIndexBuffer)
            m_pIndexBuffer->Release();
        m_pIndexBuffer = indexBuffer;
        m_pIndexBuffer->AddRef();
    }

    // 复制数据
    {
        assert(m_pVertexBuffer && m_pIndexBuffer);
        assert(m_pVertexBuffer->GetDesc().Size >= vertexData.size() && m_pIndexBuffer->GetDesc().Size >= indexData.size());
        auto context = m_stDevice.GetImmediateContext();

        void* data = nullptr;
        context->MapBuffer(m_pVertexBuffer, Diligent::MAP_WRITE, Diligent::MAP_FLAG_DISCARD, data);
        if (!data)
            return make_error_code(errc::io_error);
        ::memcpy(data, vertexData.data(), vertexData.size());
        context->UnmapBuffer(m_pVertexBuffer, Diligent::MAP_WRITE);

        context->MapBuffer(m_pIndexBuffer, Diligent::MAP_WRITE, Diligent::MAP_FLAG_DISCARD, data);
        if (!data)
            return make_error_code(errc::io_error);
        ::memcpy(data, indexData.data(), indexData.size());
        context->UnmapBuffer(m_pIndexBuffer, Diligent::MAP_WRITE);
    }
    return {};
}
