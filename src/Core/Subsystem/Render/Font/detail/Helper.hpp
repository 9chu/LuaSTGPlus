/**
 * @file
 * @date 2022/6/27
 * @author 9chu
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#pragma once
#include <cstdint>

namespace lstg::Subsystem::Render::Font::detail
{
    /**
     * 辅助类
     */
    class Helper
    {
    public:
        /**
         * 是否是 UTF-16 代理对的高半区
         * @param codePoint 码点
         */
        static bool IsHighSurrogate(uint32_t codePoint) noexcept;

        /**
         * 是否是 UTF-16 代理对的低半区
         * @param codePoint 码点
         */
        static bool IsLowSurrogate(uint32_t codePoint) noexcept;

        /**
         * 编码 UTF-16 代理对到 UTF-32
         * @param highSurrogate 高半区
         * @param lowSurrogate 低半区
         */
        static uint32_t EncodeSurrogate(uint16_t highSurrogate, uint16_t lowSurrogate) noexcept;

        /**
         * 是否渲染为空白符
         * @param codePoint 码点
         */
        static bool IsRenderAsWhitespace(char32_t codePoint) noexcept;

        /**
         * 是否是控制字符
         * @param codePoint 码点
         */
        static bool IsControlCharacter(char32_t codePoint) noexcept;
    };
}
