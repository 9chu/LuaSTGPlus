/**
 * @file
 * @author 9chu
 * @date 2022/4/18
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
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
        static void PlaySound(const char* name, double vol, std::optional<double> pan /* =0.0 */);

        /**
         * 停止播放音效
         * @param name 音效资源名称
         */
        LSTG_METHOD()
        static void StopSound(const char* name);

        /**
         * 暂停播放音效
         * @param name 音效资源名称
         */
        LSTG_METHOD()
        static void PauseSound(const char* name);

        /**
         * 继续播放音效
         * @param name 音效资源名称
         */
        LSTG_METHOD()
        static void ResumeSound(const char* name);

        /**
         * 返回音效播放状态
         * @param name 音效资源名称
         * @return 返回下述值之一：paused、playing、stopped
         */
        LSTG_METHOD()
        static const char* GetSoundState(const char* name);

        /**
         * 播放音乐
         * @param name 音乐资源名称
         * @param vol 音量，取值[0,1]，为线性值
         * @param position 起始播放位置（秒）
         */
        LSTG_METHOD()
        static void PlayMusic(const char* name, std::optional<double> vol /* =1.0 */, std::optional<double> position /* =0 */);

        /**
         * 停止播放音乐
         * @param name 音乐资源名称
         */
        LSTG_METHOD()
        static void StopMusic(const char* name);

        /**
         * 暂停播放音乐
         * @param name 音乐资源名称
         */
        LSTG_METHOD()
        static void PauseMusic(const char* name);

        /**
         * 继续播放音乐
         * @param name 音乐资源名称
         */
        LSTG_METHOD()
        static void ResumeMusic(const char* name);

        /**
         * 获取音乐播放状态
         * @param name 音乐资源名称
         * @return 返回下述值之一：paused、playing、stopped
         */
        LSTG_METHOD()
        static const char* GetMusicState(const char* name);

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
        static void SetBGMVolume(LuaStack& stack);
    };
}
