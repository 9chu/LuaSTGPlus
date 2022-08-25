/**
 * @file
 * @date 2022/8/22
 * @author 9chu
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#include <lstg/Core/Text/CmdlineParser.hpp>

#include <cstring>
#include <ryu/ryu_parse.h>

using namespace std;
using namespace lstg;
using namespace lstg::Text;

Result<float> Text::detail::CmdlineArgumentConverter<float>::operator()(const Argument& argument) noexcept
{
    if (argument.Type != ArgumentTypes::OptionWithValue)
        return make_error_code(std::errc::invalid_argument);

    float ret = 0.f;
    auto ec = s2f_n(argument.Value.c_str(), static_cast<int>(argument.Value.size()), &ret);
    if (ec == SUCCESS)
        return ret;
    return make_error_code(std::errc::invalid_argument);
}

Result<double> Text::detail::CmdlineArgumentConverter<double>::operator()(const Argument& argument) noexcept
{
    if (argument.Type != ArgumentTypes::OptionWithValue)
        return make_error_code(std::errc::invalid_argument);

    double ret = 0.;
    auto ec = s2d_n(argument.Value.c_str(), static_cast<int>(argument.Value.size()), &ret);
    if (ec == SUCCESS)
        return ret;
    return make_error_code(std::errc::invalid_argument);
}

const Text::detail::Argument& CmdlineParser::operator[](size_t index) const noexcept
{
    assert(index < m_stArguments.size());
    return m_stArguments[index];
}

const Text::detail::Argument* CmdlineParser::operator[](std::string_view key) const noexcept
{
    for (const auto& kv : m_stArguments)
    {
        if (kv.Type == Text::detail::ArgumentTypes::Option || kv.Type == Text::detail::ArgumentTypes::OptionWithValue)
        {
            if (kv.Key == key)
                return &kv;
        }
    }
    return nullptr;
}

void CmdlineParser::Parse(int argc, const char* argv[])
{
    assert(argc >= 1);
    m_stStartup = argv[0];
    m_stArguments.clear();
    m_stTransparentArguments.clear();
    m_stArguments.reserve(argc);
    m_stTransparentArguments.reserve(argc);

    bool transparent = false;
    for (int i = 1; i < argc; ++i)
    {
        const char* arg = argv[i];
        if (transparent)
        {
            // 透传参数
            m_stTransparentArguments.emplace_back(arg);
        }
        else
        {
            // 透传标记
            if (::strcmp(arg, "--") == 0)
            {
                transparent = true;
                continue;
            }

            Text::detail::Argument parsedArg {};

            if (arg[0] == '-')
            {
                // 当做 option 解析
                enum {
                    STATE_LOOK_FOR_EQUAL,
                    STATE_LOOK_FOR_END,
                } state = STATE_LOOK_FOR_EQUAL;

                size_t keyLen = 0, valueLen = 0, valueStartIndex = 0;
                size_t len = ::strlen(arg);
                for (size_t j = 1; j <= len; ++j)
                {
                    char ch = j < len ? arg[j] : '\0';
                    switch (state)
                    {
                        case STATE_LOOK_FOR_EQUAL:
                            if (('0' <= ch && ch <= '9') || ('a' <= ch && ch <= 'z') || ('A' <= ch && ch <= 'Z') || ch == '_' || ch == '-')
                            {
                                ++keyLen;
                            }
                            else if (ch == '=')
                            {
                                if (keyLen == 0)
                                    goto PARSE_FAIL;
                                state = STATE_LOOK_FOR_END;
                                valueStartIndex = j + 1;
                            }
                            else if (ch == '\0')
                            {
                                if (keyLen == 0)
                                    goto PARSE_FAIL;

                                parsedArg.Type = Text::detail::ArgumentTypes::Option;
                                parsedArg.Key = string_view { &arg[1], keyLen };
                                parsedArg.Index = m_stArguments.size();
                                m_stArguments.emplace_back(std::move(parsedArg));
                                goto CONTINUE;
                            }
                            else
                            {
                                goto PARSE_FAIL;
                            }
                            break;
                        case STATE_LOOK_FOR_END:
                            if (ch == '\0')
                            {
                                parsedArg.Type = Text::detail::ArgumentTypes::OptionWithValue;
                                parsedArg.Key = string_view { &arg[1], keyLen };
                                parsedArg.Value = string_view { &arg[valueStartIndex], valueLen };
                                parsedArg.Index = m_stArguments.size();
                                m_stArguments.emplace_back(std::move(parsedArg));
                                goto CONTINUE;
                            }
                            else
                            {
                                ++valueLen;
                            }
                            break;
                        default:
                            assert(false);
                    }
                }
            PARSE_FAIL:
                ;
            }

            parsedArg.Type = Text::detail::ArgumentTypes::Plain;
            parsedArg.Value = arg;
            parsedArg.Index = m_stArguments.size();
            m_stArguments.emplace_back(std::move(parsedArg));

        CONTINUE:
            ;
        }
    }
}

void CmdlineParser::Reset() noexcept
{
    m_stStartup.clear();
    m_stArguments.clear();
    m_stTransparentArguments.clear();
}
