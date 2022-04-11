/**
 * @file
 * @date 2022/4/11
 * @author 9chu
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#include <lstg/Core/Subsystem/Render/Texture2DData.hpp>

#include "detail/Texture2DDataImpl.hpp"

using namespace std;
using namespace lstg;
using namespace lstg::Subsystem::Render;

Texture2DData::Texture2DData(VFS::StreamPtr stream)
    : m_pImpl(make_shared<detail::Texture2DDataImpl>(std::move(stream)))
{
}

Texture2DData::Texture2DData(uint32_t width, uint32_t height, Texture2DFormats format)
    : m_pImpl(make_shared<detail::Texture2DDataImpl>(width, height, format))
{
}

uint32_t Texture2DData::GetWidth() const noexcept
{
    return m_pImpl->GetWidth();
}

uint32_t Texture2DData::GetHeight() const noexcept
{
    return m_pImpl->GetHeight();
}

size_t Texture2DData::GetStride() const noexcept
{
    return m_pImpl->GetStride();
}

Texture2DFormats Texture2DData::GetFormat() const noexcept
{
    return m_pImpl->GetFormat();
}

Span<const uint8_t> Texture2DData::GetBuffer() const noexcept
{
    const detail::Texture2DDataImpl* p = m_pImpl.get();
    return p->GetBuffer();
}

Span<uint8_t> Texture2DData::GetBuffer() noexcept
{
    return m_pImpl->GetBuffer();
}

Result<void> Texture2DData::GenerateMipmap(size_t count) noexcept
{
    return m_pImpl->GenerateMipmap(count);
}
