/**
 * @file
 * @date 2022/6/27
 * @author 9chu
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#include <lstg/Core/Subsystem/Render/Font/FontCollection.hpp>

using namespace std;
using namespace lstg;
using namespace lstg::Subsystem::Render::Font;

namespace
{
    optional<FontGlyphId> TryChooseFont(IFontFace* font, char32_t ch) noexcept
    {
        FontGlyphId glyphId[1];
        auto sz = font->BatchGetNominalGlyphs(Span<FontGlyphId, true>(glyphId, 1, sizeof(FontGlyphId)),
            Span<const char32_t, true>(&ch, 1, sizeof(char32_t)), false);
        if (sz == 0)
            return {};
        return glyphId[0];
    }
}

FontCollection::FontCollection(FontFacePtr primaryFont) noexcept
    : m_pPrimaryFont(std::move(primaryFont))
{
    assert(m_pPrimaryFont);
}

std::tuple<FontFacePtr, FontGlyphId, float> FontCollection::ChooseFontForChar(char32_t ch) const noexcept
{
    // 检查 Cache
    auto cache = m_pCharToFontCache.TryGet(ch);
    if (cache)
        return *cache;

    tuple<FontFacePtr, FontGlyphId, float> ret = { nullptr, 0, 0.f };

    // 使用主选字体
    std::optional<FontGlyphId> selectedGlyph;
    if ((selectedGlyph = TryChooseFont(m_pPrimaryFont.get(), ch)))
    {
        ret = { m_pPrimaryFont, *selectedGlyph, 1.f };
    }
    else
    {
        // 使用备选字体
        for (const auto& f : m_stFallbackFont)
        {
            if ((selectedGlyph = TryChooseFont(std::get<0>(f).get(), ch)))
            {
                ret = { std::get<0>(f), *selectedGlyph, std::get<1>(f) };
                break;
            }
        }
    }

    // 保存到 Cache
    try
    {
        m_pCharToFontCache.Emplace(ch, ret);
    }
    catch (...)  // bad_alloc
    {
        // ignore
    }
    return ret;
}

void FontCollection::AppendFallbackFont(FontFacePtr face, float scale)
{
    m_stFallbackFont.emplace_back(std::move(face), scale);
}
