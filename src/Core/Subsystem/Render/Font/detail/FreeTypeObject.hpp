/**
 * @file
 * @date 2022/6/29
 * @author 9chu
 * 此文件为 LuaSTGPlus 项目的一部分，版权与许可声明详见 COPYRIGHT.txt。
 */
#pragma once
#include <memory>
#include <freetype/freetype.h>
#include <lstg/Core/Result.hpp>

namespace lstg::Subsystem::Render::Font::detail
{
    /**
     * FreeType 对象封装
     */
    class FreeTypeObject
    {
    public:
        struct FaceDeleter
        {
            void operator()(FT_Face face) noexcept
            {
                auto ret = ::FT_Done_Face(face);
                assert(ret == 0);
            }
        };

        using LibraryPtr = std::shared_ptr<FT_LibraryRec_>;
        using FacePtr = std::unique_ptr<FT_FaceRec_, FaceDeleter>;

        /**
         * 创建 FT_Library
         */
        static Result<LibraryPtr> CreateLibrary() noexcept;

        /**
         * 位图
         */
        struct Bitmap
        {
        public:
            Bitmap(LibraryPtr library) noexcept;
            Bitmap(const Bitmap&) = delete;
            Bitmap(Bitmap&& rhs) noexcept;
            ~Bitmap();

            Bitmap& operator=(const Bitmap&) = delete;
            Bitmap& operator=(Bitmap&& rhs) noexcept;

            FT_Bitmap* operator*() noexcept;

        public:
            /**
             * 从源 Bitmap 转换灰度图
             * @param original 源
             * @param alignment 对齐，取 1,2或4
             */
            Result<void> ConvertFrom(const FT_Bitmap* original, uint32_t alignment) noexcept;

        private:
            LibraryPtr m_pLibrary;
            FT_Bitmap m_stBitmap;
        };
    };
}
