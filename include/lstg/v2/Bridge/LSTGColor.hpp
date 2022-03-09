/**
 * @file
 * @author 9chu
 * @date 2022/3/4
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#pragma once
#include <lstg/Core/Color.hpp>
#include <lstg/Core/Subsystem/Script/LuaRead.hpp>
#include <lstg/Core/Subsystem/Script/AutoBridgeHint.hpp>

namespace lstg::v2::Bridge
{
    /**
     * 颜色类
     */
    LSTG_CLASS()
    class LSTGColor :
        public RGBA32Color
    {
    public:
        LSTG_METHOD(__eq)
        static bool Equals(const LSTGColor& lhs, const LSTGColor& rhs) noexcept;

        LSTG_METHOD(__add)
        static LSTGColor Add(const LSTGColor& lhs, const LSTGColor& rhs) noexcept;

        LSTG_METHOD(__sub)
        static LSTGColor Substract(const LSTGColor& lhs, const LSTGColor& rhs) noexcept;

        LSTG_METHOD(__mul)
        static LSTGColor Multiply(std::variant<double, const LSTGColor*> lhs, std::variant<double, const LSTGColor*> rhs) noexcept;

        LSTG_METHOD(__tostring)
        std::string ToString();
    };

    /**
     * 颜色模块
     */
    LSTG_MODULE(lstgColor, GLOBAL)
    class LSTGColorModule
    {
    public:
        LSTG_METHOD(ARGB)
        static Subsystem::Script::Unpack<int, int, int, int> ToARGB(const LSTGColor& color);
    };
}
