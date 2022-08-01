/**
 * @file
 * @date 2022/7/9
 * @author 9chu
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#include <lstg/Core/Subsystem/Render/Drawing2D/Sprite.hpp>

using namespace std;
using namespace lstg;
using namespace lstg::Subsystem::Render::Drawing2D;

#define SHAPE_CHANGED 1
#define UV_CHANGED 2
#define ADDITIVE_COLOR_CHANGED 4
#define MULTIPLY_COLOR_CHANGED 8

const Texture2D* Sprite::GetTexture2D() const noexcept
{
    return m_pTexture;
}

void Sprite::SetTexture2D(const Texture2D* tex, bool updateVertex) noexcept
{
    assert(tex);
    m_pTexture = tex;
    if (updateVertex)
        PrecomputedVertex(UV_CHANGED);
}

const Math::ImageRectangleFloat& Sprite::GetFrame() const noexcept
{
    return m_stFrame;
}

void Sprite::SetFrame(const Math::ImageRectangleFloat& rect, bool updateVertex) noexcept
{
    m_stFrame = rect;
    if (updateVertex)
        PrecomputedVertex(UV_CHANGED);
}

const glm::vec2& Sprite::GetAnchor() const noexcept
{
    return m_stAnchor;
}

void Sprite::SetAnchor(glm::vec2 vec, bool updateVertex) noexcept
{
    m_stAnchor = vec;
    if (updateVertex)
        PrecomputedVertex(SHAPE_CHANGED);
}

ColorBlendMode Sprite::GetColorBlendMode() const noexcept
{
    return m_iBlendMode;
}

void Sprite::SetColorBlendMode(ColorBlendMode m) noexcept
{
    m_iBlendMode = m;
}

const SpriteColorComponents& Sprite::GetAdditiveBlendColor() const noexcept
{
    return m_stAdditiveBlendColor;
}

void Sprite::SetAdditiveBlendColor(const SpriteColorComponents& color, bool updateVertex) noexcept
{
    m_stAdditiveBlendColor = color;
    if (updateVertex)
        PrecomputedVertex(ADDITIVE_COLOR_CHANGED);
}

const SpriteColorComponents& Sprite::GetMultiplyBlendColor() const noexcept
{
    return m_stMultiplyBlendColor;
}

void Sprite::SetMultiplyBlendColor(const SpriteColorComponents& color, bool updateVertex) noexcept
{
    m_stMultiplyBlendColor = color;
    if (updateVertex)
        PrecomputedVertex(MULTIPLY_COLOR_CHANGED);
}

const std::array<Vertex, 4>& Sprite::GetPrecomputedVertex() const noexcept
{
    return m_stPrecomputedVertex;
}

void Sprite::UpdatePrecomputedVertex() noexcept
{
    PrecomputedVertex(SHAPE_CHANGED | UV_CHANGED | ADDITIVE_COLOR_CHANGED | MULTIPLY_COLOR_CHANGED);
}

Result<SpriteDrawing> Sprite::Draw(CommandBuffer& buffer) const noexcept
{
    buffer.SetColorBlendMode(m_iBlendMode);
    return SpriteDrawing::Draw(buffer, m_pTexture ? m_pTexture->GetUnderlayTexture() : nullptr, m_stPrecomputedVertex);
}

void Sprite::PrecomputedVertex(int what) noexcept
{
    if ((what & SHAPE_CHANGED) == SHAPE_CHANGED)
    {
        // 基本形状，在原点附近
        // 注意坐标轴是 Y 向上，X 向右
        auto w = m_stFrame.Width();
        auto h = m_stFrame.Height();
        auto cx = m_stAnchor.x;
        auto cy = m_stAnchor.y;
        m_stPrecomputedVertex[0].Position = { -cx, cy, 0.f };
        m_stPrecomputedVertex[1].Position = { w - cx, cy, 0.f };
        m_stPrecomputedVertex[2].Position = { w - cx, cy - h, 0.f };
        m_stPrecomputedVertex[3].Position = { -cx, cy - h, 0.f };
    }

    if (m_pTexture && (what & UV_CHANGED) == UV_CHANGED)
    {
        // 设置纹理
        auto textureWidth = m_pTexture->GetWidth();
        auto textureHeight = m_pTexture->GetHeight();
        auto u = static_cast<float>(m_stFrame.Left() / textureWidth);
        auto v = static_cast<float>(m_stFrame.Top() / textureHeight);
        auto uw = static_cast<float>(m_stFrame.Width() / textureWidth);
        auto vh = static_cast<float>(m_stFrame.Height() / textureHeight);
        m_stPrecomputedVertex[0].TexCoord = { u, v };
        m_stPrecomputedVertex[1].TexCoord = { u + uw, v };
        m_stPrecomputedVertex[2].TexCoord = { u + uw, v + vh };
        m_stPrecomputedVertex[3].TexCoord = { u, v + vh };
    }

    if ((what & ADDITIVE_COLOR_CHANGED) == ADDITIVE_COLOR_CHANGED)
    {
        m_stPrecomputedVertex[0].Color0 = m_stAdditiveBlendColor[0];
        m_stPrecomputedVertex[1].Color0 = m_stAdditiveBlendColor[1];
        m_stPrecomputedVertex[2].Color0 = m_stAdditiveBlendColor[2];
        m_stPrecomputedVertex[3].Color0 = m_stAdditiveBlendColor[3];
    }
    if ((what & MULTIPLY_COLOR_CHANGED) == MULTIPLY_COLOR_CHANGED)
    {
        m_stPrecomputedVertex[0].Color1 = m_stMultiplyBlendColor[0];
        m_stPrecomputedVertex[1].Color1 = m_stMultiplyBlendColor[1];
        m_stPrecomputedVertex[2].Color1 = m_stMultiplyBlendColor[2];
        m_stPrecomputedVertex[3].Color1 = m_stMultiplyBlendColor[3];
    }
}
