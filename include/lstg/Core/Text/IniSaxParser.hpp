/**
 * @file
 * @date 2022/7/3
 * @author 9chu
 * 此文件为 LuaSTGPlus 项目的一部分，版权与许可声明详见 COPYRIGHT.txt。
 */
#pragma once
#include <string>
#include <string_view>
#include "../Flag.hpp"
#include "../Result.hpp"

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
        virtual Result<void> OnSectionBegin(std::string_view name) noexcept = 0;

        /**
         * 结束处理 INI 节
         */
        virtual Result<void> OnSectionEnd() noexcept = 0;

        /**
         * 处理 INI 键值对
         * @param key 键
         * @param value 值
         */
        virtual Result<void> OnValue(std::string_view key, std::string_view value) noexcept = 0;
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
        static Result<void> Parse(std::string_view content, IIniSaxListener* listener, IniParsingFlags flags) noexcept;
    };
}
