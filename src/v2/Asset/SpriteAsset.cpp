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

using namespace lstg::Subsystem::Render::Drawing2D;

Subsystem::Asset::AssetTypeId SpriteAsset::GetAssetTypeIdStatic() noexcept
{
    const auto& uniqueTypeName = Subsystem::Script::detail::GetUniqueTypeName<SpriteAsset>();
    return uniqueTypeName.Id;
}

SpriteAsset::SpriteAsset(std::string name, TextureAssetPtr texture, Math::ImageRectangleFloat frame, ColliderShape colliderShape)
    : Subsystem::Asset::Asset(std::move(name)), m_pTextureAsset(std::move(texture)),  m_stColliderShape(colliderShape)
{
    m_stSprite.SetFrame(frame, false);
    m_stSprite.SetAnchor(frame.GetCenter() - frame.GetTopLeft(), false);
    m_stSprite.SetTexture2D(&m_pTextureAsset->GetDrawingTexture(), false);
    SyncBlendMode(false);
    m_stSprite.UpdatePrecomputedVertex();  // 统一刷新顶点
}

void SpriteAsset::SetDefaultBlendMode(BlendMode mode) noexcept
{
    m_stDefaultBlendMode = mode;
    SyncBlendMode();
}

void SpriteAsset::SetDefaultBlendColor(const Subsystem::Render::Drawing2D::SpriteColorComponents& color) noexcept
{
    m_stDefaultBlendColor = color;
    SyncBlendMode();
}

Subsystem::Asset::AssetTypeId SpriteAsset::GetAssetTypeId() const noexcept
{
    return GetAssetTypeIdStatic();
}

void SpriteAsset::UpdateResource() noexcept
{
    m_stSprite.UpdatePrecomputedVertex();

#if LSTG_ASSET_HOT_RELOAD
    UpdateVersion();
#endif
}

void SpriteAsset::SyncBlendMode(bool updateVertex) noexcept
{
    m_stSprite.SetColorBlendMode(m_stDefaultBlendMode.ColorBlend);
    if (m_stDefaultBlendMode.VertexColorBlend == VertexColorBlendMode::Additive)
    {
        m_stSprite.SetAdditiveBlendColor(m_stDefaultBlendColor, updateVertex);
        m_stSprite.SetMultiplyBlendColor(SpriteColorComponents { 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu }, updateVertex);
    }
    else
    {
        assert(m_stDefaultBlendMode.VertexColorBlend == VertexColorBlendMode::Multiply);
        m_stSprite.SetAdditiveBlendColor(SpriteColorComponents { 0x000000FFu, 0x000000FFu, 0x000000FFu, 0x000000FFu }, updateVertex);
        m_stSprite.SetMultiplyBlendColor(m_stDefaultBlendColor, updateVertex);
    }
}
