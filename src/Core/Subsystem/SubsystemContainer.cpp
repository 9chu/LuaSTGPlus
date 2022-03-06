/**
 * @file
 * @date 2022/3/6
 * @author 9chu
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#include <lstg/Core/Subsystem/SubsystemContainer.hpp>

#include <lstg/Core/Logging.hpp>

using namespace std;
using namespace lstg;
using namespace lstg::Subsystem;

LSTG_DEF_LOG_CATEGORY(SubsystemContainer);

void SubsystemContainer::ConstructAll()
{
    for (auto& pair : m_stSubsystems)
    {
        auto storage = pair.second;
        if (storage->Status == SubsystemStatus::NotInit)
            Construct(storage);
    }
}

void SubsystemContainer::UpdateAll(double elapsedTime) noexcept
{
    for (const auto& pair : m_stSubsystemByPriority)
    {
        const auto& storage = pair.second;
        assert(storage->Instance && storage->Status == SubsystemStatus::Ready);
        storage->Instance->OnUpdate(elapsedTime);
    }
}

void SubsystemContainer::BubbleEvent(SubsystemEvent& event) noexcept
{
    if (!event.IsBubbles())
        return;

    for (const auto& pair : m_stSubsystemByPriority)
    {
        const auto& storage = pair.second;
        assert(storage->Instance && storage->Status == SubsystemStatus::Ready);
        storage->Instance->OnEvent(event);
        if (!event.IsBubbles())
            break;
    }
}

void SubsystemContainer::Construct(const SubsystemStoragePtr& storage)
{
    assert(storage->Status == SubsystemStatus::NotInit);
    assert(!storage->Instance);
    storage->Status = SubsystemStatus::Initializing;

    try
    {
        LSTG_LOG_TRACE_CAT(SubsystemContainer, "Construct subsystem {}", storage->Name);
        assert(storage->Constructor);
        storage->Instance = storage->Constructor(*this);
        storage->Status = SubsystemStatus::Ready;
        m_stSubsystemByPriority.emplace(storage->Priority, storage);
    }
    catch (const std::exception& ex)
    {
        LSTG_LOG_ERROR_CAT(SubsystemContainer, "Construct subsystem {} fail: {}", storage->Name, ex.what());
        storage->Status = SubsystemStatus::NotInit;
        storage->Instance.reset();
        throw;
    }
    catch (...)
    {
        LSTG_LOG_ERROR_CAT(SubsystemContainer, "Construct subsystem {} fail", storage->Name);
        storage->Status = SubsystemStatus::NotInit;
        storage->Instance.reset();
        throw;
    }
}

bool SubsystemContainer::Register(SubsystemStoragePtr storage)
{
    // 检查名称是否存在
    auto it = m_stSubsystems.find(storage->Name);
    if (it != m_stSubsystems.end())
        return false;

    LSTG_LOG_TRACE_CAT(SubsystemContainer, "Subsystem {} register with priority {}", storage->Name, storage->Priority);

    // 记录到容器
    m_stSubsystems.emplace(storage->Name, storage);
    m_stSubsystemById.emplace(storage->UniqueId, std::move(storage));
    // 构造完成后才放入
    //m_stSubsystemByPriority.emplace(storage->Priority, storage);

    return true;
}

SubsystemPtr SubsystemContainer::FindByName(std::string_view name)
{
    auto it = m_stSubsystems.find(name);
    if (it == m_stSubsystems.end())
    {
        LSTG_LOG_ERROR_CAT(SubsystemContainer, "Dependent subsystem {} not found", name);
        throw system_error(make_error_code(errc::no_such_file_or_directory));
    }
    auto storage = it->second;

    // 如果对象没有构造，则触发构造
    if (storage->Status == SubsystemStatus::NotInit)
    {
        Construct(storage);
    }
    else if (storage->Status == SubsystemStatus::Initializing)
    {
        LSTG_LOG_ERROR_CAT(SubsystemContainer, "Subsystem {} is in initializing, circular reference detected", name);
        throw system_error(make_error_code(errc::operation_in_progress));
    }

    assert(storage->Instance && storage->Status == SubsystemStatus::Ready);
    return storage->Instance;
}

SubsystemPtr SubsystemContainer::FindById(size_t id)
{
    auto it = m_stSubsystemById.find(id);
    if (it == m_stSubsystemById.end())
    {
        LSTG_LOG_ERROR_CAT(SubsystemContainer, "Dependent subsystem #{} not found", id);
        throw system_error(make_error_code(errc::no_such_file_or_directory));
    }
    auto storage = it->second;

    // 如果对象没有构造，则触发构造
    if (storage->Status == SubsystemStatus::NotInit)
    {
        Construct(storage);
    }
    else if (storage->Status == SubsystemStatus::Initializing)
    {
        LSTG_LOG_ERROR_CAT(SubsystemContainer, "Subsystem {} is in initializing, circular reference detected", storage->Name);
        throw system_error(make_error_code(errc::operation_in_progress));
    }

    assert(storage->Instance && storage->Status == SubsystemStatus::Ready);
    return storage->Instance;
}
