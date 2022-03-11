/**
 * @file
 * @date 2022/3/11
 * @author 9chu
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#include <lstg/Core/Subsystem/ProfileSystem.hpp>

using namespace std;
using namespace lstg;
using namespace lstg::Subsystem;

static ProfileSystem* s_pInstance = nullptr;

ProfileSystem& ProfileSystem::GetInstance() noexcept
{
    assert(s_pInstance);
    return *s_pInstance;
}

ProfileSystem::ProfileSystem(SubsystemContainer& container)
{
    assert(s_pInstance == nullptr);
    s_pInstance = this;
    static_cast<void>(container);

    m_ullLastFrameTime = chrono::steady_clock::now();
}

double ProfileSystem::GetPerformanceCounter(PerformanceCounterTypes type, std::string_view name) const noexcept
{
    const auto& container = (type == PerformanceCounterTypes::PerFrame) ? m_stLastFramePerFrameCounter : m_stRealTimeCounter;

    auto it = container.find(name);
    if (it == container.end())
        return 0;
    return it->second;
}

Result<void> ProfileSystem::SetPerformanceCounter(PerformanceCounterTypes type, std::string_view name, double value) noexcept
{
    auto& container = (type == PerformanceCounterTypes::PerFrame) ? m_stPerFrameCounter : m_stRealTimeCounter;

    try
    {
        container[string{name}] = value;
    }
    catch (...)
    {
        return make_error_code(errc::not_enough_memory);
    }
    return {};
}

Result<void> ProfileSystem::IncrementPerformanceCounter(PerformanceCounterTypes type, std::string_view name, double add) noexcept
{
    auto& container = (type == PerformanceCounterTypes::PerFrame) ? m_stPerFrameCounter : m_stRealTimeCounter;

    try
    {
        container[string{name}] += add;
    }
    catch (...)
    {
        return make_error_code(errc::not_enough_memory);
    }
    return {};
}

void ProfileSystem::NewFrame() noexcept
{
    auto now = chrono::steady_clock::now();
    m_ullLastFrameElapsedTime = static_cast<double>(chrono::duration_cast<chrono::milliseconds>(now - m_ullLastFrameTime).count()) / 1000.;
    m_ullLastFrameTime = now;
    std::swap(m_stLastFramePerFrameCounter, m_stPerFrameCounter);
    m_stPerFrameCounter.clear();
}
