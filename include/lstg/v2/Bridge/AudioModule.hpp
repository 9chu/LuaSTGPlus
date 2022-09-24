/**
 * @file
 * @author 9chu
 * @date 2022/4/18
 * 此文件为 LuaSTGPlus 项目的一部分，版权与许可声明详见 COPYRIGHT.txt。
 */
#pragma once
#include <lstg/Core/Subsystem/Script/LuaRead.hpp>
#include <lstg/Core/Subsystem/Script/AutoBridgeHint.hpp>

namespace lstg::v2::Bridge
{
    /**
     * 音频模块
     */
    LSTG_MODULE(lstg, GLOBAL)
    class AudioModule
    {
    public:
        using LuaStack = Subsystem::Script::LuaStack;

    public:
        /**
         * 播放音效
         * @param name 音效资源名称
         * @param vol 音量，取值范围[0~1]，为线性值
         * @param pan 平衡，取值范围[-1~1]
         */
        LSTG_METHOD()
        static void PlaySound(LuaStack& stack, const char* name, double vol, std::optional<double> pan /* =0.0 */);

        /**
         * 停止播放音效
         * @param name 音效资源名称
         */
        LSTG_METHOD()
        static void StopSound(LuaStack& stack, const char* name);

        /**
         * 暂停播放音效
         * @param name 音效资源名称
         */
        LSTG_METHOD()
        static void PauseSound(LuaStack& stack, const char* name);

        /**
         * 继续播放音效
         * @param name 音效资源名称
         */
        LSTG_METHOD()
        static void ResumeSound(LuaStack& stack, const char* name);

        /**
         * 返回音效播放状态
         * @param name 音效资源名称
         * @return 返回下述值之一：paused、playing、stopped
         */
        LSTG_METHOD()
        static const char* GetSoundState(LuaStack& stack, const char* name);

        /**
         * 播放音乐
         * @param name 音乐资源名称
         * @param vol 音量，取值[0,1]，为线性值
         * @param position 起始播放位置（秒）
         */
        LSTG_METHOD()
        static void PlayMusic(LuaStack& stack, const char* name, std::optional<double> vol, std::optional<double> position);

        /**
         * 停止播放音乐
         * @param name 音乐资源名称
         */
        LSTG_METHOD()
        static void StopMusic(LuaStack& stack, const char* name);

        /**
         * 暂停播放音乐
         * @param name 音乐资源名称
         */
        LSTG_METHOD()
        static void PauseMusic(LuaStack& stack, const char* name);

        /**
         * 继续播放音乐
         * @param name 音乐资源名称
         */
        LSTG_METHOD()
        static void ResumeMusic(LuaStack& stack, const char* name);

        /**
         * 获取音乐播放状态
         * @param name 音乐资源名称
         * @return 返回下述值之一：paused、playing、stopped
         */
        LSTG_METHOD()
        static const char* GetMusicState(LuaStack& stack, const char* name);

        /**
         * 更新音频系统
         * @deprecated 已弃用方法
         */
        LSTG_METHOD()
        static void UpdateSound();

        /**
         * 设置全局音效音量
         * @param vol 音量值，取值[0,1]，线性值
         */
        LSTG_METHOD()
        static void SetSEVolume(double vol);

        /**
         * 设置全局音乐音量
         * 若参数个数为1，则设置全局音乐音量。该操作将影响后续播放音乐的音量。
         * 若参数个数为2，则设置指定音乐的播放音量。
         * @param stack Lua栈
         */
        LSTG_METHOD()
        static void SetBGMVolume(LuaStack& stack, std::variant<double, const char*> arg1, std::optional<double> arg2);
    };
}
