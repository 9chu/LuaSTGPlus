/**
 * @file
 * @date 2022/6/7
 * @author 9chu
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#include <lstg/v2/Asset/SpriteSequenceAsset.hpp>

#include <lstg/Core/Subsystem/Script/LuaStack.hpp>

using namespace std;
using namespace lstg;
using namespace lstg::v2::Asset;

#define SHAPE_CHANGED 1
#define UV_CHANGED 2
#define COLOR_CHANGED 4

Subsystem::Asset::AssetTypeId SpriteSequenceAsset::GetAssetTypeIdStatic() noexcept
{
    const auto& uniqueTypeName = Subsystem::Script::detail::GetUniqueTypeName<SpriteSequenceAsset>();
    return uniqueTypeName.Id;
}

SpriteSequenceAsset::SpriteSequenceAsset(std::string name, TextureAssetPtr texture, SequenceContainer frame, ColliderShape colliderShape)
    : Subsystem::Asset::Asset(std::move(name)), m_pTextureAsset(std::move(texture)), m_stFrames(std::move(frame)),
      m_stColliderShape(colliderShape)
{
    // 中心点取第一个 Frame 进行计算
    assert(!m_stFrames.empty());
    m_stAnchor = m_stFrames[0].GetCenter() - m_stFrames[0].GetTopLeft();
    m_stPrecomputedSequenceVertex.resize(m_stFrames.size());
    PrecomputedVertex(SHAPE_CHANGED | UV_CHANGED | COLOR_CHANGED);
}

void SpriteSequenceAsset::SetAnchor(Vec2 vec) noexcept
{
    m_stAnchor = vec;
    PrecomputedVertex(SHAPE_CHANGED);
}

void SpriteSequenceAsset::SetDefaultBlendMode(BlendMode mode) noexcept
{
    m_stDefaultBlendMode = mode;
    PrecomputedVertex(COLOR_CHANGED);
}

void SpriteSequenceAsset::SetDefaultBlendColor(std::array<ColorRGBA32, 4> color) noexcept
{
    m_stDefaultBlendColor = color;
    PrecomputedVertex(COLOR_CHANGED);
}

Subsystem::Asset::AssetTypeId SpriteSequenceAsset::GetAssetTypeId() const noexcept
{
    return GetAssetTypeIdStatic();
}

void SpriteSequenceAsset::PrecomputedVertex(int what) noexcept
{
    assert(m_stFrames.size() == m_stPrecomputedSequenceVertex.size());

    if ((what & SHAPE_CHANGED) == SHAPE_CHANGED)
    {
        for (size_t i = 0; i < m_stFrames.size(); ++i)
        {
            const auto& frame = m_stFrames[i];
            auto& precomputedVertex = m_stPrecomputedSequenceVertex[i];

            // 基本形状，在原点附近，且 z = 0.5
            // 注意坐标轴是 Y 向上，X 向右
            auto w = static_cast<float>(frame.Width());
            auto h = static_cast<float>(frame.Height());
            auto cx = static_cast<float>(m_stAnchor.x);
            auto cy = static_cast<float>(m_stAnchor.y);
            precomputedVertex[0].Position = { -cx, cy, 0.5f };
            precomputedVertex[1].Position = { w - cx, cy, 0.5f };
            precomputedVertex[2].Position = { w - cx, cy - h, 0.5f };
            precomputedVertex[3].Position = { -cx, cy - h, 0.5f };
        }
    }

    if ((what & UV_CHANGED) == UV_CHANGED)
    {
        for (size_t i = 0; i < m_stFrames.size(); ++i)
        {
            const auto& frame = m_stFrames[i];
            auto& precomputedVertex = m_stPrecomputedSequenceVertex[i];

            // 设置纹理
            auto texture = static_pointer_cast<v2::Asset::TextureAsset>(GetTexture());
            auto textureWidth = texture->GetWidth();
            auto textureHeight = texture->GetHeight();
            auto u = static_cast<float>(frame.Left() / textureWidth);
            auto v = static_cast<float>(frame.Top() / textureHeight);
            auto uw = static_cast<float>(frame.Width() / textureWidth);
            auto vh = static_cast<float>(frame.Height() / textureHeight);
            precomputedVertex[0].TexCoord = {u, v};
            precomputedVertex[1].TexCoord = {u + uw, v};
            precomputedVertex[2].TexCoord = {u + uw, v + vh};
            precomputedVertex[3].TexCoord = {u, v + vh};
        }
    }

    if ((what & COLOR_CHANGED) == COLOR_CHANGED)
    {
        for (size_t i = 0; i < m_stFrames.size(); ++i)
        {
            auto& precomputedVertex = m_stPrecomputedSequenceVertex[i];

            // 设置混合模式
            auto vertexColorBlendMode = m_stDefaultBlendMode.VertexColorBlend;
            if (vertexColorBlendMode == VertexColorBlendMode::Multiply)
            {
                precomputedVertex[0].Color0 = 0x000000FF;
                precomputedVertex[1].Color0 = 0x000000FF;
                precomputedVertex[2].Color0 = 0x000000FF;
                precomputedVertex[3].Color0 = 0x000000FF;

                precomputedVertex[0].Color1 = m_stDefaultBlendColor[0];
                precomputedVertex[1].Color1 = m_stDefaultBlendColor[1];
                precomputedVertex[2].Color1 = m_stDefaultBlendColor[2];
                precomputedVertex[3].Color1 = m_stDefaultBlendColor[3];
            }
            else
            {
                precomputedVertex[0].Color0 = m_stDefaultBlendColor[0];
                precomputedVertex[1].Color0 = m_stDefaultBlendColor[1];
                precomputedVertex[2].Color0 = m_stDefaultBlendColor[2];
                precomputedVertex[3].Color0 = m_stDefaultBlendColor[3];

                precomputedVertex[0].Color1 = 0xFFFFFFFF;
                precomputedVertex[1].Color1 = 0xFFFFFFFF;
                precomputedVertex[2].Color1 = 0xFFFFFFFF;
                precomputedVertex[3].Color1 = 0xFFFFFFFF;
            }
        }
    }
}
