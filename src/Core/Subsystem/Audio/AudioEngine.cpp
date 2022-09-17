/**
 * @file
 * @date 2022/9/3
 * @author 9chu
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#include <lstg/Core/Subsystem/Audio/AudioEngine.hpp>

#include <lstg/Core/Logging.hpp>
#include "detail/StaticTopologicalSorter.hpp"

using namespace std;
using namespace lstg;
using namespace lstg::Subsystem::Audio;

LSTG_DEF_LOG_CATEGORY(AudioEngine);

#ifndef LSTG_AUDIO_SINGLE_THREADED
#define LOCK_SCOPE std::unique_lock<std::mutex> lockGuard_(m_stMutex)
#else
#define LOCK_SCOPE
#endif

// <editor-fold desc="Bus 通道控制">

bool AudioEngine::BusIsMuted(BusId id) const noexcept
{
    if (id >= kBusChannelCount)
    {
        assert(false);
        return false;
    }
    else
    {
        LOCK_SCOPE;
        return m_stBuses[id].Muted;
    }
}

void AudioEngine::BusSetMuted(BusId id, bool muted) noexcept
{
    if (id < kBusChannelCount)
    {
        LOCK_SCOPE;
        m_stBuses[id].Muted = true;
    }
}

float AudioEngine::BusGetVolume(BusId id) const noexcept
{
    if (id >= kBusChannelCount)
    {
        assert(false);
        return false;
    }
    else
    {
        LOCK_SCOPE;
        return m_stBuses[id].Volume;
    }
}

void AudioEngine::BusSetVolume(BusId id, float value) noexcept
{
    if (id < kBusChannelCount)
    {
        LOCK_SCOPE;
        m_stBuses[id].Volume = std::max(0.f, value);
    }
}

float AudioEngine::BusGetPan(BusId id) const noexcept
{
    if (id >= kBusChannelCount)
    {
        assert(false);
        return false;
    }
    else
    {
        LOCK_SCOPE;
        return m_stBuses[id].Pan;
    }
}

void AudioEngine::BusSetPan(BusId id, float value) noexcept
{
    if (id < kBusChannelCount)
    {
        LOCK_SCOPE;
        m_stBuses[id].Pan = std::min(1.f, std::max(-1.f, value));
    }
}

size_t AudioEngine::BusGetPluginCount(BusId id) const noexcept
{
    if (id >= kBusChannelCount)
    {
        assert(false);
        return 0u;
    }
    else
    {
        LOCK_SCOPE;
        return m_stBuses[id].PluginList.size();
    }
}

DspPluginPtr AudioEngine::BusGetPlugin(BusId id, size_t index) const noexcept
{
    if (id >= kBusChannelCount)
    {
        assert(false);
        return nullptr;
    }
    else
    {
        LOCK_SCOPE;
        auto& pluginList = m_stBuses[id].PluginList;
        if (index >= pluginList.size())
            return nullptr;
        return pluginList[index];
    }
}

Result<void> AudioEngine::BusInsertPlugin(BusId id, DspPluginPtr plugin, size_t index) noexcept
{
    if (id >= kBusChannelCount)
    {
        assert(false);
        return make_error_code(errc::invalid_argument);
    }
    else
    {
        LOCK_SCOPE;
        auto& pluginList = m_stBuses[id].PluginList;
        try
        {
            if (index >= pluginList.size())
                pluginList.emplace_back(std::move(plugin));
            else
                pluginList.insert(pluginList.begin() + static_cast<ptrdiff_t>(index), std::move(plugin));
        }
        catch (...)
        {
            return make_error_code(errc::not_enough_memory);
        }
        return {};
    }
}

bool AudioEngine::BusRemovePlugin(BusId id, size_t index) noexcept
{
    if (id >= kBusChannelCount)
    {
        assert(false);
        return false;
    }
    else
    {
        LOCK_SCOPE;
        auto& pluginList = m_stBuses[id].PluginList;
        if (index >= pluginList.size())
            return false;
        pluginList.erase(pluginList.begin() + static_cast<ptrdiff_t>(index));
        return true;
    }
}

size_t AudioEngine::BusGetSendTargetCount(BusId id, BusSendStages stage) const noexcept
{
    if (id >= kBusChannelCount || static_cast<int>(stage) >= static_cast<int>(BusSendStages::Count_))
    {
        assert(false);
        return 0u;
    }
    else
    {
        LOCK_SCOPE;
        auto& sendList = m_stBuses[id].SendList[static_cast<int>(stage)];
        return sendList.size();
    }
}

Result<BusSend> AudioEngine::BusGetSendTarget(BusId id, BusSendStages stage, size_t index) const noexcept
{
    if (id >= kBusChannelCount || static_cast<int>(stage) >= static_cast<int>(BusSendStages::Count_))
    {
        assert(false);
        return make_error_code(errc::invalid_argument);
    }
    else
    {
        LOCK_SCOPE;
        auto& sendList = m_stBuses[id].SendList[static_cast<int>(stage)];
        if (index >= sendList.size())
            return make_error_code(errc::invalid_argument);
        return sendList[index];
    }
}

Result<void> AudioEngine::BusAddSendTarget(BusId id, BusSendStages stage, BusSend send) noexcept
{
    if (id >= kBusChannelCount || static_cast<int>(stage) >= static_cast<int>(BusSendStages::Count_))
    {
        assert(false);
        return make_error_code(errc::invalid_argument);
    }
    else
    {
        LOCK_SCOPE;
        auto& sendList = m_stBuses[id].SendList[static_cast<int>(stage)];
        try
        {
            sendList.emplace_back(std::move(send));
        }
        catch (...)
        {
            return make_error_code(errc::not_enough_memory);
        }

        // 重建拓扑
        auto ret = RebuildBusUpdateList();
        if (!ret)
        {
            // rollback
            sendList.pop_back();
            return ret;
        }
        return {};
    }
}

bool AudioEngine::BusRemoveSendTarget(BusId id, BusSendStages stage, size_t index) noexcept
{
    if (id >= kBusChannelCount || static_cast<int>(stage) >= static_cast<int>(BusSendStages::Count_))
    {
        assert(false);
        return false;
    }
    else
    {
        LOCK_SCOPE;
        auto& sendList = m_stBuses[id].SendList[static_cast<int>(stage)];
        if (index >= sendList.size())
            return false;
        sendList.erase(sendList.begin() + static_cast<ptrdiff_t>(index));
        // 删除操作不需要重建更新列表
        return true;
    }
}

BusId AudioEngine::BusGetOutputTarget(BusId id) const noexcept
{
    if (id >= kBusChannelCount)
    {
        assert(false);
        return static_cast<size_t>(-1);
    }
    else
    {
        LOCK_SCOPE;
        return m_stBuses[id].OutputTarget;
    }
}

Result<void> AudioEngine::BusSetOutputTarget(BusId id) noexcept
{
    if (id >= kBusChannelCount)
    {
        assert(false);
        return make_error_code(errc::invalid_argument);
    }
    else
    {
        LOCK_SCOPE;
        auto oldTarget = m_stBuses[id].OutputTarget;
        m_stBuses[id].OutputTarget = id;
        auto ret = RebuildBusUpdateList();
        if (!ret)
        {
            // rollback
            m_stBuses[id].OutputTarget = oldTarget;
            return ret;
        }
        return {};
    }
}

// </editor-fold>

Result<void> AudioEngine::RebuildBusUpdateList() noexcept
{
    detail::StaticTopologicalSorter<kBusChannelCount> sorter;

    // 构建邻接矩阵
    sorter.Reset();
    for (size_t i = 0; i < kBusChannelCount; ++i)
    {
        // 检查 Send
        for (size_t j = 0; j < static_cast<int>(BusSendStages::Count_); ++j)
        {
            // 通道 i 发送到 send.Target
            for (const auto& send : m_stBuses[i].SendList[j])
                sorter.SetAdjacency(i, send.Target);
        }

        // 通道 i 输出到 OutputTarget
        if (m_stBuses[i].OutputTarget != static_cast<BusId>(-1))
            sorter.SetAdjacency(i, m_stBuses[i].OutputTarget);
    }

    // 执行拓扑排序
    if (!sorter.Sort())
    {
        LSTG_LOG_ERROR_CAT(AudioEngine, "Topological sort fail, circular reference detected");
        return make_error_code(errc::invalid_argument);
    }

    // 刷新更新顺序
    for (size_t i = 0; i < kBusChannelCount; ++i)
        m_stBusesUpdateList[i] = sorter.GetResult(i);
    return {};
}
