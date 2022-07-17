/**
* @file
* @date 2022/6/30
* @author 9chu
* 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
*/
#include <lstg/Core/Subsystem/Render/Font/DynamicFontGlyphAtlas.hpp>

#include <lstg/Core/Subsystem/RenderSystem.hpp>

using namespace std;
using namespace lstg;
using namespace lstg::Subsystem::Render::Font;

using namespace lstg::Subsystem::Render::Font::detail;

static const unsigned kSlotMargin = 1;
static const unsigned kSlotMinHeight = 10;
static const unsigned kPixelSize = 4;
static const unsigned kAtlasTextureWidth = 2048;
static const unsigned kAtlasTextureHeight = 2048;

namespace
{
    AtlasSlotKey MakeSlotKey(IFontFace* face, const FontGlyphRasterParam& param, FontGlyphId id) noexcept
    {
        AtlasSlotKey ret;
        ret.Face = face;
        ret.Param = param;
        ret.GlyphId = id;
        return ret;
    }
}

// <editor-fold desc="detail::AtlasTexture">

AtlasTexture::AtlasTexture(RenderSystem& renderSystem, uint32_t width, uint32_t height)
    : TextureData(width, height, Render::Texture2DFormats::R8G8B8A8)
{
    auto ret = renderSystem.CreateDynamicTexture2D(width, height, Render::Texture2DFormats::R8G8B8A8);
    Texture = std::move(ret.ThrowIfError());
    ImageDirtyRegion = { 0, 0, 0, 0 };
}

// </editor-fold>

DynamicFontGlyphAtlas::DynamicFontGlyphAtlas(RenderSystem& renderSystem) noexcept
    : m_stRenderSystem(renderSystem)
{
}

DynamicFontGlyphAtlas::~DynamicFontGlyphAtlas()
{
    Reset();
}

Subsystem::Render::TexturePtr DynamicFontGlyphAtlas::GetAtlasTexture(size_t index) const noexcept
{
    if (index >= m_stAtlasList.size())
        return nullptr;

    auto it = m_stAtlasList.begin();
    std::advance(it, index);
    return it->Texture;
}

bool DynamicFontGlyphAtlas::FindGlyph(FontGlyphAtlasInfo& out, IFontFace* fontFace, FontGlyphRasterParam param,
    FontGlyphId glyphId) const noexcept
{
    assert(fontFace);

    auto slotKey = MakeSlotKey(fontFace, param, glyphId);
    auto it = m_stLookupTable.find(slotKey);
    if (it == m_stLookupTable.end())
        return false;

    auto slot = it->second;
    assert(slot);
    out.Texture = slot->Parent->Parent->Texture;
    out.TextureRect = slot->TextureRect;
    out.DrawOffset = slot->DrawOffset;
    out.DrawSize = { slot->SlotWidth, slot->SlotHeight };
    return true;
}

Result<FontGlyphAtlasInfo> DynamicFontGlyphAtlas::CacheGlyphFromBGRA(std::shared_ptr<IFontFace> fontFace, FontGlyphRasterParam param,
    FontGlyphId glyphId, BitmapSource source) noexcept
{
    return CacheGlyphFrom(std::move(fontFace), param, glyphId, source, false, 0);
}

Result<FontGlyphAtlasInfo> DynamicFontGlyphAtlas::CacheGlyphFromGrayscale(std::shared_ptr<IFontFace> fontFace, FontGlyphRasterParam param,
    FontGlyphId glyphId, BitmapSource source, uint32_t grays) noexcept
{
    return CacheGlyphFrom(std::move(fontFace), param, glyphId, source, true, grays);
}

void DynamicFontGlyphAtlas::Reset() noexcept
{
    // 回收内存
    for (auto& atlas : m_stAtlasList)
    {
        for (auto& strip : atlas.SlotRowHeaders)
        {
            while (strip.SlotListHead)
            {
                auto p = strip.SlotListHead;
                strip.SlotListHead = p->Next;
                delete p;
            }
        }
        atlas.SlotRowHeaders.clear();
    }
    m_stAtlasList.clear();
    m_stLookupTable.clear();
}

