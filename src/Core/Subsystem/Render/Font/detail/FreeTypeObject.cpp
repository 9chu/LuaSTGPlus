/**
 * @file
 * @date 2022/6/29
 * @author 9chu
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#include "FreeTypeObject.hpp"

#include <freetype/ftbitmap.h>
#include "FreeTypeError.hpp"

using namespace std;
using namespace lstg;
using namespace lstg::Subsystem::Render::Font::detail;

Result<FreeTypeObject::LibraryPtr> FreeTypeObject::CreateLibrary() noexcept
{
    FT_Library library = nullptr;
    auto err = FT_Init_FreeType(&library);
    if (err != 0)
        return make_error_code(static_cast<FreeTypeError>(err));

    return shared_ptr<FT_LibraryRec_>(library, [](FT_Library lib) noexcept {
        auto err = ::FT_Done_FreeType(lib);
        assert(err == 0);
    });
}

/// <editor-fold desc="Bitmap">

FreeTypeObject::Bitmap::Bitmap(LibraryPtr library) noexcept
    : m_pLibrary(std::move(library))
{
    ::FT_Bitmap_Init(&m_stBitmap);
}

FreeTypeObject::Bitmap::Bitmap(Bitmap&& rhs) noexcept
    : m_pLibrary(rhs.m_pLibrary)
{
    m_stBitmap = rhs.m_stBitmap;
    ::FT_Bitmap_Init(&rhs.m_stBitmap);
}

FreeTypeObject::Bitmap::~Bitmap()
{
    auto err = ::FT_Bitmap_Done(m_pLibrary.get(), &m_stBitmap);
    assert(err == 0);
}

FreeTypeObject::Bitmap& FreeTypeObject::Bitmap::Bitmap::operator=(Bitmap&& rhs) noexcept
{
    if (this == &rhs)
        return *this;

    auto err = ::FT_Bitmap_Done(m_pLibrary.get(), &m_stBitmap);
    assert(err == 0);

    m_pLibrary = rhs.m_pLibrary;
    m_stBitmap = rhs.m_stBitmap;
    ::FT_Bitmap_Init(&rhs.m_stBitmap);
    return *this;
}

FT_Bitmap* FreeTypeObject::Bitmap::operator*() noexcept
{
    return &m_stBitmap;
}

Result<void> FreeTypeObject::Bitmap::ConvertFrom(const FT_Bitmap* original, uint32_t alignment) noexcept
{
    auto ret = ::FT_Bitmap_Convert(m_pLibrary.get(), original, &m_stBitmap, static_cast<FT_Int>(alignment));
    if (!ret)
        return make_error_code(static_cast<FreeTypeError>(ret));
    return {};
}

/// </editor-fold>
