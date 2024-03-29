/**
 * @file
 * @date 2022/4/14
 * @author 9chu
 * 此文件为 LuaSTGPlus 项目的一部分，版权与许可声明详见 COPYRIGHT.txt。
 */
#include <lstg/Core/Subsystem/Render/Drawing2D/CommandBuffer.hpp>

#include <glm/ext.hpp>
#include <lstg/Core/Logging.hpp>

using namespace std;
using namespace lstg;
using namespace lstg::Subsystem::Render::Drawing2D;

// LSTG_DEF_LOG_CATEGORY(CommandBuffer);

CommandBuffer::CommandBuffer()
    : m_stCurrentView(glm::identity<glm::mat4x4>()), m_stCurrentProjection(glm::identity<glm::mat4x4>())
{
}

void CommandBuffer::Begin() noexcept
{
    // Begin 时不重置渲染状态，保留最后一次的 Set
    m_stCameraReferences.clear();
    m_stCameraMapping.clear();
    m_stTextureReferences.clear();
    m_stTextureMapping.clear();
    m_stMaterialReferences.clear();
    m_stMaterialMapping.clear();
    m_uCurrentBaseVertexIndex = 0;
    m_stVertices.clear();
    m_stIndexes.clear();
    m_stCommandGroups.clear();
    m_stCurrentGroup = {};
    m_stCurrentQueue = {};
    m_stCurrentDrawCommand = {};
}

CommandBuffer::DrawData CommandBuffer::End() noexcept
{
    // 如果 End 后还有新的写操作，要求总是产生新的 CommandGroup 和 CommandQueue
    m_stCurrentGroup = {};
    m_stCurrentQueue = {};
    m_stCurrentDrawCommand = {};

    return {
        m_stCommandGroups,
        m_stVertices,
        m_stIndexes,
        m_stCameraReferences,
        m_stTextureReferences,
        m_stMaterialReferences,
    };
}

Subsystem::Render::TexturePtr CommandBuffer::FindTextureById(size_t id) const noexcept
{
    if (id >= m_stTextureReferences.size())
        return {};
    return m_stTextureReferences[id];
}

Subsystem::Render::MaterialPtr CommandBuffer::FindMaterialById(size_t id) const noexcept
{
    if (id >= m_stMaterialReferences.size())
        return {};
    return m_stMaterialReferences[id];
}

void CommandBuffer::SetGroupId(uint64_t id) noexcept
{
    if (id == m_ullCurrentGroupId)
        return;
    PrepareNewGroup();
    m_ullCurrentGroupId = id;
}

void CommandBuffer::SetView(glm::mat4x4 matrix) noexcept
{
    if (matrix == m_stCurrentView)
        return;
    PrepareNewQueue();
    m_stCurrentView = matrix;
}

void CommandBuffer::SetProjection(glm::mat4x4 matrix) noexcept
{
    if (matrix == m_stCurrentProjection)
        return;
    PrepareNewQueue();
    m_stCurrentProjection = matrix;
}

void CommandBuffer::SetViewport(float l, float t, float w, float h) noexcept
{
    Render::Camera::Viewport vp { l, t, w, h };
    if (vp == m_stCurrentViewport)
        return;
    PrepareNewQueue();
    m_stCurrentViewport = vp;
}

void CommandBuffer::SetOutputViews(TexturePtr colorView, TexturePtr depthStencilView) noexcept
{
    Render::Camera::OutputViews ov { std::move(colorView), std::move(depthStencilView) };
    if (ov == m_stCurrentOutputViews)
        return;
    PrepareNewQueue();
    m_stCurrentOutputViews = std::move(ov);
}

void CommandBuffer::SetMaterial(MaterialPtr material) noexcept
{
    if (material == m_pCurrentMaterial)
        return;
    PrepareNewCommand();
    m_pCurrentMaterial = std::move(material);
}

void CommandBuffer::SetColorBlendMode(ColorBlendMode m) noexcept
{
    if (m == m_iCurrentColorBlendMode)
        return;
    PrepareNewCommand();
    m_iCurrentColorBlendMode = m;
}

void CommandBuffer::SetNoDepth(bool b) noexcept
{
    if (b == m_bNoDepth)
        return;
    PrepareNewCommand();
    m_bNoDepth = b;
}

void CommandBuffer::SetFog(FogTypes fog, ColorRGBA32 fogColor, float arg1, float arg2) noexcept
{
    if (fog == m_iCurrentFogType && arg1 == m_fCurrentFogArg1 && arg2 == m_fCurrentFogArg2)
        return;
    PrepareNewCommand();
    m_iCurrentFogType = fog;
    m_fCurrentFogArg1 = arg1;
    m_fCurrentFogArg2 = arg2;
    m_stCurrentFogColor = fogColor;
}

