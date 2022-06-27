/**
 * @file
 * @date 2022/6/27
 * @author 9chu
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#include "FreeTypeFontFactory.hpp"

#include "../../../detail/FreeTypeError.hpp"
#include "FreeTypeFontFace.hpp"

using namespace std;
using namespace lstg;
using namespace lstg::Subsystem::Render::Font;

FreeTypeFontFactory::FreeTypeFontFactory()
{
    auto err = ::FT_Init_FreeType(&m_pLibrary);
    if (err != 0)
        throw system_error(make_error_code(static_cast<lstg::detail::FreeTypeError>(err)));
}

FreeTypeFontFactory::~FreeTypeFontFactory()
{
    auto err = ::FT_Done_FreeType(m_pLibrary);
    assert(err == 0);
}

Result<FontFacePtr> FreeTypeFontFactory::CreateFontFace(VFS::StreamPtr stream, int faceIndex) noexcept
{
    try
    {
        // 创建流
        detail::FreeTypeStreamPtr ftStream = make_unique<detail::FreeTypeStream>(std::move(stream));

        // 创建字体
        FT_Open_Args args;
        ::memset(&args, 0, sizeof(args));
        args.flags = FT_OPEN_STREAM;
        args.stream = ftStream.get();

        FT_Face face = nullptr;
        auto ret = ::FT_Open_Face(m_pLibrary, &args, faceIndex, &face);
        if (ret != 0)
            return make_error_code(static_cast<lstg::detail::FreeTypeError>(ret));

        return make_shared<FreeTypeFontFace>(std::move(ftStream), face);
    }
    catch (const system_error& ex)
    {
        return ex.code();
    }
    catch (...)  // bad_alloc
    {
        return make_error_code(errc::not_enough_memory);
    }
}

Result<size_t> FreeTypeFontFactory::EnumFontFace(std::vector<FontFaceInfo>& out, VFS::StreamPtr stream) noexcept
{
    try
    {
        // 创建流
        detail::FreeTypeStreamPtr ftStream = make_unique<detail::FreeTypeStream>(std::move(stream));

        // 创建字体
        FT_Open_Args args;
        ::memset(&args, 0, sizeof(args));
        args.flags = FT_OPEN_STREAM;
        args.stream = ftStream.get();

        FT_Face face = nullptr;
        auto ret = ::FT_Open_Face(m_pLibrary, &args, -1, &face);
        if (ret != 0)
            return make_error_code(static_cast<lstg::detail::FreeTypeError>(ret));

        assert(face);
        auto faceCount = face->num_faces;
        ::FT_Done_Face(face);
        face = nullptr;

        out.reserve(faceCount);
        for (auto i = 0; i < faceCount; ++i)
        {
            ret = ::FT_Open_Face(m_pLibrary, &args, i, &face);
            if (face)
            {
                FontFaceInfo info;
                try
                {
                    info.FamilyName = face->family_name;
                    info.StyleName = face->style_name;
                }
                catch (...)
                {
                    ::FT_Done_Face(face);
                    throw;
                }
                ::FT_Done_Face(face);
                face = nullptr;
                out.emplace_back(std::move(info));
            }
        }
        return static_cast<size_t>(faceCount);
    }
    catch (const system_error& ex)
    {
        return ex.code();
    }
    catch (...)  // bad_alloc
    {
        return make_error_code(errc::not_enough_memory);
    }
}
