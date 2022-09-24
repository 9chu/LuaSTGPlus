/**
 * @file
 * @date 2022/4/12
 * @author 9chu
 * 此文件为 LuaSTGPlus 项目的一部分，版权与许可声明详见 COPYRIGHT.txt。
 */
#include <lstg/Core/Subsystem/Render/Texture.hpp>

#include <cassert>
#include <Texture.h>
#include <DeviceContext.h>
#include <lstg/Core/Logging.hpp>
#include <lstg/Core/Subsystem/Render/RenderDevice.hpp>
#include "detail/Texture2DFormats.hpp"

using namespace std;
using namespace lstg;
using namespace lstg::Subsystem::Render;

LSTG_DEF_LOG_CATEGORY(Texture);

Texture::Texture(RenderDevice& device, Diligent::ITexture* handler)
    : m_stDevice(device), m_pNativeHandler(handler)
{
    assert(handler);
    m_pNativeHandler->AddRef();
}

Texture::~Texture()
{
    if (m_pNativeHandler)
    {
        m_pNativeHandler->Release();
        m_pNativeHandler = nullptr;
    }
}

uint32_t Texture::GetWidth() const noexcept
{
    assert(m_pNativeHandler);
    return m_pNativeHandler->GetDesc().GetWidth();
}

uint32_t Texture::GetHeight() const noexcept
{
    assert(m_pNativeHandler);
    return m_pNativeHandler->GetDesc().GetHeight();
}

bool Texture::IsRenderTarget() const noexcept
{
    assert(m_pNativeHandler);
    return (m_pNativeHandler->GetDesc().BindFlags & Diligent::BIND_RENDER_TARGET) == Diligent::BIND_RENDER_TARGET;
}

bool Texture::IsDepthStencil() const noexcept
{
    assert(m_pNativeHandler);
    return (m_pNativeHandler->GetDesc().BindFlags & Diligent::BIND_DEPTH_STENCIL) == Diligent::BIND_DEPTH_STENCIL;
}

Result<void> Texture::Commit(Math::ImageRectangle range, Span<const uint8_t> data, size_t stride, size_t mipmapLevel,
    size_t arrayIndex) noexcept
{
    const auto& desc = m_pNativeHandler->GetDesc();

    // 检查是否允许进行操作
    if (desc.Usage == Diligent::USAGE_IMMUTABLE)
    {
        LSTG_LOG_ERROR_CAT(Texture, "Cannot update immutable texture");
        return make_error_code(errc::operation_not_permitted);
    }
    else if (desc.Usage != Diligent::USAGE_DEFAULT)
    {
        LSTG_LOG_ERROR_CAT(Texture, "Operation not supported yet");
        return make_error_code(errc::invalid_argument);
    }

    // 检查类型
    if (!desc.Is2D())
    {
        LSTG_LOG_ERROR_CAT(Texture, "Object is not a 2d texture");
        return make_error_code(errc::invalid_argument);
    }

    // 检查 Mipmap
    assert(desc.MipLevels != 0);
    if (mipmapLevel >= desc.MipLevels)
    {
        LSTG_LOG_ERROR_CAT(Texture, "Mipmap level {} is out of range {}", mipmapLevel, desc.MipLevels);
        return make_error_code(errc::invalid_argument);
    }

    // 检查数组
    if (!desc.IsArray() && arrayIndex != 0)
    {
        LSTG_LOG_ERROR_CAT(Texture, "Object is not an array texture");
        return make_error_code(errc::invalid_argument);
    }
    else if (desc.IsArray() && arrayIndex >= desc.GetArraySize())
    {
        LSTG_LOG_ERROR_CAT(Texture, "Array index {} is out of range {}", arrayIndex, desc.GetArraySize());
        return make_error_code(errc::invalid_argument);
    }

    // 检查范围
    Diligent::Box updateRange;
    updateRange.MinX = std::min<uint32_t>(desc.Width, range.Left());
    updateRange.MaxX = std::max(updateRange.MinX, std::min<uint32_t>(desc.Width, range.Left() + range.Width()));
    updateRange.MinY = std::min<uint32_t>(desc.Height, range.Top());
    updateRange.MaxY = std::max(updateRange.MinY, std::min<uint32_t>(desc.Height, range.Top() + range.Height()));
    auto componentSize = Render::detail::GetPixelComponentSize(Render::detail::FromDiligent(desc.Format));
    if (updateRange.Width() * componentSize > stride || updateRange.Height() * stride > data.GetSize())
    {
        LSTG_LOG_ERROR_CAT(Texture, "Invalid memory size");
        return make_error_code(errc::invalid_argument);
    }

    // 发起更新操作
    Diligent::TextureSubResData subResData;
    subResData.pData = data.GetData();
    subResData.Stride = stride;
    m_stDevice.GetImmediateContext()->UpdateTexture(m_pNativeHandler, mipmapLevel, arrayIndex, updateRange, subResData,
        Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
    return {};
}
