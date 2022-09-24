/**
 * @file
 * @date 2022/6/27
 * @author 9chu
 * 此文件为 LuaSTGPlus 项目的一部分，版权与许可声明详见 COPYRIGHT.txt。
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
