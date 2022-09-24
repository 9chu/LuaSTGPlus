/**
 * @file
 * @date 2022/8/7
 * @author 9chu
 * 此文件为 LuaSTGPlus 项目的一部分，版权与许可声明详见 COPYRIGHT.txt。
 */
#include <lstg/v2/GamePlay/BentLaser.hpp>

#include <lstg/Core/Logging.hpp>
#include <lstg/Core/Math/VectorHelper.hpp>
#include <lstg/Core/Math/Collider2D/IntersectCheck.hpp>
#include <lstg/v2/GameApp.hpp>
#include <lstg/v2/Asset/TextureAsset.hpp>
#include <lstg/v2/GamePlay/Components/Transform.hpp>
#include <lstg/v2/GamePlay/Components/Collider.hpp>

using namespace std;
using namespace lstg;
using namespace lstg::v2::GamePlay;

LSTG_DEF_LOG_CATEGORY(BentLaser);

bool BentLaser::Update(ScriptObjectId id, int32_t length, double width) noexcept
{
    auto& world = static_cast<GameApp&>(v2::GameApp::GetInstance()).GetDefaultWorld();

    auto emitterEntity = world.GetEntityByScriptObjectId(id);
    if (!emitterEntity)
        return false;
    auto transformComponent = emitterEntity->TryGetComponent<Components::Transform>();
    if (!transformComponent)
        return false;

    if (length <= 1)
    {
        LSTG_LOG_ERROR_CAT(BentLaser, "Invalid length {}", length);
        return false;
    }

    // 移除多余的节点，保证长度在length范围内
    while (m_stQueue.IsFull() || m_stQueue.GetSize() >= static_cast<size_t>(length))
    {
        LaserNode last;
        m_stQueue.Pop(last);

        // 减少总长度
        if (!m_stQueue.IsEmpty())
        {
            LaserNode front = m_stQueue.Front();
            m_dLength -= glm::length(last.Location - front.Location);
        }
    }

    // 添加新节点
    if (m_stQueue.GetSize() < static_cast<size_t>(length))
    {
        LaserNode node;
        node.Location = transformComponent->Location;
        node.HalfWidth = width / 2.;
        m_stQueue.Push(node);

        // 增加总长度
        if (m_stQueue.GetSize() > 1)
        {
            LaserNode& last = m_stQueue.Back();
            LaserNode& beforeLast = m_stQueue[m_stQueue.GetSize() - 2];
            m_dLength += glm::length(beforeLast.Location - last.Location);
        }
    }

    return true;
}

void BentLaser::Release() noexcept
{
}

