/**
 * @file
 * @author 9chu
 * @date 2022/3/4
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#pragma once
#include <lstg/Core/Subsystem/Render/ColorRGBA32.hpp>
#include <lstg/Core/Subsystem/Script/LuaRead.hpp>
#include <lstg/Core/Subsystem/Script/AutoBridgeHint.hpp>

namespace lstg::v2::Bridge
{
    /**
     * 颜色类
     */
    LSTG_CLASS()
    class LSTGColor :
        public Subsystem::Render::ColorRGBA32
    {
    public:
        using Subsystem::Render::ColorRGBA32::ColorRGBA32;

        LSTGColor(const Subsystem::Render::ColorRGBA32& org)
            : Subsystem::Render::ColorRGBA32(org) {}

    public:
        LSTG_METHOD(__eq)
        static bool Equals(const LSTGColor& lhs, const LSTGColor& rhs) noexcept;

        LSTG_METHOD(__add)
        static LSTGColor Add(const LSTGColor& lhs, const LSTGColor& rhs) noexcept;

        LSTG_METHOD(__sub)
        static LSTGColor Substract(const LSTGColor& lhs, const LSTGColor& rhs) noexcept;

        LSTG_METHOD(__mul)
        static LSTGColor Multiply(std::variant<double, const LSTGColor*> lhs, std::variant<double, const LSTGColor*> rhs) noexcept;

        LSTG_METHOD(ARGB)
        Subsystem::Script::Unpack<int, int, int, int> ToARGB() const noexcept;

        LSTG_METHOD(__tostring)
        std::string ToString() const;
    };
}
