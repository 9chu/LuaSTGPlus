/**
* @file
* @date 2022/8/28
* @author 9chu
* 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
*/
#pragma once
#include <map>
#include <string_view>
#include "ISubsystem.hpp"
#include "Audio/AudioEngine.hpp"

namespace lstg::Subsystem
{
    /**
     * 音频子系统
     */
    class AudioSystem :
        public ISubsystem
    {
    public:
        AudioSystem(SubsystemContainer& container);
        ~AudioSystem() override;

    public:
        /**
         * 获取音频引擎
         */
        Audio::AudioEngine& GetEngine() noexcept { return *m_pEngine; }

    protected:  // ISubsystem
        void OnUpdate(double elapsedTime) noexcept override;

    private:
        std::unique_ptr<Audio::AudioEngine> m_pEngine;
    };
}
