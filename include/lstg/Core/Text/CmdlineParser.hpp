/**
 * @file
 * @date 2022/8/22
 * @author 9chu
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#pragma once
#include <cassert>
#include <charconv>
#include <vector>
#include <string>
#include <optional>
#include "../Result.hpp"

namespace lstg::Text
{
    namespace detail
    {
        enum class ArgumentTypes
        {
            Plain,
            Option,
            OptionWithValue,
        };

        struct Argument
        {
            size_t Index = 0;
            ArgumentTypes Type;
            std::string Key;  // only if Type == Option or Type == OptionWithValue
            std::string Value;
        };

        template <typename T>
        struct CmdlineArgumentConverter;

        template <>
        struct CmdlineArgumentConverter<bool>
        {
            Result<int> operator()(const Argument& argument) noexcept
            {
                if (argument.Type == ArgumentTypes::Option)  // 当做开关处理，例如 -foo，总是返回 true
                    return true;
                if (argument.Type != ArgumentTypes::OptionWithValue)
                    return make_error_code(std::errc::invalid_argument);

                if (argument.Value == "true" || argument.Value == "1")
                    return true;
                else if (argument.Value == "false" || argument.Value == "0")
                    return false;
                return make_error_code(std::errc::invalid_argument);
            }
        };

        template <>
        struct CmdlineArgumentConverter<int>
        {
            Result<int> operator()(const Argument& argument) noexcept
            {
                if (argument.Type != ArgumentTypes::OptionWithValue)
                    return make_error_code(std::errc::invalid_argument);

                int ret = 0;
                auto [end, ec] = std::from_chars(argument.Value.data(), argument.Value.data() + argument.Value.size(), ret, 10);
                if (ec == std::errc{})
                    return ret;
                return make_error_code(ec);
            }
        };

        template <>
        struct CmdlineArgumentConverter<float>
        {
            Result<float> operator()(const Argument& argument) noexcept;
        };

        template <>
        struct CmdlineArgumentConverter<double>
        {
            Result<double> operator()(const Argument& argument) noexcept;
        };

        template <>
        struct CmdlineArgumentConverter<std::string_view>
        {
            Result<std::string_view> operator()(const Argument& argument) noexcept
            {
                if (argument.Type != ArgumentTypes::OptionWithValue)
                    return make_error_code(std::errc::invalid_argument);
                return argument.Value;
            }
        };

        template <>
        struct CmdlineArgumentConverter<std::string>
        {
            Result<std::string> operator()(const Argument& argument) noexcept
            {
                if (argument.Type != ArgumentTypes::OptionWithValue)
                    return make_error_code(std::errc::invalid_argument);
                try
                {
                    return argument.Value;
                }
                catch (...)
                {
                    return make_error_code(std::errc::not_enough_memory);
                }
            }
        };
    }

    /**
     * 命令行解析器
     */
    class CmdlineParser
    {
    public:

    public:
        const detail::Argument& operator[](size_t index) const noexcept;
        const detail::Argument* operator[](std::string_view key) const noexcept;

    public:
        /**
         * 解析命令行
         * @param argc 参数个数
         * @param argv 参数值
         */
        void Parse(int argc, const char* argv[]);

        /**
         * 清空
         */
        void Reset() noexcept;

        /**
         * 获取参数个数
         */
        size_t GetArgumentCount() const noexcept { return m_stArguments.size(); }

        /**
         * 获取透传参数个数
         */
        size_t GetTransparentArgumentCount() const noexcept { return m_stTransparentArguments.size(); }

        /**
         * 获取透传参数
         * @param i 下标
         */
        const std::string& GetTransparentArgument(size_t i) const noexcept
        {
            assert(i < m_stTransparentArguments.size());
            return m_stTransparentArguments[i];
        }

        /**
         * 获取执行路径
         */
        const std::string& GetExecutablePath() const noexcept { return m_stStartup; }

        /**
         * 获取命令行选项
         * @tparam T 类型
         * @param key 键
         * @return 值
         */
        template <typename T>
        Result<T> GetOption(std::string_view key) const noexcept
        {
            auto arg = operator[](key);
            if (!arg)
                return make_error_code(std::errc::no_such_file_or_directory);
            return detail::CmdlineArgumentConverter<T>{}(*arg);
        }

        /**
         * 获取命令行选项
         * @tparam T 类型
         * @tparam P 类型，同 T
         * @param key 键
         * @param defaultValue 默认值
         * @return 值
         */
        template <typename T, typename P>
        T GetOption(std::string_view key, P&& defaultValue) const noexcept
        {
            auto arg = operator[](key);
            if (!arg)
                return std::forward<P>(defaultValue);
            auto ret = detail::CmdlineArgumentConverter<T>{}(*arg);
            if (!ret)
                return std::forward<P>(defaultValue);
            return *ret;
        }

    private:
        std::string m_stStartup;
        std::vector<detail::Argument> m_stArguments;
        std::vector<std::string> m_stTransparentArguments;
    };
}
