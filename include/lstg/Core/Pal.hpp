/**
 * @file
 * @date 2022/2/15
 * @author 9chu
 * 此文件为 LuaSTGPlus 项目的一部分，版权与许可声明详见 COPYRIGHT.txt。
 */
#pragma once
#include <filesystem>

namespace lstg
{
    namespace Pal
    {
        /**
         * 获取用户数据存储文件夹地址
         * @note 方法会自动创建用户数据存储目录
         */
        std::filesystem::path GetUserStorageDirectory() noexcept;

        /**
         * 关键性终止
         * 弹出对话框提示消息并终止程序运行。
         * @param msg 消息
         * @param abort 是否立即退出
         */
        void FatalError(const char* msg, bool abort=true) noexcept;

        /**
         * 获取当前单调时钟时间
         */
        uint64_t GetCurrentTick() noexcept;

        /**
         * 获取当前单调时钟频率
         */
        uint64_t GetTickFrequency() noexcept;
    }
} // namespace lstg
