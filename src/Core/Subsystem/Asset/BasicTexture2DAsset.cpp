/**
 * @file
 * @date 2022/4/26
 * @author 9chu
 * 此文件为 LuaSTGPlus 项目的一部分，版权与许可声明详见 COPYRIGHT.txt。
 */
#include <lstg/Core/Subsystem/Asset/BasicTexture2DAsset.hpp>

#include <lstg/Core/Logging.hpp>
#include <lstg/Core/AppBase.hpp>
#include <lstg/Core/Subsystem/AssetSystem.hpp>
#include <lstg/Core/Subsystem/RenderSystem.hpp>
#include "../Render/detail/Texture2DDataImpl.hpp"

using namespace std;
using namespace lstg;
using namespace lstg::Subsystem::Asset;

LSTG_DEF_LOG_CATEGORY(BasicTexture2DAsset);

AssetTypeId BasicTexture2DAsset::GetAssetTypeIdStatic() noexcept
{
    const auto& uniqueTypeName = Script::detail::GetUniqueTypeName<BasicTexture2DAsset>();
    return uniqueTypeName.Id;
}

BasicTexture2DAsset::BasicTexture2DAsset(std::string name, std::string path)
    : Asset(std::move(name)), m_stPath(std::move(path))
{}

uint32_t BasicTexture2DAsset::GetWidth() const noexcept
{
    if (m_stTextureInfo)
    {
        return m_stTextureInfo->Width;
    }
    else
    {
        const_cast<BasicTexture2DAsset*>(this)->InitTextureInfo();
        if (!m_stTextureInfo)
        {
            LSTG_LOG_WARN_CAT(BasicTexture2DAsset, "Fail to init texture info, asset name=\"{}\"", GetName());
            return RenderSystem::GetDefaultTexture2DSize().x;  // 加载失败，此时返回默认纹理宽度
        }
        else
        {
            return m_stTextureInfo->Width;
        }
    }
}

uint32_t BasicTexture2DAsset::GetHeight() const noexcept
{
    if (m_stTextureInfo)
    {
        return m_stTextureInfo->Height;
    }
    else
    {
        const_cast<BasicTexture2DAsset*>(this)->InitTextureInfo();
        if (!m_stTextureInfo)
        {
            LSTG_LOG_WARN_CAT(BasicTexture2DAsset, "Fail to init texture info, asset name=\"{}\"", GetName());
            return RenderSystem::GetDefaultTexture2DSize().y;  // 加载失败，此时返回默认纹理高度
        }
        else
        {
            return m_stTextureInfo->Height;
        }
    }
}

AssetTypeId BasicTexture2DAsset::GetAssetTypeId() const noexcept
{
    return GetAssetTypeIdStatic();
}

void BasicTexture2DAsset::InitTextureInfo() noexcept
{
    if (!m_stTextureInfo)
    {
        if (m_pTexture)
        {
            m_stTextureInfo = TextureInfo {
                m_pTexture->GetWidth(),
                m_pTexture->GetHeight(),
            };
        }
        else
        {
            // 此时，纹理正在加载，只能阻塞地再次打开文件获取信息
            LSTG_LOG_WARN_CAT(BasicTexture2DAsset, "Get texture info when asset is not ready may influence performance, asset={}",
                GetName());

            auto stream = AssetSystem::GetInstance().OpenAssetStream(m_stPath);
            if (!stream)
            {
                LSTG_LOG_ERROR_CAT(BasicTexture2DAsset, "Open asset stream from \"{}\" fail: {}", m_stPath, stream.GetError());
            }
            else
            {
                // 从流读取纹理信息
                uint32_t width = 0, height = 0;
                auto ret = Render::detail::Texture2DDataImpl::ReadImageInfoFromStream(width, height, *stream);
                if (!ret)
                    LSTG_LOG_ERROR_CAT(BasicTexture2DAsset, "Read image size from \"{}\" fail: {}", m_stPath, ret.GetError());
                else
                    m_stTextureInfo = TextureInfo { width, height };
            }
        }
    }
}

void BasicTexture2DAsset::ReceiveLoadedAsset(Render::TexturePtr tex) noexcept
{
    m_pTexture = std::move(tex);
    InitTextureInfo();
#if LSTG_ASSET_HOT_RELOAD
    UpdateVersion();  // 刷新版本号
#endif
}
