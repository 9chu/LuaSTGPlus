/**
 * @file
 * @date 2022/3/19
 * @author 9chu
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#include <lstg/Core/Subsystem/Render/Mesh.hpp>

#include <Buffer.h>

using namespace std;
using namespace lstg;
using namespace lstg::Subsystem::Render;

Mesh::Mesh(GraphDef::ImmutableMeshDefinitionPtr definition, Diligent::IBuffer* vertexBuffer, Diligent::IBuffer* indexBuffer,
    bool use32BitsIndex)
    : m_pDefinition(std::move(definition)), m_bUse32BitsIndex(use32BitsIndex), m_pVertexBuffer(vertexBuffer), m_pIndexBuffer(indexBuffer)
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
    assert(m_pVertexBuffer->GetDesc().Size % m_pDefinition->GetVertexStride() == 0);
    return m_pVertexBuffer->GetDesc().Size / m_pDefinition->GetVertexStride();
}

size_t Mesh::GetIndexCount() const noexcept
{
    assert(m_pIndexBuffer->GetDesc().Size % (m_bUse32BitsIndex ? 4 : 2) == 0);
    return m_pIndexBuffer->GetDesc().Size / (m_bUse32BitsIndex ? 4 : 2);
}