Result<void> DynamicFontGlyphAtlas::Commit() noexcept
{
    for (auto& atlas : m_stAtlasList)
    {
        if (atlas.ImageDirtyRegion.Width() != 0 && atlas.ImageDirtyRegion.Height() != 0)
        {
            auto ret = atlas.Texture->Commit({ atlas.ImageDirtyRegion.Left(), atlas.ImageDirtyRegion.Top(), atlas.ImageDirtyRegion.Width(),
                atlas.ImageDirtyRegion.Height() }, { atlas.TextureData.GetBuffer().GetData(), atlas.TextureData.GetBuffer().GetSize() },
                atlas.TextureData.GetStride());
            if (!ret)
                return ret.GetError();
            atlas.ImageDirtyRegion = { 0, 0, 0, 0 };
        }
    }
    return {};
}

Result<AtlasSlot*> DynamicFontGlyphAtlas::AllocSlotInTexture(AtlasTexture& atlasTexture, uint32_t width, uint32_t height,
    uint32_t stripHeight) noexcept
{
    auto texWidth = atlasTexture.TextureData.GetWidth();
    auto texHeight = atlasTexture.TextureData.GetHeight();
    if (texWidth < width + kSlotMargin || texHeight < stripHeight + kSlotMargin)
        return make_error_code(errc::invalid_argument);

    // 先搜索各个 Strip
    AtlasSlotStrip* lastStrip = nullptr;
    auto it = atlasTexture.SlotRowHeaders.begin();
    while (it != atlasTexture.SlotRowHeaders.end())
    {
        lastStrip = &(*it);

        if (it->StripHeight == stripHeight)
        {
            // 找到一个可以放置的 Span
            AtlasSlot* slot = nullptr;
            AtlasSlot* leftSlot = it->SlotListHead;
            while (leftSlot)
            {
                auto spanLeft = leftSlot->SlotLeft + leftSlot->SlotWidth + kSlotMargin;
                auto spanRight = leftSlot->Next ? leftSlot->Next->SlotLeft : texWidth;
                if (spanRight - spanLeft >= width + kSlotMargin)
                    break;

                if (leftSlot->Next)
                    leftSlot = leftSlot->Next;  // 尝试下一个 Slot
                else
                    goto CONTINUE;  // 这行最后一个 Slot，且剩余的空间不足以存储
            }

            // 此时一定可以分配 Slot
            try
            {
                slot = new AtlasSlot();
                slot->Parent = &(*it);
                slot->SlotWidth = width;
                slot->SlotHeight = height;
            }
            catch (...)
            {
                return make_error_code(errc::not_enough_memory);
            }

            if (leftSlot == nullptr)
            {
                // 此时这一行都是空的，并且必然能塞下 Slot
                assert(it->SlotListHead == nullptr);
                assert(width + kSlotMargin <= texWidth);

                it->SlotListHead = slot;

                slot->SlotLeft = 0;
            }
            else
            {
                assert(leftSlot->SlotLeft + leftSlot->SlotWidth + kSlotMargin + width + kSlotMargin <= texWidth);

                slot->Prev = leftSlot;
                slot->Next = leftSlot->Next;
                if (leftSlot->Next)
                    leftSlot->Next->Prev = slot;
                leftSlot->Next = slot;

                slot->SlotLeft = leftSlot->SlotLeft + leftSlot->SlotWidth + kSlotMargin;
            }
            return slot;
        }

    CONTINUE:
        ++it;
    }

    // 如果没有找到合适的 Strip，则检查剩下的空间是否足够
    try
    {
        if (lastStrip == nullptr)
        {
            // 此时是空纹理
            assert(atlasTexture.SlotRowHeaders.empty());

            AtlasSlotStrip strip;
            strip.Parent = &atlasTexture;
            strip.StripTop = 0;
            strip.StripHeight = stripHeight;
            atlasTexture.SlotRowHeaders.push_back(strip);
            lastStrip = &atlasTexture.SlotRowHeaders.back();
        }
        else
        {
            // 检查剩余空间
            auto nextTop = lastStrip->StripTop + lastStrip->StripHeight + kSlotMargin;
            if (nextTop + stripHeight + kSlotMargin > texHeight)
                return nullptr;

            AtlasSlotStrip strip;
            strip.Parent = &atlasTexture;
            strip.StripTop = nextTop;
            strip.StripHeight = stripHeight;
            atlasTexture.SlotRowHeaders.push_back(strip);
            lastStrip = &atlasTexture.SlotRowHeaders.back();
        }

        // 在新创建的条带上创建 Slot
        auto slot = new AtlasSlot();
        slot->Parent = lastStrip;
        slot->SlotLeft = 0;
        slot->SlotWidth = width;
        slot->SlotHeight = height;
        lastStrip->SlotListHead = slot;
        return slot;
    }
    catch (...)
    {
        return make_error_code(errc::not_enough_memory);
    }
}

