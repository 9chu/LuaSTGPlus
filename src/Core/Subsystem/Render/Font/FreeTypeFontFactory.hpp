/**
 * @file
 * @date 2022/6/27
 * @author 9chu
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#pragma once
#include <freetype/freetype.h>
#include <lstg/Core/Subsystem/Render/Font/IFontFactory.hpp>

namespace lstg::Subsystem::Render::Font
{
    /**
     * FreeType 字体工厂
     */
    class FreeTypeFontFactory :
        public IFontFactory
    {
    public:
        FreeTypeFontFactory();
        ~FreeTypeFontFactory();

    public:  // IFontFactory
        Result<FontFacePtr> CreateFontFace(VFS::StreamPtr stream, int faceIndex) noexcept override;
        Result<size_t> EnumFontFace(std::vector<FontFaceInfo>& out, VFS::StreamPtr stream) noexcept override;

    private:
        FT_Library m_pLibrary = nullptr;
    };
}
