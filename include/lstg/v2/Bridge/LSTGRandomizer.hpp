/**
 * @file
 * @author 9chu
 * @date 2022/3/4
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#pragma once
#include <lstg/Core/Math/Randomizer.hpp>
#include <lstg/Core/Subsystem/Script/LuaRead.hpp>
#include <lstg/Core/Subsystem/Script/AutoBridgeHint.hpp>

namespace lstg::v2::Bridge
{
    /**
     * 随机数发生器
     */
    LSTG_CLASS()
    class LSTGRandomizer :
        public Math::Randomizer
    {
    public:
        /**
         * 设置随机数种子
         * @param v 种子
         */
        LSTG_METHOD()
        void Seed(uint32_t v) noexcept;

        /**
         * 获取随机数种子
         * @return 种子
         */
        LSTG_METHOD()
        uint32_t GetSeed() const noexcept;

        /**
         * 返回 [low, upper] 上的整数随机数
         * @note 有效数字只有 float 级别，不适用于大整数
         * @param low 下界
         * @param upper 上界
         * @return 随机数
         */
        LSTG_METHOD()
        int32_t Int(int32_t low, int32_t upper) noexcept;

        /**
         * 返回 [low, upper) 上的浮点随机数
         * @note 满足 float 级别的精度
         * @param low 下界
         * @param upper 上界
         * @return 随机数
         */
        LSTG_METHOD()
        double Float(double low, double upper) noexcept;

        /**
         * 返回 -1 或 +1
         * @return 随机数
         */
        LSTG_METHOD()
        int32_t Sign() noexcept;

        /**
         * 对象转字符串表示
         */
        LSTG_METHOD(__tostring)
        std::string ToString() const;
    };
}
