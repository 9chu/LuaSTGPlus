/**
 * @file
 * @date 2022/6/27
 * @author 9chu
 * 此文件为 LuaSTGPlus 项目的一部分，版权与许可声明详见 COPYRIGHT.txt。
 */
#pragma once
#include "detail/FreeTypeObject.hpp"
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

    public:  // IFontFactory
        Result<FontFacePtr> CreateFontFace(VFS::StreamPtr stream, IFontDependencyLoader* dependencyLoader, int faceIndex) noexcept override;
        Result<size_t> EnumFontFace(std::vector<FontFaceInfo>& out, VFS::StreamPtr stream) noexcept override;

    private:
        detail::FreeTypeObject::LibraryPtr m_pLibrary;
    };
}
