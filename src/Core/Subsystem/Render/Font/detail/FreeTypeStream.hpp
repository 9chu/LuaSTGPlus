/**
 * @file
 * @date 2022/6/27
 * @author 9chu
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#pragma once
#include <freetype/freetype.h>
#include <lstg/Core/Subsystem/VFS/IStream.hpp>

namespace lstg::Subsystem::Render::Font::detail
{
    /**
     * FreeType 流
     */
    struct FreeTypeStream :
        public FT_StreamRec
    {
        static unsigned long OnRead(FT_Stream stream, unsigned long offset, unsigned char* buffer, unsigned long count) noexcept;
        static void OnClose(FT_Stream stream) noexcept;

        VFS::StreamPtr Stream;

        FreeTypeStream(VFS::StreamPtr s);
        void RefreshPosition() noexcept;
    };

    using FreeTypeStreamPtr = std::unique_ptr<FreeTypeStream>;
}
