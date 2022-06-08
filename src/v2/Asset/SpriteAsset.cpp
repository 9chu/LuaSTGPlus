/**
 * @file
 * @date 2022/5/31
 * @author 9chu
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#include <lstg/v2/Asset/SpriteAsset.hpp>

#include <lstg/Core/Subsystem/Script/LuaStack.hpp>

using namespace std;
using namespace lstg;
using namespace lstg::v2::Asset;

#define SHAPE_CHANGED 1
#define UV_CHANGED 2
#define COLOR_CHANGED 4

Subsystem::Asset::AssetTypeId SpriteAsset::GetAssetTypeIdStatic() noexcept
{
    const auto& uniqueTypeName = Subsystem::Script::detail::GetUniqueTypeName<SpriteAsset>();
    return uniqueTypeName.Id;
}

SpriteAsset::SpriteAsset(std::string name, TextureAssetPtr texture, UVRectangle frame, ColliderShape colliderShape)
    : Subsystem::Asset::Asset(std::move(name)), m_pTextureAsset(std::move(texture)), m_stFrame(frame), m_stColliderShape(colliderShape)
{
    m_stAnchor = m_stFrame.GetCenter() - m_stFrame.GetTopLeft();
    PrecomputedVertex(SHAPE_CHANGED | UV_CHANGED | COLOR_CHANGED);
}

void SpriteAsset::SetAnchor(Vec2 vec) noexcept
{
    m_stAnchor = vec;
    PrecomputedVertex(SHAPE_CHANGED);
}

void SpriteAsset::SetDefaultBlendMode(BlendMode mode) noexcept
{
    m_stDefaultBlendMode = mode;
    PrecomputedVertex(COLOR_CHANGED);
}

void SpriteAsset::SetDefaultBlendColor(std::array<ColorRGBA32, 4> color) noexcept
{
    m_stDefaultBlendColor = color;
    PrecomputedVertex(COLOR_CHANGED);
}

Subsystem::Asset::AssetTypeId SpriteAsset::GetAssetTypeId() const noexcept
{
    return GetAssetTypeIdStatic();
}

void SpriteAsset::PrecomputedVertex(int what) noexcept
{
    if ((what & SHAPE_CHANGED) == SHAPE_CHANGED)
    {
        // 基本形状，在原点附近，且 z = 0.5
        // 注意坐标轴是 Y 向上，X 向右
        auto w = static_cast<float>(m_stFrame.Width());
        auto h = static_cast<float>(m_stFrame.Height());
        auto cx = static_cast<float>(m_stAnchor.x);
        auto cy = static_cast<float>(m_stAnchor.y);
        m_stPrecomputedVertex[0].Position = { -cx, cy, 0.5f };
        m_stPrecomputedVertex[1].Position = { w - cx, cy, 0.5f };
        m_stPrecomputedVertex[2].Position = { w - cx, cy - h, 0.5f };
        m_stPrecomputedVertex[3].Position = { -cx, cy - h, 0.5f };
    }

    if ((what & UV_CHANGED) == UV_CHANGED)
    {
        // 设置纹理
        auto texture = static_pointer_cast<v2::Asset::TextureAsset>(GetTexture());
        auto textureWidth = texture->GetWidth();
        auto textureHeight = texture->GetHeight();
        auto u = static_cast<float>(m_stFrame.Left() / textureWidth);
        auto v = static_cast<float>(m_stFrame.Top() / textureHeight);
        auto uw = static_cast<float>(m_stFrame.Width() / textureWidth);
        auto vh = static_cast<float>(m_stFrame.Height() / textureHeight);
        m_stPrecomputedVertex[0].TexCoord = { u, v };
        m_stPrecomputedVertex[1].TexCoord = { u + uw, v };
        m_stPrecomputedVertex[2].TexCoord = { u + uw, v + vh };
        m_stPrecomputedVertex[3].TexCoord = { u, v + vh };
    }

    if ((what & COLOR_CHANGED) == COLOR_CHANGED)
    {
        // 设置混合模式
        auto vertexColorBlendMode = m_stDefaultBlendMode.VertexColorBlend;
        if (vertexColorBlendMode == VertexColorBlendMode::Multiply)
        {
            m_stPrecomputedVertex[0].Color0 = 0x000000FF;
            m_stPrecomputedVertex[1].Color0 = 0x000000FF;
            m_stPrecomputedVertex[2].Color0 = 0x000000FF;
            m_stPrecomputedVertex[3].Color0 = 0x000000FF;

            m_stPrecomputedVertex[0].Color1 = m_stDefaultBlendColor[0];
            m_stPrecomputedVertex[1].Color1 = m_stDefaultBlendColor[1];
            m_stPrecomputedVertex[2].Color1 = m_stDefaultBlendColor[2];
            m_stPrecomputedVertex[3].Color1 = m_stDefaultBlendColor[3];
        }
        else
        {
            m_stPrecomputedVertex[0].Color0 = m_stDefaultBlendColor[0];
            m_stPrecomputedVertex[1].Color0 = m_stDefaultBlendColor[1];
            m_stPrecomputedVertex[2].Color0 = m_stDefaultBlendColor[2];
            m_stPrecomputedVertex[3].Color0 = m_stDefaultBlendColor[3];

            m_stPrecomputedVertex[0].Color1 = 0xFFFFFFFF;
            m_stPrecomputedVertex[1].Color1 = 0xFFFFFFFF;
            m_stPrecomputedVertex[2].Color1 = 0xFFFFFFFF;
            m_stPrecomputedVertex[3].Color1 = 0xFFFFFFFF;
        }
    }
}
