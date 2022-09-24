/**
 * @file
 * @date 2022/3/20
 * @author 9chu
 * 此文件为 LuaSTGPlus 项目的一部分，版权与许可声明详见 COPYRIGHT.txt。
 */
#include <lstg/Core/Subsystem/Render/ConstantBuffer.hpp>

#include <Buffer.h>
#include <RenderDevice.h>
#include <DeviceContext.h>

using namespace std;
using namespace lstg;
using namespace lstg::Subsystem::Render;

ConstantBuffer::ConstantBuffer(RenderDevice& device, GraphDef::ImmutableConstantBufferDefinitionPtr definition, Usage usage)
    : m_stDevice(device), m_pDefinition(std::move(definition)), m_iUsage(usage)
{
    assert(m_pDefinition);
    assert(m_pDefinition->GetSize() != 0);

    // 申请内存缓冲区
    m_stBuffer.resize(m_pDefinition->GetSize());
    assert(m_stBuffer.size() % 16 == 0);  // 总是 16 字节对齐

    // 初始化 Dirty Flags
    if (m_iUsage == Usage::Default)
    {
        m_stDirtyState.Region.Start = 0;
        m_stDirtyState.Region.Size = m_stBuffer.size();
    }
    else
    {
        m_stDirtyState.Flag.CommitRequired = true;
        m_stDirtyState.Flag.LastCommitFrameId = 0;
    }

    // 创建 Buffer 对象
    Diligent::BufferDesc desc;
    desc.Size = m_pDefinition->GetSize();
    desc.Usage = (m_iUsage == Usage::Default ? Diligent::USAGE_DEFAULT : Diligent::USAGE_DYNAMIC);
    desc.BindFlags = Diligent::BIND_UNIFORM_BUFFER;
    desc.CPUAccessFlags = (m_iUsage == Usage::Default ? Diligent::CPU_ACCESS_NONE : Diligent::CPU_ACCESS_WRITE);
    device.GetDevice()->CreateBuffer(desc, nullptr, &m_pNativeHandler);
    if (!m_pNativeHandler)
        throw system_error(make_error_code(errc::not_enough_memory));
}

ConstantBuffer::~ConstantBuffer()
{
    if (m_pNativeHandler)
    {
        m_pNativeHandler->Release();
        m_pNativeHandler = nullptr;
    }
}

const GraphDef::ImmutableConstantBufferDefinitionPtr& ConstantBuffer::GetDefinition() const noexcept
{
    return m_pDefinition;
}

size_t ConstantBuffer::GetSize() const noexcept
{
    assert(m_pDefinition->GetSize() <= m_pNativeHandler->GetDesc().Size);  // 实际申请 GPU 侧的 Buffer 由于不同的对齐，可能大于我们需要的
    assert(m_pDefinition->GetSize() == m_stBuffer.size());
    return m_pDefinition->GetSize();
}

void ConstantBuffer::CopyFrom(void* src, size_t size, size_t offset) noexcept
{
    assert(offset < m_stBuffer.size() && offset + size <= m_stBuffer.size());
    uint8_t* memory = m_stBuffer.data() + offset;
    ::memcpy(memory, src, size);

    if (m_iUsage == Usage::Default)
    {
        // 刷新脏数据区块
        if (m_stDirtyState.Region.Size == 0)
        {
            m_stDirtyState.Region.Start = offset;
            m_stDirtyState.Region.Size = size;
        }
        else
        {
            auto dirtyRegionEnd = m_stDirtyState.Region.Start + m_stDirtyState.Region.Size;
            dirtyRegionEnd = std::max(dirtyRegionEnd, offset + size);
            m_stDirtyState.Region.Start = std::min(m_stDirtyState.Region.Start, offset);
            m_stDirtyState.Region.Size = dirtyRegionEnd - m_stDirtyState.Region.Start;
        }
    }
    else
    {
        // 动态 Buffer 总是整个刷新，这里只记录脏标记
        m_stDirtyState.Flag.CommitRequired = true;
    }
}

Result<void> ConstantBuffer::Commit() noexcept
{
    auto context = m_stDevice.GetImmediateContext();
    if (m_iUsage == Usage::Default)
    {
        if (m_stDirtyState.Region.Size == 0)
            return {};

        assert(m_stDirtyState.Region.Start + m_stDirtyState.Region.Size <= m_stBuffer.size());
        context->UpdateBuffer(m_pNativeHandler, m_stDirtyState.Region.Start, m_stDirtyState.Region.Size,
            m_stBuffer.data() + m_stDirtyState.Region.Start, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

        m_stDirtyState.Region.Start = m_stDirtyState.Region.Size = 0;
    }
    else
    {
        assert(m_iUsage == Usage::Dynamic);

        auto frame = m_stDevice.GetPresentedFrameCount();
        if (!m_stDirtyState.Flag.CommitRequired && m_stDirtyState.Flag.LastCommitFrameId == frame)
            return {};

        void* data = nullptr;
        context->MapBuffer(m_pNativeHandler, Diligent::MAP_WRITE, Diligent::MAP_FLAG_DISCARD, data);
        if (!data)
            return make_error_code(errc::io_error);
        ::memcpy(data, m_stBuffer.data(), m_stBuffer.size());
        context->UnmapBuffer(m_pNativeHandler, Diligent::MAP_WRITE);

        m_stDirtyState.Flag.CommitRequired = false;
        m_stDirtyState.Flag.LastCommitFrameId = frame;
    }
    return {};
}
