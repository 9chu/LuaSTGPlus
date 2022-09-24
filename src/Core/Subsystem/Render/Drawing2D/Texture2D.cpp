/**
 * @file
 * @date 2022/7/9
 * @author 9chu
 * 此文件为 LuaSTGPlus 项目的一部分，版权与许可声明详见 COPYRIGHT.txt。
 */
#include <lstg/Core/Subsystem/Render/Drawing2D/Texture2D.hpp>

#include <lstg/Core/Subsystem/RenderSystem.hpp>

using namespace std;
using namespace lstg;
using namespace lstg::Subsystem::Render::Drawing2D;

using namespace lstg::Subsystem::Render;

const TexturePtr& Texture2D::GetUnderlayTexture() const noexcept
{
    return m_pTexture;
}

void Texture2D::SetUnderlayTexture(TexturePtr tex) noexcept
{
    m_pTexture = std::move(tex);
}

float Texture2D::GetPixelPerUnit() const noexcept
{
    return m_fPPU;
}

void Texture2D::SetPixelPerUnit(float ppu) noexcept
{
    m_fPPU = ppu;
}

float Texture2D::GetWidth() const noexcept
{
    float width = m_pTexture ? static_cast<float>(m_pTexture->GetWidth()) : RenderSystem::GetDefaultTexture2DSize().x;
    return width / m_fPPU;
}

float Texture2D::GetHeight() const noexcept
{
    float height = m_pTexture ? static_cast<float>(m_pTexture->GetHeight()) : RenderSystem::GetDefaultTexture2DSize().y;
    return height / m_fPPU;
}