bool BentLaser::Render(const char* textureName, BlendMode blend, Subsystem::Render::ColorRGBA32 c, Math::UVRectangle textureRect,
    double scale) const noexcept
{
    auto& app = static_cast<GameApp&>(v2::GameApp::GetInstance());
    auto& cmdBuffer = app.GetCommandBuffer();

    // 忽略只有一个节点的情况
    if (m_stQueue.GetSize() <= 1)
        return true;

    auto asset = app.GetAssetPools()->FindAsset(AssetTypes::Texture, textureName);
    if (!asset)
    {
        LSTG_LOG_ERROR_CAT(BentLaser, "Texture asset '{0}' not found", textureName);
        return false;
    }
    auto texture = static_pointer_cast<Asset::TextureAsset>(asset);

    // 缩放到 UV 坐标
    auto textureWidth = static_cast<float>(texture->GetWidth());
    auto textureHeight = static_cast<float>(texture->GetHeight());
    textureRect.SetLeft(textureRect.Left() / textureWidth);
    textureRect.SetTop(textureRect.Top() / textureHeight);
    textureRect.SetWidth(textureRect.Width() / textureWidth);
    textureRect.SetHeight(textureRect.Height() / textureHeight);

    // 预先生成顶点
    // 我们总是假定曲线激光的素材是横向放置的，因此 V 坐标总是可以确定。
    Subsystem::Render::Drawing2D::Vertex renderVertex[4] = {
        {
            { 0, 0, 0.5f, },
            { 0, textureRect.Top() },
            blend.VertexColorBlend == VertexColorBlendMode::Additive ? c : 0x000000FF,
            blend.VertexColorBlend == VertexColorBlendMode::Multiply ? c : 0xFFFFFFFF,
        },
        {
            { 0, 0, 0.5f, },
            { 0, textureRect.Top() },
            blend.VertexColorBlend == VertexColorBlendMode::Additive ? c : 0x000000FF,
            blend.VertexColorBlend == VertexColorBlendMode::Multiply ? c : 0xFFFFFFFF,
        },
        {
            { 0, 0, 0.5f, },
            { 0, textureRect.Top() + textureRect.Height() },
            blend.VertexColorBlend == VertexColorBlendMode::Additive ? c : 0x000000FF,
            blend.VertexColorBlend == VertexColorBlendMode::Multiply ? c : 0xFFFFFFFF,
        },
        {
            { 0, 0, 0.5f, },
            { 0, textureRect.Top() + textureRect.Height() },
            blend.VertexColorBlend == VertexColorBlendMode::Additive ? c : 0x000000FF,
            blend.VertexColorBlend == VertexColorBlendMode::Multiply ? c : 0xFFFFFFFF,
        },
    };

    double vecLength = 0.;
    for (size_t i = 0; i < m_stQueue.GetSize() - 1; ++i)
    {
        auto& cur = m_stQueue[i];
        auto& next = m_stQueue[i + 1];

        // === 计算最左侧的两个点 ===
        // 计算从cur到next的向量
        auto offsetA = cur.Location - next.Location;
        auto lenOffsetA = glm::length(offsetA);
        if (lenOffsetA < 0.0001f && i + 1 != m_stQueue.GetSize() - 1)
            continue;

        // 计算宽度上的扩展长度(旋转270度)
        auto expandVec = Math::Normalize(offsetA);
        std::swap(expandVec.x, expandVec.y);
        expandVec.y = -expandVec.y;

        if (i == 0)  // 如果是第一个节点，则其宽度扩展使用expandVec计算
        {
            auto expandX = expandVec.x * scale * cur.HalfWidth;
            auto expandY = expandVec.y * scale * cur.HalfWidth;
            renderVertex[0].Position.x = static_cast<float>(cur.Location.x + expandX);
            renderVertex[0].Position.y = static_cast<float>(cur.Location.y + expandY);
            renderVertex[0].TexCoord.x = textureRect.Left();
            renderVertex[3].Position.x = static_cast<float>(cur.Location.x - expandX);
            renderVertex[3].Position.y = static_cast<float>(cur.Location.y - expandY);
            renderVertex[3].TexCoord.x = textureRect.Left();
        }
        else  // 否则，拷贝1和2
        {
            renderVertex[0].Position = renderVertex[1].Position;
            renderVertex[0].TexCoord.x = renderVertex[1].TexCoord.x;
            renderVertex[3].Position = renderVertex[2].Position;
            renderVertex[3].TexCoord.x = renderVertex[2].TexCoord.x;
        }

        // === 计算最右侧的两个点 ===
        vecLength += lenOffsetA;
        if (i == m_stQueue.GetSize() - 2)  // 这是最后两个节点，则其宽度扩展使用expandVec计算
        {
            auto expandX = expandVec.x * scale * next.HalfWidth;
            auto expandY = expandVec.y * scale * next.HalfWidth;
            renderVertex[1].Position.x = static_cast<float>(next.Location.x + expandX);
            renderVertex[1].Position.y = static_cast<float>(next.Location.y + expandY);
            renderVertex[1].TexCoord.x = textureRect.Left() + textureRect.Width();
            renderVertex[2].Position.x = static_cast<float>(next.Location.x - expandX);
            renderVertex[2].Position.y = static_cast<float>(next.Location.y - expandY);
            renderVertex[2].TexCoord.x = textureRect.Left() + textureRect.Width();
        }
        else  // 否则，参考第三个点
        {
            auto& afterNext = m_stQueue[i + 2];

            // 计算向量next->afterNext并规范化，相加offsetA和offsetB后得角平分线
            auto offsetB = afterNext.Location - next.Location;
            auto angleBisect = Math::Normalize(offsetA) + Math::Normalize(offsetB);
            auto angleBisectLen = glm::length(angleBisect);

            double expandX, expandY;
            if (angleBisectLen < 0.00002f || angleBisectLen > 1.99998f)  // 几乎在一条直线上
            {
                expandX = expandVec.x * scale * next.HalfWidth;
                expandY = expandVec.y * scale * next.HalfWidth;
            }
            else // 计算角平分线到角两边距离为next.half_width * scale的偏移量
            {
                angleBisect *= (1. / angleBisectLen);  // angleBisect.Normalize();
                auto t = angleBisect * Math::Normalize(offsetA);
                auto l = scale * next.HalfWidth;
                auto expandDelta = sqrt(l * l / (1. - glm::length2(t)));
                expandX = angleBisect.x * expandDelta;
                expandY = angleBisect.y * expandDelta;
            }

            // 设置顶点
            auto u = static_cast<float>(textureRect.Left() + vecLength / m_dLength * textureRect.Width());
            renderVertex[1].Position.x = static_cast<float>(next.Location.x + expandX);
            renderVertex[1].Position.y = static_cast<float>(next.Location.y + expandY);
            renderVertex[1].TexCoord.x = u;
            renderVertex[2].Position.x = static_cast<float>(next.Location.x - expandX);
            renderVertex[2].Position.y = static_cast<float>(next.Location.y - expandY);
            renderVertex[2].TexCoord.x = u;

            // 修正交叉的情况
            auto cross1 = glm::dot(glm::vec2(renderVertex[1].Position.x - renderVertex[0].Position.x,
                                             renderVertex[1].Position.y - renderVertex[0].Position.y),
                                   glm::vec2(renderVertex[2].Position.x - renderVertex[3].Position.x,
                                             renderVertex[2].Position.y - renderVertex[3].Position.y));
            auto cross2 = glm::dot(glm::vec2(renderVertex[2].Position.x - renderVertex[0].Position.x,
                                             renderVertex[2].Position.y - renderVertex[0].Position.y),
                                   glm::vec2(renderVertex[1].Position.x - renderVertex[3].Position.x,
                                             renderVertex[1].Position.y - renderVertex[3].Position.y));
            if (cross2 > cross1)
            {
                std::swap(renderVertex[1].Position.x, renderVertex[2].Position.x);
                std::swap(renderVertex[1].Position.y, renderVertex[2].Position.y);
            }
        }

        // 绘制这一段
        cmdBuffer.SetColorBlendMode(blend.ColorBlend);
        auto ret = cmdBuffer.DrawQuad(texture->GetDrawingTexture().GetUnderlayTexture(), renderVertex);
        if (!ret)
        {
            LSTG_LOG_ERROR_CAT(BentLaser, "Draw texture '%s' fail: %s", textureName, ret.GetError().message().c_str());
            return false;
        }
    }
    return true;
}

