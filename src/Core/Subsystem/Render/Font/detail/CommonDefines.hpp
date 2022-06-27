/**
 * @file
 * @date 2022/6/27
 * @author 9chu
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#pragma once
#include <cstdint>
#include <cstdlib>

namespace lstg::Subsystem::Render::Font::detail
{
    /**
     * 渲染DPI
     */
    static const int32_t kFontRenderDPI = 72;

    /**
     * 无效字符代换码点
     */
    static const char32_t kInvalidCharCodePoint = U'\uFFFD';

    /**
     * 不同字体大小缓存个数
     */
    static const size_t kFontSizeCacheSize = 16;

    /**
     * 字形数据缓存个数
     */
    static const size_t kGlyphCacheCount = 1024;

    /**
     * Kerning 数据缓存个数
     */
    static const size_t kKerningCacheCount = 1024;

    /**
     * 前进量数据缓存个数
     */
    static const size_t kAdvanceCacheCount = 1024;
}
