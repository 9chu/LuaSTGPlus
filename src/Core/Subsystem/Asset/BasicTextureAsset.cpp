/**
 * @file
 * @date 2022/4/26
 * @author 9chu
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#include <lstg/Core/Subsystem/Asset/BasicTextureAsset.hpp>

#include <lstg/Core/Logging.hpp>
#include <lstg/Core/AppBase.hpp>
#include <lstg/Core/Subsystem/AssetSystem.hpp>
#include <lstg/Core/Subsystem/RenderSystem.hpp>
#include "../Render/detail/Texture2DDataImpl.hpp"

using namespace std;
using namespace lstg;
using namespace lstg::Subsystem::Asset;

LSTG_DEF_LOG_CATEGORY(BasicTextureAsset);

AssetTypeId BasicTextureAsset::GetAssetTypeIdStatic() noexcept
{
    const auto& uniqueTypeName = Script::detail::GetUniqueTypeName<BasicTextureAsset>();
    return uniqueTypeName.Id;
}

BasicTextureAsset::BasicTextureAsset(std::string name, std::string path)
    : Asset(std::move(name)), m_stPath(std::move(path))
{}

uint32_t BasicTextureAsset::GetWidth() const noexcept
{
    if (m_stTextureInfo)
    {
        return m_stTextureInfo->Width;
    }
    else
    {
        const_cast<BasicTextureAsset*>(this)->InitTextureInfo();
        if (!m_stTextureInfo)
        {
            LSTG_LOG_WARN_CAT(BasicTextureAsset, "Fail to init texture info, asset name=\"{}\"", GetName());
            return 0;
        }
        else
        {
            return m_stTextureInfo->Width;
        }
    }
}

uint32_t BasicTextureAsset::GetHeight() const noexcept
{
    if (m_stTextureInfo)
    {
        return m_stTextureInfo->Height;
    }
    else
    {
        const_cast<BasicTextureAsset*>(this)->InitTextureInfo();
        if (!m_stTextureInfo)
        {
            LSTG_LOG_WARN_CAT(BasicTextureAsset, "Fail to init texture info, asset name=\"{}\"", GetName());
            return 0;
        }
        else
        {
            return m_stTextureInfo->Height;
        }
    }
}

AssetTypeId BasicTextureAsset::GetAssetTypeId() const noexcept
{
    return GetAssetTypeIdStatic();
}

void BasicTextureAsset::InitTextureInfo() noexcept
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
            LSTG_LOG_WARN_CAT(BasicTextureAsset, "Get texture info when asset is not ready may influence performance, asset={}", GetName());

            auto stream = AssetSystem::GetInstance().OpenAssetStream(m_stPath);
            if (!stream)
            {
                LSTG_LOG_ERROR_CAT(BasicTextureAsset, "Open asset stream from \"{}\" fail: {}", m_stPath, stream.GetError());
            }
            else
            {
                // 从流读取纹理信息
                uint32_t width, height;
                auto ret = Render::detail::Texture2DDataImpl::ReadImageInfoFromStream(width, height, *stream);
                if (!ret)
                    LSTG_LOG_ERROR_CAT(BasicTextureAsset, "Read image size from \"{}\" fail: {}", m_stPath, ret.GetError());
                else
                    m_stTextureInfo = TextureInfo { width, height };
            }
        }
    }
}

void BasicTextureAsset::ReceiveLoadedAsset(Render::TexturePtr tex) noexcept
{
    m_pTexture = std::move(tex);
    InitTextureInfo();
}
