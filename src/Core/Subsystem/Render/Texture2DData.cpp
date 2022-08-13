/**
 * @file
 * @date 2022/4/11
 * @author 9chu
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#include <lstg/Core/Subsystem/Render/Texture2DData.hpp>

#include <stb_image_write.h>
#include <lstg/Core/Logging.hpp>
#include "detail/Texture2DDataImpl.hpp"

using namespace std;
using namespace lstg;
using namespace lstg::Subsystem::Render;

LSTG_DEF_LOG_CATEGORY(Texture2DData);

Texture2DData::Texture2DData(VFS::StreamPtr stream)
    : m_pImpl(make_shared<detail::Texture2DDataImpl>(std::move(stream)))
{
}

Texture2DData::Texture2DData(uint32_t width, uint32_t height, Texture2DFormats format)
    : m_pImpl(make_shared<detail::Texture2DDataImpl>(width, height, format))
{
}

Texture2DData::Texture2DData(Texture2DData&& rhs) noexcept
    : m_pImpl(std::move(rhs.m_pImpl))
{
}

uint32_t Texture2DData::GetWidth() const noexcept
{
    assert(m_pImpl);
    return m_pImpl->GetWidth();
}

uint32_t Texture2DData::GetHeight() const noexcept
{
    assert(m_pImpl);
    return m_pImpl->GetHeight();
}

size_t Texture2DData::GetStride() const noexcept
{
    assert(m_pImpl);
    return m_pImpl->GetStride();
}

Texture2DFormats Texture2DData::GetFormat() const noexcept
{
    assert(m_pImpl);
    return m_pImpl->GetFormat();
}

Span<const uint8_t> Texture2DData::GetBuffer() const noexcept
{
    assert(m_pImpl);
    const detail::Texture2DDataImpl* p = m_pImpl.get();
    return p->GetBuffer();
}

Span<uint8_t> Texture2DData::GetBuffer() noexcept
{
    assert(m_pImpl);
    return m_pImpl->GetBuffer();
}

Result<void> Texture2DData::GenerateMipmap(size_t count) noexcept
{
    assert(m_pImpl);
    return m_pImpl->GenerateMipmap(count);
}

Result<void> Subsystem::Render::SaveToPng(Subsystem::VFS::IStream* out, const Texture2DData* data) noexcept
{
    assert(out && data);

    int comp;
    switch (data->GetFormat())
    {
        case Texture2DFormats::R8:
            comp = 1;
            break;
        case Texture2DFormats::R8G8:
            comp = 2;
            break;
        case Texture2DFormats::R8G8B8A8:
        case Texture2DFormats::R8G8B8A8_SRGB:
            comp = 4;
            break;
        default:
            LSTG_LOG_ERROR_CAT(Texture2DData, "Unsupported format for png saver, format={}", static_cast<int>(data->GetFormat()));
            return make_error_code(errc::invalid_argument);
    }

    struct WriteContext
    {
        Result<void> ErrorCode;
        VFS::IStream* Stream;
    } context { {}, out };

    auto ret = ::stbi_write_png_to_func([](void* context, void* data, int size) {
            auto writeContext = static_cast<WriteContext*>(context);
            assert(writeContext && writeContext->Stream);
            if (!writeContext->ErrorCode)
                return;
            writeContext->ErrorCode = writeContext->Stream->Write(reinterpret_cast<const uint8_t*>(data), static_cast<size_t>(size));
        },
        &context,
        static_cast<int>(data->GetWidth()),
        static_cast<int>(data->GetHeight()),
        comp,
        data->GetBuffer().GetData(),
        static_cast<int>(data->GetStride()));
    if (ret == 0)
    {
        LSTG_LOG_ERROR_CAT(Texture2DData, "Write PNG fail, ret={}", ret);
        return make_error_code(errc::invalid_argument);
    }
    return context.ErrorCode;
}