Result<void> CommandBuffer::DrawQuad(TexturePtr tex2d, const Vertex (&arr)[4]) noexcept
{
    static_assert(is_trivially_copyable_v<Vertex>);

    auto ret = DrawQuadInPlace(std::move(tex2d));
    if (!ret)
        return ret.GetError();
    assert(ret->GetSize() == 4);
    ::memcpy(ret->GetData(), arr, sizeof(arr));
    return {};
}

Result<Span<Vertex>> CommandBuffer::DrawQuadInPlace(TexturePtr tex2d) noexcept
{
    // 创建纹理
    auto texId = AllocTexture(std::move(tex2d));
    if (!texId)
        return texId.GetError();

    // 创建材质
    auto matId = AllocMaterial(m_pCurrentMaterial);
    if (!matId)
        return matId.GetError();

    // 创建命令（若没有）
    auto ret = InstantialCommand();
    if (!ret)
        return ret.GetError();

    if ((*m_stCurrentDrawCommand)->TextureId == static_cast<size_t>(-1))
    {
        // 此时是新创建的命令，直接设置 TextureId 和 MatId
        assert((*m_stCurrentDrawCommand)->MaterialId == static_cast<size_t>(-1));
        assert((*m_stCurrentDrawCommand)->IndexCount == 0);
        (*m_stCurrentDrawCommand)->TextureId = *texId;
        (*m_stCurrentDrawCommand)->MaterialId = *matId;
    }
    else if ((*m_stCurrentDrawCommand)->TextureId != *texId || (*m_stCurrentDrawCommand)->MaterialId != *matId)
    {
        // 此时需要创建新的 Command
        PrepareNewCommand();
        ret = InstantialCommand();
        if (!ret)
            return ret.GetError();
        assert((*m_stCurrentDrawCommand)->TextureId == static_cast<size_t>(-1));
        assert((*m_stCurrentDrawCommand)->MaterialId == static_cast<size_t>(-1));
        (*m_stCurrentDrawCommand)->TextureId = *texId;
        (*m_stCurrentDrawCommand)->MaterialId = *matId;
    }

    // 防止索引越界
    assert(m_stVertices.size() + 4 - m_uCurrentBaseVertexIndex <= std::numeric_limits<uint16_t>::max());

    // 分配内存
    try
    {
        m_stVertices.resize(m_stVertices.size() + 4);
        m_stIndexes.resize(m_stIndexes.size() + 6);
    }
    catch (...)  // bad_alloc
    {
        return make_error_code(errc::not_enough_memory);
    }

    // 分配顶点
    auto vertexStartIndex = m_stVertices.size() - 4;
    auto vertexStart = m_stVertices.data() + vertexStartIndex;
    vertexStartIndex -= m_uCurrentBaseVertexIndex;  // 基于 BaseVertexIndex 的偏移
//    vertexStart[0] = arr[0];
//    vertexStart[1] = arr[1];
//    vertexStart[2] = arr[2];
//    vertexStart[3] = arr[3];

    // 生成索引
    // 0 -- 1
    // | \  |
    // |  \ |
    // 3 -- 2
    auto indexStart = m_stIndexes.data() + m_stIndexes.size() - 6;
    indexStart[0] = vertexStartIndex + 0;
    indexStart[1] = vertexStartIndex + 1;
    indexStart[2] = vertexStartIndex + 2;
    indexStart[3] = vertexStartIndex + 0;
    indexStart[4] = vertexStartIndex + 2;
    indexStart[5] = vertexStartIndex + 3;

    (*m_stCurrentDrawCommand)->IndexCount += 6;
    return Span<Vertex> { vertexStart, 4 };
}

Result<void> CommandBuffer::Clear(ColorRGBA32 color) noexcept
{
    // Clear 总是会占用一个独立的 Queue
    PrepareNewQueue();
    auto ret = InstantialQueue();
    if (!ret)
        return ret.GetError();

    // 设置 Clear 标记
    (*m_stCurrentQueue)->get()->ClearFlag = ClearFlags::Color | ClearFlags::Depth;
    (*m_stCurrentQueue)->get()->ClearColor = color;
    return {};
}

void CommandBuffer::PrepareNewGroup() noexcept
{
    m_stCurrentGroup = {};
    m_stCurrentQueue = {};
    m_stCurrentDrawCommand = {};
}

void CommandBuffer::PrepareNewQueue() noexcept
{
    m_stCurrentQueue = {};
    m_stCurrentDrawCommand = {};
}

void CommandBuffer::PrepareNewCommand() noexcept
{
    m_stCurrentDrawCommand = {};
}

