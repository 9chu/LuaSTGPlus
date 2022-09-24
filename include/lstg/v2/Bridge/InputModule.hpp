/**
 * @file
 * @author 9chu
 * @date 2022/4/17
 * 此文件为 LuaSTGPlus 项目的一部分，版权与许可声明详见 COPYRIGHT.txt。
 */
#pragma once
#include <lstg/Core/Subsystem/Script/LuaRead.hpp>
#include <lstg/Core/Subsystem/Script/AutoBridgeHint.hpp>

namespace lstg::v2::Bridge
{
    /**
     * 输入模块
     */
    LSTG_MODULE(lstg, GLOBAL)
    class InputModule
    {
    public:
        template <typename... TArgs>
        using Unpack = Subsystem::Script::Unpack<TArgs...>;

    public:
        /**
         * 检查按键是否被按下
         * @note 手柄输入被映射到 0x92~0xB1 和 0xDF~0xFE（共2个手柄、32个按键）的位置上
         * @param vkCode 被检查的按键
         * @return 是否按下
         */
        LSTG_METHOD()
        static bool GetKeyState(int32_t vkCode);

        /**
         * 获取最后一次被输入的按键
         */
        LSTG_METHOD()
        static int32_t GetLastKey();

        /**
         * 获取最后一次被输入的字符
         * 对于 Unicode 字符总是转换到 UTF-8 编码。
         */
        LSTG_METHOD()
        static const char* GetLastChar();

        /**
         * 获取鼠标位置
         * 以窗口左下角为坐标原点。
         * @return X, Y
         */
        LSTG_METHOD()
        static Unpack<double, double> GetMousePosition();

        /**
         * 获取鼠标按键是否被按下
         * @param button 按钮（0: 左键，1: 中键，2: 右键）
         * @return 是否被按下
         */
        LSTG_METHOD()
        static bool GetMouseState(int32_t button);
    };
}
