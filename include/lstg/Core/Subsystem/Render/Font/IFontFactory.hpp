/**
* @file
* @date 2022/6/27
* @author 9chu
* 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
*/
#pragma once
#include <vector>
#include "../../VFS/IStream.hpp"
#include "IFontFace.hpp"

namespace lstg::Subsystem::Render::Font
{
    /**
     * 字体信息
     */
    struct FontFaceInfo
    {
        std::string FamilyName;
        std::string StyleName;
    };

    /**
     * 字体工厂
     */
    class IFontFactory
    {
    public:
        IFontFactory() noexcept = default;
        virtual ~IFontFactory() noexcept = default;

    public:
        /**
         * 创建字体
         * @param stream 流
         * @param faceIndex 字体ID
         * @return 创建的字体
         */
        virtual Result<FontFacePtr> CreateFontFace(VFS::StreamPtr stream, int faceIndex = 0) noexcept = 0;

        /**
         * 枚举字体
         * @param out 输出
         * @param stream 流
         * @return 子字体个数
         */
        virtual Result<size_t> EnumFontFace(std::vector<FontFaceInfo>& out, VFS::StreamPtr stream) noexcept = 0;
    };

    using FontFactoryPtr = std::shared_ptr<IFontFactory>;
}
