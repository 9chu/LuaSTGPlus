/**
* @file
* @date 2022/8/28
* @author 9chu
* 此文件为 LuaSTGPlus 项目的一部分，版权与许可声明详见 COPYRIGHT.txt。
*/
#pragma once
#include <map>
#include <functional>
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

        /**
         * 注册 DSP 插件
         * @tparam T 插件类
         * @param name 名称
         */
        template <typename T>
        Result<void> RegisterDspPlugin() noexcept
        {
            const char* name = T::GetNameStatic();
            if (m_stDspPluginFactory.find(name) != m_stDspPluginFactory.end())
                return make_error_code(std::errc::file_exists);
            try
            {
                m_stDspPluginFactory.emplace(name, []() {
                    return std::static_pointer_cast<Audio::IDspPlugin>(std::make_shared<T>());
                });
                return {};
            }
            catch (...)
            {
                return std::make_error_code(std::errc::not_enough_memory);
            }
        }

        /**
         * 创建 DSP 插件
         * @param name 名称
         */
        Result<Audio::DspPluginPtr> CreateDspPlugin(std::string_view name) noexcept;

        /**
         * 遍历 DSP 插件列表
         * @tparam TFunc 回调类型
         * @param func 回调
         */
        template <typename TFunc>
        void VisitDspPlugins(TFunc func)
        {
            for (const auto& p : m_stDspPluginFactory)
                func(p.first);
        }

    protected:  // ISubsystem
        void OnUpdate(double elapsedTime) noexcept override;

    private:
        std::unique_ptr<Audio::AudioEngine> m_pEngine;
        std::map<std::string, std::function<Audio::DspPluginPtr()>, std::less<>> m_stDspPluginFactory;
    };
}