Result<DynamicFontGlyphAtlas::LookupTableContainer::iterator> DynamicFontGlyphAtlas::AllocSlot(const detail::AtlasSlotKey& key,
    const BitmapSource& source) noexcept
{
    AtlasSlot* slot = nullptr;
    enum {
        STATE_ALLOC,
        STATE_ALLOC_AFTER_CLEANUP,
        STATE_ALLOC_AFTER_CREATE,
    } state = STATE_ALLOC;

    auto stripHeight = (std::max(1u, source.Height) + kSlotMinHeight - 1) / kSlotMinHeight * kSlotMinHeight;
    while (true)
    {
        // 根据大小在各个 Texture 中尝试分配 Slot
        for (auto& atlas : m_stAtlasList)
        {
            auto ret = AllocSlotInTexture(atlas, source.Width, source.Height, stripHeight);
            if (!ret)
                return ret.GetError();
            slot = *ret;
            if (slot)
                break;
        }
        if (slot)
            break;

        if (state == STATE_ALLOC)
        {
            state = STATE_ALLOC_AFTER_CLEANUP;

            // 首先尝试清理垃圾
            auto cleanUpCount = DeleteUnused();
            if (cleanUpCount != 0)
                continue;  // 没能腾出空间的情况下直接创建新纹理
        }
        if (state == STATE_ALLOC_AFTER_CLEANUP)
        {
            // 如果清理后也没能分配，则创建纹理
            try
            {
                m_stAtlasList.emplace_back(m_stRenderSystem, kAtlasTextureWidth, kAtlasTextureHeight);
            }
            catch (const system_error& ex)
            {
                return ex.code();
            }
            catch (...)  // bad_alloc
            {
                return make_error_code(errc::not_enough_memory);
            }

            state = STATE_ALLOC_AFTER_CREATE;
            continue;
        }

        // 正常情况下不应该走到这个分支，除非申请的大小超过了纹理的大小
        assert(state == STATE_ALLOC_AFTER_CREATE);
        return make_error_code(errc::invalid_argument);
    }

    assert(slot);

    // 记录到查找表
    try
    {
        auto ret = m_stLookupTable.emplace(key, slot);
        return ret.first;
    }
    catch (...)  // bad_alloc
    {
        DeleteSlot(slot);
        return make_error_code(errc::not_enough_memory);
    }
}

DynamicFontGlyphAtlas::LookupTableContainer::iterator DynamicFontGlyphAtlas::DeleteSlot(LookupTableContainer::iterator it) noexcept
{
    assert(it != m_stLookupTable.end());
    auto slot = it->second;
    DeleteSlot(slot);

    // 最后删掉迭代器
    return m_stLookupTable.erase(it);
}

void DynamicFontGlyphAtlas::DeleteSlot(detail::AtlasSlot* slot) noexcept
{
    assert(slot);

    // 断开 Linklist
    if (slot->Prev)
        slot->Prev->Next = slot->Next;
    if (slot->Next)
        slot->Next->Prev = slot->Prev;

    auto strip = slot->Parent;
    assert(strip);
    if (slot->Parent->SlotListHead == slot)
        slot->Parent->SlotListHead = slot->Next;

    // 删除 Slot
    delete slot;

    // 如果 Strip 中没有 Slot，且 Strip 是纹理中最后一个 Strip，则删掉连续的空 Strip
    if (strip->SlotListHead == nullptr)
    {
        auto texture = strip->Parent;
        while (!texture->SlotRowHeaders.empty() && texture->SlotRowHeaders.back().SlotListHead == nullptr)
            texture->SlotRowHeaders.pop_back();
    }
}

size_t DynamicFontGlyphAtlas::DeleteUnused() noexcept
{
    size_t cnt = 0;
    for (auto it = m_stLookupTable.begin(); it != m_stLookupTable.end(); )
    {
        auto slot = it->second;
        if (slot->Face.expired())
        {
            ++cnt;
            it = DeleteSlot(it);
        }
        else
        {
            ++it;
        }
    }
    return cnt;
}

