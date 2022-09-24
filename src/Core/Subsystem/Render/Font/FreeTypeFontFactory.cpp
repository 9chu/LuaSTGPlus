/**
 * @file
 * @date 2022/6/27
 * @author 9chu
 * 此文件为 LuaSTGPlus 项目的一部分，版权与许可声明详见 COPYRIGHT.txt。
 */
#include "FreeTypeFontFactory.hpp"

#include "FreeTypeFontFace.hpp"
#include "detail/FreeTypeError.hpp"

using namespace std;
using namespace lstg;
using namespace lstg::Subsystem::Render::Font;

FreeTypeFontFactory::FreeTypeFontFactory()
{
    auto ret = detail::FreeTypeObject::CreateLibrary();
    m_pLibrary = std::move(ret.ThrowIfError());
}

Result<FontFacePtr> FreeTypeFontFactory::CreateFontFace(VFS::StreamPtr stream, IFontDependencyLoader* dependencyLoader,
    int faceIndex) noexcept
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
        auto ret = ::FT_Open_Face(m_pLibrary.get(), &args, faceIndex, &face);
        if (ret != 0)
            return make_error_code(static_cast<detail::FreeTypeError>(ret));

        return make_shared<FreeTypeFontFace>(m_pLibrary, std::move(ftStream), detail::FreeTypeObject::FacePtr(face));
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

        detail::FreeTypeObject::FacePtr face;

        FT_Face tmpFace = nullptr;
        auto ret = ::FT_Open_Face(m_pLibrary.get(), &args, -1, &tmpFace);
        face.reset(tmpFace);
        if (ret != 0)
            return make_error_code(static_cast<detail::FreeTypeError>(ret));

        assert(face);
        auto faceCount = face->num_faces;

        out.reserve(faceCount);
        for (auto i = 0; i < faceCount; ++i)
        {
            tmpFace = nullptr;
            ::FT_Open_Face(m_pLibrary.get(), &args, i, &tmpFace);
            face.reset(tmpFace);
            if (face)
            {
                FontFaceInfo info;
                info.FamilyName = face->family_name;
                info.StyleName = face->style_name;
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
