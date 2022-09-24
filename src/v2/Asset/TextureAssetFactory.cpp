/**
 * @file
 * @date 2022/5/30
 * @author 9chu
 * 此文件为 LuaSTGPlus 项目的一部分，版权与许可声明详见 COPYRIGHT.txt。
 */
#include <lstg/v2/Asset/TextureAssetFactory.hpp>

#include <memory>
#include <lstg/Core/Logging.hpp>
#include <lstg/Core/Text/JsonHelper.hpp>
#include <lstg/Core/Subsystem/AssetSystem.hpp>
#include <lstg/v2/Asset/TextureAsset.hpp>
#include <lstg/v2/Asset/TextureAssetLoader.hpp>

using namespace std;
using namespace lstg;
using namespace lstg::v2::Asset;

using namespace lstg::Text;

LSTG_DEF_LOG_CATEGORY(TextureAssetFactory);

std::string_view TextureAssetFactory::GetAssetTypeName() const noexcept
{
    return "Texture";
}

Subsystem::Asset::AssetTypeId TextureAssetFactory::GetAssetTypeId() const noexcept
{
    return TextureAsset::GetAssetTypeIdStatic();
}

Result<Subsystem::Asset::CreateAssetResult> TextureAssetFactory::CreateAsset(Subsystem::AssetSystem& assetSystem,
    Subsystem::Asset::AssetPoolPtr pool, std::string_view name, const nlohmann::json& arguments,
    Subsystem::Asset::IAssetDependencyResolver* resolver) noexcept
{
    auto isRenderTarget = JsonHelper::ReadValue<bool>(arguments, "/rt", false);
    auto pixelPerUnit = JsonHelper::ReadValue<float>(arguments, "/ppu", 1.f);

    try
    {
        if (isRenderTarget)
        {
            auto& renderSystem = assetSystem.GetRenderSystem();

            // 获取当前的渲染大小
            auto width = renderSystem.GetRenderDevice()->GetRenderOutputWidth();
            auto height = renderSystem.GetRenderDevice()->GetRenderOutputHeight();

            // 创建 View
            auto view = CreateViews(width, height);

            // 创建 Asset
            auto asset = make_shared<TextureAsset>(string{name}, std::move(view.ThrowIfError()), pixelPerUnit);
            auto loader = make_shared<TextureAssetLoader>(asset);

            // 记录 RT
            m_stRenderTargets.push_back(asset);

            return Subsystem::Asset::CreateAssetResult {
                static_pointer_cast<Subsystem::Asset::Asset>(asset),
                static_pointer_cast<Subsystem::Asset::AssetLoader>(loader)
            };
        }
        else
        {
            auto basicTexture = assetSystem.CreateAsset<Subsystem::Asset::BasicTexture2DAsset>(pool, {}, arguments);
            auto asset = make_shared<TextureAsset>(string{name}, std::move(basicTexture.ThrowIfError()), pixelPerUnit);
            auto loader = make_shared<TextureAssetLoader>(asset);
            return Subsystem::Asset::CreateAssetResult {
                static_pointer_cast<Subsystem::Asset::Asset>(asset),
                static_pointer_cast<Subsystem::Asset::AssetLoader>(loader)
            };
        }
    }
    catch (const std::system_error& ex)
    {
        return ex.code();
    }
    catch (...)
    {
        return make_error_code(errc::not_enough_memory);
    }
}

void TextureAssetFactory::ResizeRenderTarget(uint32_t width, uint32_t height) noexcept
{
    auto it = m_stRenderTargets.begin();
    while (it != m_stRenderTargets.end())
    {
        auto tex = it->lock();

        // 删除已经释放的 Texture
        if (!tex)
        {
            it = m_stRenderTargets.erase(it);
            continue;
        }

        assert(tex->IsRenderTarget());
        auto& view = tex->GetOutputViews();

        // 大小一样的情况下不需要调整
        if (view.ColorView->GetWidth() == width && view.ColorView->GetHeight() == height)
        {
            assert(view.DepthStencilView->GetWidth() == view.ColorView->GetWidth());
            assert(view.DepthStencilView->GetHeight() == view.ColorView->GetHeight());

            ++it;
            continue;
        }

        // 创建新的 View
        auto newView = CreateViews(width, height);
        if (!newView)
        {
            LSTG_LOG_ERROR_CAT(TextureAssetFactory, "Fail to resize render target {}", tex->GetName());
            ++it;
            continue;
        }

        tex->GetOutputViews().ColorView = std::move(newView->ColorView);
        tex->GetOutputViews().DepthStencilView = std::move(newView->DepthStencilView);

        ++it;
    }
}

Result<Subsystem::Render::Camera::OutputViews> TextureAssetFactory::CreateViews(uint32_t width, uint32_t height) noexcept
{
    auto& renderSystem = Subsystem::AssetSystem::GetInstance().GetRenderSystem();

    // 创建 RT
    auto rt = renderSystem.CreateRenderTarget(width, height);
    if (!rt)
    {
        LSTG_LOG_ERROR_CAT(TextureAssetFactory, "Create render target {}x{} fail: {}", width, height, rt.GetError());
        return rt.GetError();
    }
    auto ds = renderSystem.CreateDepthStencil(width, height);
    if (!ds)
    {
        LSTG_LOG_ERROR_CAT(TextureAssetFactory, "Create depth stencil buffer {}x{} fail: {}", width, height, rt.GetError());
        return rt.GetError();
    }

    // 创建 View
    return Subsystem::Render::Camera::OutputViews { std::move(*rt), std::move(*ds) };
}