Result<FontGlyphAtlasInfo> DynamicFontGlyphAtlas::CacheGlyphFrom(std::shared_ptr<IFontFace> fontFace, FontGlyphRasterParam param,
    FontGlyphId glyphId, BitmapSource source, bool isGrayscale, uint32_t grays) noexcept
{
    assert(fontFace);

    // 如果 Slot 已经存在，则需要删除
    auto slotKey = MakeSlotKey(fontFace.get(), param, glyphId);
    auto it = m_stLookupTable.find(slotKey);
    if (it != m_stLookupTable.end())
        DeleteSlot(std::move(it));

    // 分配 Slot
    auto newSlot = AllocSlot(slotKey, source);
    if (!newSlot)
        return newSlot.GetError();
    it = *newSlot;
    auto slot = it->second;
    assert(slot);
    auto strip = slot->Parent;
    assert(strip);
    auto atlas = strip->Parent;
    assert(atlas);
    auto destBuffer = atlas->TextureData.GetBuffer();

    // 填充空白数据
    {
        auto dest = destBuffer.GetData() + slot->SlotLeft * kPixelSize + strip->StripTop * atlas->TextureData.GetStride();
        for (uint32_t y = 0; y < (source.Height + kSlotMargin); ++y)
        {
            assert(dest + (source.Width + kSlotMargin) * kPixelSize <= destBuffer.GetData() + destBuffer.GetSize());
            ::memset(dest, 0, (source.Width + kSlotMargin) * kPixelSize);
            dest += atlas->TextureData.GetStride();
        }
    }

    // 填充纹理数据
    {
        assert(!isGrayscale || grays > 1);
        auto grayBoost = isGrayscale ? 255 / (grays - 1) : 1;

        auto src = source.Buffer;
        auto dest = destBuffer.GetData() + slot->SlotLeft * kPixelSize + strip->StripTop * atlas->TextureData.GetStride();
        for (uint32_t y = 0; y < source.Height; ++y)
        {
            assert(dest + source.Width * kPixelSize <= destBuffer.GetData() + destBuffer.GetSize());

            if (isGrayscale)
            {
                for (uint32_t x = 0; x < source.Width; ++x)
                {
                    auto srcGray = src[x];
                    if (grays != 256)
                        srcGray *= grayBoost;

                    *(dest + x * kPixelSize) = srcGray > 0 ? 255 : 0;
                    *(dest + x * kPixelSize + 1) = srcGray > 0 ? 255 : 0;
                    *(dest + x * kPixelSize + 2) = srcGray > 0 ? 255 : 0;
                    *(dest + x * kPixelSize + 3) = srcGray;
                }
            }
            else
            {
                // 复制一行
                ::memcpy(dest, src, source.Width * kPixelSize);

                // BGRA -> RGBA
                for (uint32_t x = 0; x < source.Width; ++x)
                    std::swap(*(dest + x * kPixelSize), *(dest + x * kPixelSize + 2));
            }

            src += source.Stride;
            dest += atlas->TextureData.GetStride();
        }
    }

    // 刷新 Dirty Rect
    Math::ImageRectangle dirtyRange { slot->SlotLeft, strip->StripTop, source.Width + kSlotMargin, source.Height + kSlotMargin };
    if (atlas->ImageDirtyRegion.Width() == 0 || atlas->ImageDirtyRegion.Height() == 0)
        atlas->ImageDirtyRegion = dirtyRange;
    else
        atlas->ImageDirtyRegion = atlas->ImageDirtyRegion.Union(dirtyRange);

    // 填充纹理信息
    slot->Face = fontFace;
    slot->TextureRect = {
        static_cast<float>(slot->SlotLeft) / static_cast<float>(atlas->TextureData.GetWidth()),
        static_cast<float>(strip->StripTop) / static_cast<float>(atlas->TextureData.GetHeight()),
        static_cast<float>(slot->SlotWidth) / static_cast<float>(atlas->TextureData.GetWidth()),
        static_cast<float>(slot->SlotHeight) / static_cast<float>(atlas->TextureData.GetHeight()),
    };
    slot->DrawOffset = { source.DrawLeftOffset, source.DrawTopOffset };

    // 构造返回值
    FontGlyphAtlasInfo ret;
    ret.Texture = slot->Parent->Parent->Texture;
    ret.TextureRect = slot->TextureRect;
    ret.DrawOffset = slot->DrawOffset;
    ret.DrawSize = { slot->SlotWidth, slot->SlotHeight };
    return ret;
}