bool BentLaser::CollisionCheck(double x, double y, double rot, double a, double b, bool rect) const noexcept
{
    // 忽略只有一个节点的情况
    if (m_stQueue.GetSize() <= 1)
        return false;

    // 创建形状
    Components::Collider colliderA;
    if (rect)
        colliderA.Shape = Math::Collider2D::OBBShape<double> { Vec2 { a, b } };
    else if (a == b)
        colliderA.Shape = Math::Collider2D::CircleShape<double> { a };
    else
        colliderA.Shape = Math::Collider2D::EllipseShape<double> { a, b };
    colliderA.RefreshAABB();

    // 计算 AABB 范围
    auto leftA = x - colliderA.AABBHalfSize.x;
    auto rightA = x + colliderA.AABBHalfSize.x;
    auto topA = y + colliderA.AABBHalfSize.y;
    auto bottomA = y - colliderA.AABBHalfSize.y;

    // 遍历队列依次判断碰撞
    for (size_t i = 0; i < m_stQueue.GetSize(); ++i)
    {
        auto& n = m_stQueue[i];

        // 创建形状
        Components::Collider colliderB;
        colliderB.Shape = Math::Collider2D::CircleShape<double> { n.HalfWidth };
        colliderB.RefreshAABB();

        // 计算 AABB 范围
        auto leftB = x - colliderB.AABBHalfSize.x;
        auto rightB = x + colliderB.AABBHalfSize.x;
        auto topB = y + colliderB.AABBHalfSize.y;
        auto bottomB = y - colliderB.AABBHalfSize.y;

        // 使用 AABB 进行快速判断
        Vec2 na { std::max(leftA, leftB), std::min(topA, topB) };
        Vec2 nb { std::min(rightA , rightB), std::max(bottomA, bottomB) };
        if (na.x <= nb.x && na.y >= nb.y)
        {
            // 执行碰撞检查
            if (Math::Collider2D::IsIntersect(Vec2 { x, y }, rot, colliderA.Shape, n.Location, 0., colliderB.Shape))
                return true;
        }
    }
    return false;
}

bool BentLaser::BoundCheck() const noexcept
{
    auto& world = static_cast<GameApp&>(v2::GameApp::GetInstance()).GetDefaultWorld();

    auto boundary = world.GetBoundary();
    auto topLeft = boundary.GetTopLeft();
    auto bottomRight = boundary.GetBottomRight();
    for (size_t i = 0; i < m_stQueue.GetSize(); ++i)
    {
        auto& n = m_stQueue[i];
        if (n.Location.x >= topLeft.x && n.Location.x <= bottomRight.x && n.Location.y <= topLeft.y && n.Location.y >= bottomRight.y)
            return true;
    }

    // 越界时返回 false，只有当所有的弹幕越界才返回 false
    return false;
}
