/**
 * @file
 * @date 2022/4/5
 * @author 9chu
 * 此文件为 LuaSTGPlus 项目的一部分，版权与许可声明详见 COPYRIGHT.txt。
 */
#pragma once
#include <string_view>

namespace lstg::Subsystem::Render::GraphDef::detail
{
    /**
     * 是否是有效的标识符
     * @param name 名字
     * @return 是否有效
     */
    inline bool IsValidIdentifier(std::string_view name) noexcept
    {
        enum {
            STATE_IDENTIFIER_START,
            STATE_IDENTIFIER,
        } state = STATE_IDENTIFIER_START;

        for (size_t i = 0; i <= name.size(); ++i)
        {
            char ch = (i >= name.size()) ? '\0' : name[i];
            switch (state)
            {
                case STATE_IDENTIFIER_START:
                    if (('a' <= ch && ch <= 'z') || ('A' <= ch && ch <= 'Z') || ch == '_')
                        state = STATE_IDENTIFIER;
                    else
                        return false;
                    break;
                case STATE_IDENTIFIER:
                    if (ch == '\0')
                        break;
                    if (!(('a' <= ch && ch <= 'z') || ('A' <= ch && ch <= 'Z') || ('0' <= ch && ch <= '9') || ch == '_'))
                        return false;
                    break;
            }
        }
        return true;
    }
}
