/**
 * @file
 * @date 2022/8/28
 * @author 9chu
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#include <lstg/Core/Subsystem/AudioSystem.hpp>

using namespace std;
using namespace lstg;
using namespace lstg::Subsystem;

AudioSystem::AudioSystem(SubsystemContainer& container)
{
    // 初始化音频引擎
    m_pEngine = make_unique<Audio::AudioEngine>();
}

AudioSystem::~AudioSystem()
{
}

void AudioSystem::OnUpdate(double elapsedTime) noexcept
{
    m_pEngine->Update(elapsedTime);
}
