/**
 * @file
 * @date 2022/7/3
 * @author 9chu
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#pragma once
#include <lstg/Core/Subsystem/Render/Font/IFontFactory.hpp>

namespace lstg::Subsystem::Render::Font
{
    /**
     * HGE 纹理化字体工厂
     */
    class HgeFontFactory :
        public IFontFactory
    {
    public:  // IFontFactory
        Result<FontFacePtr> CreateFontFace(VFS::StreamPtr stream, IFontDependencyLoader* dependencyLoader, int faceIndex) noexcept override;
        Result<size_t> EnumFontFace(std::vector<FontFaceInfo>& out, VFS::StreamPtr stream) noexcept override;
    };
}
