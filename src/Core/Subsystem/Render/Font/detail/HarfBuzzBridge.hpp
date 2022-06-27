/**
 * @file
 * @date 2022/6/27
 * @author 9chu
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#pragma once
#include <hb.h>
#include <lstg/Core/Subsystem/Render/Font/IFontFace.hpp>

namespace lstg::Subsystem::Render::Font::detail
{
    /**
     * HarfBuzz 封装
     */
    class HarfBuzzBridge
    {
    public:
        /**
         * HarfBuzz 对象删除器
         * @tparam T
         * @tparam TDeleter
         */
        template <typename T, void TDeleter(T*)>
        struct ObjectDeleter
        {
            void operator()(T* face)
            {
                TDeleter(face);
            }
        };

        using BufferPtr = std::unique_ptr<::hb_buffer_t, ObjectDeleter<::hb_buffer_t, ::hb_buffer_destroy>>;
        using FacePtr = std::unique_ptr<::hb_face_t, ObjectDeleter<::hb_face_t, ::hb_face_destroy>>;
        using FontPtr = std::unique_ptr<::hb_font_t, ObjectDeleter<::hb_font_t, ::hb_font_destroy>>;
        using FontFuncsPtr = std::unique_ptr<::hb_font_funcs_t, ObjectDeleter<::hb_font_funcs_t, ::hb_font_funcs_destroy>>;

        /**
         * 创建 HarfBuzz 缓冲区
         */
        static Result<BufferPtr> CreateBuffer() noexcept;

        /**
         * 创建 HarfBuzz 字体面
         * @param face 字体
         */
        static Result<FacePtr> CreateFace(FontFacePtr face) noexcept;

        /**
         * 创建 HarfBuzz 字体
         * @param face 字体
         * @param param 光栅化参数
         */
        static Result<FontPtr> CreateFont(FacePtr face, FontGlyphRasterParam param) noexcept;
    };
}
