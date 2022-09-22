/**
 * @file
 * @date 2022/8/28
 * @author 9chu
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#include <lstg/Core/Subsystem/AudioSystem.hpp>

#include "Audio/DspPlugins/Limiter.hpp"
#include "Audio/DspPlugins/Filter.hpp"
#include "Audio/DspPlugins/Reverb.hpp"

using namespace std;
using namespace lstg;
using namespace lstg::Subsystem;

AudioSystem::AudioSystem(SubsystemContainer& container)
{
    // 初始化音频引擎
    m_pEngine = make_unique<Audio::AudioEngine>();

    // 注册自带的 DSP 插件
    RegisterDspPlugin<Audio::DspPlugins::Limiter>();
    RegisterDspPlugin<Audio::DspPlugins::Filter>();
    RegisterDspPlugin<Audio::DspPlugins::Reverb>();
}

AudioSystem::~AudioSystem()
{
}

Result<Audio::DspPluginPtr> AudioSystem::CreateDspPlugin(std::string_view name) noexcept
{
    auto it = m_stDspPluginFactory.find(name);
    if (it == m_stDspPluginFactory.end())
        return make_error_code(errc::no_such_file_or_directory);
    try
    {
        assert(it->second);
        return it->second();
    }
    catch (...)
    {
        return make_error_code(errc::not_enough_memory);
    }
}

void AudioSystem::OnUpdate(double elapsedTime) noexcept
{
    m_pEngine->Update(elapsedTime);
}
