/**
 * @file
 * @date 2022/5/30
 * @author 9chu
 * 此文件为 LuaSTGPlus 项目的一部分，版权与许可声明详见 COPYRIGHT.txt。
 */
#include <lstg/v2/Asset/TextureAsset.hpp>

#include <lstg/Core/Subsystem/Script/LuaStack.hpp>
#include <lstg/Core/Subsystem/Asset/AssetPool.hpp>

using namespace std;
using namespace lstg;
using namespace lstg::v2::Asset;

Subsystem::Asset::AssetTypeId TextureAsset::GetAssetTypeIdStatic() noexcept
{
    const auto& uniqueTypeName = Subsystem::Script::detail::GetUniqueTypeName<TextureAsset>();
    return uniqueTypeName.Id;
}

TextureAsset::TextureAsset(std::string name, Subsystem::Asset::BasicTexture2DAssetPtr textureAsset, float pixelPerUnit)
    : Subsystem::Asset::Asset(std::move(name)), m_stUnderlay(std::move(textureAsset))
{
    assert(std::get<0>(m_stUnderlay));
    m_stDrawingTexture.SetUnderlayTexture(std::get<0>(m_stUnderlay)->GetTexture());
    m_stDrawingTexture.SetPixelPerUnit(std::abs(pixelPerUnit) <= std::numeric_limits<float>::epsilon() ? 1.f : pixelPerUnit);
}

TextureAsset::TextureAsset(std::string name, Subsystem::Render::Camera::OutputViews view, float pixelPerUnit)
    : Subsystem::Asset::Asset(std::move(name)), m_stUnderlay(std::move(view))
{
    assert(std::get<1>(m_stUnderlay).ColorView);
    m_stDrawingTexture.SetUnderlayTexture(std::get<1>(m_stUnderlay).ColorView);
    m_stDrawingTexture.SetPixelPerUnit(std::abs(pixelPerUnit) <= std::numeric_limits<float>::epsilon() ? 1.f : pixelPerUnit);
}

TextureAsset::~TextureAsset()
{
    // 非 RT 资源：检查资源是否已经释放
    assert(IsRenderTarget() || std::get<0>(m_stUnderlay) == nullptr ||
        std::get<0>(m_stUnderlay)->GetId() == Subsystem::Asset::kEmptyAssetId);
}

bool TextureAsset::IsRenderTarget() const noexcept
{
    return m_stUnderlay.index() == 1;
}

const Subsystem::Asset::BasicTexture2DAssetPtr& TextureAsset::GetBasicTextureAsset() const noexcept
{
    assert(!IsRenderTarget());
    return std::get<0>(m_stUnderlay);
}

Subsystem::Render::Camera::OutputViews& TextureAsset::GetOutputViews() noexcept
{
    assert(IsRenderTarget());
    return std::get<1>(m_stUnderlay);
}

double TextureAsset::GetWidth() const noexcept
{
    if (!IsRenderTarget() && !m_stDrawingTexture.GetUnderlayTexture())
    {
        // 如果异步加载未完成，需要向 Asset 进行请求，阻塞地获取纹理大小
        return std::get<0>(m_stUnderlay)->GetWidth() / m_stDrawingTexture.GetPixelPerUnit();
    }
    return m_stDrawingTexture.GetWidth();
}

double TextureAsset::GetHeight() const noexcept
{
    if (!IsRenderTarget() && !m_stDrawingTexture.GetUnderlayTexture())
    {
        // 如果异步加载未完成，需要向 Asset 进行请求，阻塞地获取纹理大小
        return std::get<0>(m_stUnderlay)->GetHeight() / m_stDrawingTexture.GetPixelPerUnit();
    }
    return m_stDrawingTexture.GetHeight();
}

Subsystem::Asset::AssetTypeId TextureAsset::GetAssetTypeId() const noexcept
{
    return GetAssetTypeIdStatic();
}

void TextureAsset::OnRemove() noexcept
{
    FreeResource();
}

void TextureAsset::FreeResource() noexcept
{
    if (IsRenderTarget())
        return;

    // 释放关联资源
    auto& textureAsset = std::get<0>(m_stUnderlay);
    if (textureAsset && !textureAsset->IsWildAsset())
    {
        assert(textureAsset->GetId() != Subsystem::Asset::kEmptyAssetId);

        auto pool = textureAsset->GetPool().lock();
        if (pool)
            pool->RemoveAsset(textureAsset->GetId());
        textureAsset.reset();
    }
}

void TextureAsset::UpdateResource() noexcept
{
    // 纹理可能重新创建了
    if (IsRenderTarget())
    {
        auto& textureAsset = std::get<1>(m_stUnderlay);
        m_stDrawingTexture.SetUnderlayTexture(textureAsset.ColorView);
    }
    else
    {
        auto& textureAsset = std::get<0>(m_stUnderlay);
        m_stDrawingTexture.SetUnderlayTexture(textureAsset->GetTexture());
    }

#if LSTG_ASSET_HOT_RELOAD
    UpdateVersion();
#endif
}
