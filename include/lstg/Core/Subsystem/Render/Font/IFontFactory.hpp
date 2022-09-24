/**
* @file
* @date 2022/6/27
* @author 9chu
* 此文件为 LuaSTGPlus 项目的一部分，版权与许可声明详见 COPYRIGHT.txt。
*/
#pragma once
#include <vector>
#include <string_view>
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
     * 字体依赖加载器
     */
    class IFontDependencyLoader
    {
    public:
        /**
         * 当加载依赖的纹理时调用
         * @param path 路径
         * @return 纹理
         */
        virtual Result<TexturePtr> OnLoadTexture(std::string_view path) noexcept = 0;
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
         * @param dependencyLoader 依赖加载器
         * @param faceIndex 字体ID
         * @return 创建的字体
         */
        virtual Result<FontFacePtr> CreateFontFace(VFS::StreamPtr stream, IFontDependencyLoader* dependencyLoader,
            int faceIndex = 0) noexcept = 0;

        /**
         * 枚举字体
         * @param out 输出
         * @param stream 流
         * @return 子字体个数
         */
        virtual Result<size_t> EnumFontFace(std::vector<FontFaceInfo>& out, VFS::StreamPtr stream) noexcept = 0;
    };

    using FontFactoryPtr = std::shared_ptr<IFontFactory>;

    /**
     * 创建 FreeType 字体工厂
     */
    FontFactoryPtr CreateFreeTypeFactory();

    /**
     * 创建 HGE 字体工厂
     */
    FontFactoryPtr CreateHgeFontFactory();
}