size_t CommandBuffer::AllocCamera()
{
    // 先检查是否存在重复的相机，此时可以直接复用
    CameraStateKey state { {}, m_stCurrentView, m_stCurrentProjection, m_stCurrentViewport, m_stCurrentOutputViews };
    auto it = m_stCameraMapping.find(state);
    if (it != m_stCameraMapping.end())
    {
        auto idx = it->second;
        assert(idx < m_stCameraReferences.size());
        return idx;
    }

    // 如果不存在复用，则创建相机
    auto ptr = m_stCameraFreeList.Alloc();
    if (*(ptr.get()) == nullptr)
        *(ptr.get()) = make_shared<Render::Camera>();
    assert(ptr.get());

    // 设置参数
    Render::Camera* camera = (*(ptr.get())).get();
    assert(camera);
    camera->SetViewMatrix(m_stCurrentView);
    camera->SetProjectMatrix(m_stCurrentProjection);
    camera->SetViewport(m_stCurrentViewport);
    camera->SetOutputViews(m_stCurrentOutputViews);

    m_stCameraReferences.emplace_back(std::move(ptr));
    m_stCameraMapping.emplace(state, m_stCameraReferences.size() - 1);
    return m_stCameraReferences.size() - 1;
}

Result<size_t> CommandBuffer::AllocTexture(TexturePtr tex2d) noexcept
{
    auto p = tex2d.get();
    auto it = m_stTextureMapping.find(p);
    if (it == m_stTextureMapping.end())
    {
        try
        {
            m_stTextureReferences.emplace_back(std::move(tex2d));
            m_stTextureMapping.emplace(p, m_stTextureReferences.size() - 1);
            return m_stTextureReferences.size() - 1;
        }
        catch (...)  // bad_alloc
        {
            return make_error_code(errc::not_enough_memory);
        }
    }
    return it->second;
}

Result<size_t> CommandBuffer::AllocMaterial(MaterialPtr mat) noexcept
{
    auto p = mat.get();
    auto it = m_stMaterialMapping.find(p);
    if (it == m_stMaterialMapping.end())
    {
        try
        {
            m_stMaterialReferences.emplace_back(std::move(mat));
            m_stMaterialMapping.emplace(p, m_stMaterialReferences.size() - 1);
            return m_stMaterialReferences.size() - 1;
        }
        catch (...)  // bad_alloc
        {
            return make_error_code(errc::not_enough_memory);
        }
    }
    return it->second;
}

Result<void> CommandBuffer::InstantialGroup() noexcept
{
    if (!m_stCurrentGroup.has_value())
    {
        try
        {
            auto group = m_stCommandGroupFreeList.Alloc();
            group->Queue.clear();
            group->GroupId = m_ullCurrentGroupId;
            m_stCommandGroups.emplace_back(std::move(group));
            m_stCurrentGroup = m_stCommandGroups.end() - 1;
        }
        catch (...)  // bad_alloc
        {
            return make_error_code(errc::not_enough_memory);
        }
    }
    return {};
}

Result<void> CommandBuffer::InstantialQueue() noexcept
{
    if (!m_stCurrentQueue.has_value())
    {
        auto ret = InstantialGroup();
        if (!ret)
            return ret.GetError();

        assert(m_stCurrentGroup.has_value());
        assert(*m_stCurrentGroup != m_stCommandGroups.end());
        try
        {
            auto queue = m_stCommandQueueFreeList.Alloc();
            queue->CameraId = AllocCamera();
            queue->ClearFlag = ClearFlags::None;
            queue->ClearColor = 0x00000000;
            queue->Commands.clear();
            (*(*m_stCurrentGroup))->Queue.emplace_back(std::move(queue));
            m_stCurrentQueue = (*(*m_stCurrentGroup))->Queue.end() - 1;
        }
        catch (...)  // bad_alloc
        {
            return make_error_code(errc::not_enough_memory);
        }
    }
    return {};
}

Result<void> CommandBuffer::InstantialCommand() noexcept
{
    if (!m_stCurrentDrawCommand.has_value())
    {
        auto ret = InstantialQueue();
        if (!ret)
            return ret.GetError();

        assert(m_stCurrentQueue.has_value());
        assert((*m_stCurrentQueue) != (*m_stCurrentGroup)->get()->Queue.end());
        try
        {
            auto command = DrawCommand {
                m_iCurrentColorBlendMode,
                m_bNoDepth,
                m_iCurrentFogType,
                m_fCurrentFogArg1,
                m_fCurrentFogArg2,
                m_stCurrentFogColor,
                static_cast<size_t>(-1),
                static_cast<size_t>(-1),
                m_stIndexes.size(),
                0,
                m_uCurrentBaseVertexIndex,
            };
            (*m_stCurrentQueue)->get()->Commands.emplace_back(command);
            m_stCurrentDrawCommand = (*m_stCurrentQueue)->get()->Commands.end() - 1;
        }
        catch (...)  // bad_alloc
        {
            return make_error_code(errc::not_enough_memory);
        }
    }

    // 如果当前 DrawCommand 无法容纳足够数量的顶点，则创建新的 DrawCommand 并刷新 BaseVertexIndex
    if (m_stVertices.size() + 4 - m_uCurrentBaseVertexIndex > std::numeric_limits<uint16_t>::max())
    {
        m_uCurrentBaseVertexIndex = m_stVertices.size();
        PrepareNewCommand();
        return InstantialCommand();
    }
    return {};
}
