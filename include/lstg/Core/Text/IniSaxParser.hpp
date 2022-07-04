/**
 * @file
 * @date 2022/7/3
 * @author 9chu
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#pragma once
#include <string>
#include <string_view>
#include "../Flag.hpp"

namespace lstg::Text
{
    /**
     * Ini Sax 解析监听器
     */
    class IIniSaxListener
    {
    public:
        /**
         * 开始处理 INI 节
         * @param name 节名称
         */
        virtual void OnSectionBegin(std::string_view name) = 0;

        /**
         * 结束处理 INI 节
         */
        virtual void OnSectionEnd() = 0;

        /**
         * 处理 INI 键值对
         * @param key 键
         * @param value 值
         */
        virtual void OnValue(std::string_view key, std::string_view value) = 0;
    };

    /**
     * INI 解析参数
     */
    LSTG_FLAG_BEGIN(IniParsingFlags)
        IgnoreSectionLeadingSpaces = 1,
        IgnoreSectionTailingSpaces = 2,
        IgnoreKeyLeadingSpaces = 4,
        IgnoreKeyTailingSpaces = 8,
        IgnoreValueLeadingSpaces = 16,
        IgnoreValueTailingSpaces = 32,
        RemoveCommentInValue = 64,
        UnixStyleComment = 128,
    LSTG_FLAG_END(IniParsingFlags)

    /**
     * INI Sax 解析器
     */
    class IniSaxParser
    {
    public:
        /**
         * 解析
         * @param content 内容
         * @param listener 监听器
         * @param flags 解析参数
         */
        static void Parse(std::string_view content, IIniSaxListener* listener, IniParsingFlags flags);
    };
}
