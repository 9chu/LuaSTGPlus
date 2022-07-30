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

using namespace lstg::Subsystem::Render::Drawing2D;

Subsystem::Asset::AssetTypeId SpriteSequenceAsset::GetAssetTypeIdStatic() noexcept
{
    const auto& uniqueTypeName = Subsystem::Script::detail::GetUniqueTypeName<SpriteSequenceAsset>();
    return uniqueTypeName.Id;
}

SpriteSequenceAsset::SpriteSequenceAsset(std::string name, TextureAssetPtr texture, SequenceContainer frames, int32_t interval,
    ColliderShape colliderShape)
    : Subsystem::Asset::Asset(std::move(name)), m_pTextureAsset(std::move(texture)), m_stSequences(std::move(frames)),
    m_iInterval(std::max<int32_t>(1, interval)), m_stColliderShape(colliderShape)
{
    assert(!m_stSequences.empty());

    // Factory 只需要初始化 Frames，在这里填充其他参数
    // 以第一个 Frame 来标记中心点
    auto anchor = m_stSequences[0].GetFrame().GetCenter() - m_stSequences[0].GetFrame().GetTopLeft();
    for (auto& f : m_stSequences)
    {
        f.SetAnchor(anchor, false);
        f.SetTexture2D(&m_pTextureAsset->GetDrawingTexture(), false);
    }
    SyncBlendMode(false);

    // 统一刷新顶点
    for (auto& f : m_stSequences)
        f.UpdatePrecomputedVertex();
}

const glm::vec2& SpriteSequenceAsset::GetAnchor() const noexcept
{
    return m_stSequences[0].GetAnchor();
}

void SpriteSequenceAsset::SetAnchor(glm::vec2 vec) noexcept
{
    for (auto& f : m_stSequences)
        f.SetAnchor(vec);
}

void SpriteSequenceAsset::SetDefaultBlendMode(BlendMode mode) noexcept
{
    m_stDefaultBlendMode = mode;
    SyncBlendMode();
}

void SpriteSequenceAsset::SetDefaultBlendColor(const Subsystem::Render::Drawing2D::SpriteColorComponents& color) noexcept
{
    m_stDefaultBlendColor = color;
    SyncBlendMode();
}

Subsystem::Asset::AssetTypeId SpriteSequenceAsset::GetAssetTypeId() const noexcept
{
    return GetAssetTypeIdStatic();
}

void SpriteSequenceAsset::UpdateResource() noexcept
{
    for (auto& f : m_stSequences)
        f.UpdatePrecomputedVertex();

#if LSTG_ASSET_HOT_RELOAD
    UpdateVersion();
#endif
}

void SpriteSequenceAsset::SyncBlendMode(bool updateVertex) noexcept
{
    for (auto& f : m_stSequences)
    {
        f.SetColorBlendMode(m_stDefaultBlendMode.ColorBlend);
        if (m_stDefaultBlendMode.VertexColorBlend == VertexColorBlendMode::Additive)
        {
            f.SetAdditiveBlendColor(m_stDefaultBlendColor, false);
            f.SetMultiplyBlendColor(SpriteColorComponents { 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu, 0xFFFFFFFFu }, updateVertex);
        }
        else
        {
            assert(m_stDefaultBlendMode.VertexColorBlend == VertexColorBlendMode::Multiply);
            f.SetAdditiveBlendColor(SpriteColorComponents { 0x000000FFu, 0x000000FFu, 0x000000FFu, 0x000000FFu }, false);
            f.SetMultiplyBlendColor(m_stDefaultBlendColor, updateVertex);
        }
    }
}
