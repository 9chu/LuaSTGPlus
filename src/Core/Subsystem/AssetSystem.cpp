/**
 * @file
 * @date 2022/5/8
 * @author 9chu
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#include <lstg/Core/Subsystem/AssetSystem.hpp>

#include <lstg/Core/Logging.hpp>
#include <lstg/Core/Subsystem/SubsystemContainer.hpp>
#include <lstg/Core/Subsystem/ProfileSystem.hpp>
#include "Asset/detail/WeakPtrTraits.hpp"

// Core 中预定义的 Factory
#include <lstg/Core/Subsystem/Asset/BasicTexture2DAssetFactory.hpp>

using namespace std;
using namespace lstg;
using namespace lstg::Subsystem;

LSTG_DEF_LOG_CATEGORY(AssetSystem);

namespace
{
    /**
     * 决定加载线程数量
     */
    uint32_t DetermineLoadingThreads() noexcept
    {
        // 最大分配 4 个线程
        //  vcore  threads
        //     1        1
        //     2        1
        //     4        2
        //     8        4
        //    16        4
        return std::min(4u, std::max(1u, ThreadPool<>::GetSystemThreadCount() / 2));
    }

    /**
     * 资产加载器锁
     * 用于防止重入。
     */
    class AssetLoaderLock
    {
    public:
        AssetLoaderLock(Asset::AssetLoaderPtr loader)
            : m_pLoader(std::move(loader))
        {
            assert(m_pLoader);
            m_pLoader->SetLock(true);
        }

        AssetLoaderLock(const AssetLoaderLock&) = delete;

        AssetLoaderLock(AssetLoaderLock&& rhs)
            : m_pLoader(std::move(rhs.m_pLoader))
        {
        }

        ~AssetLoaderLock()
        {
            if (m_pLoader)
                m_pLoader->SetLock(false);
        }

    private:
        Asset::AssetLoaderPtr m_pLoader;
    };
}

static AssetSystem* s_pInstance = nullptr;

static const uint32_t kMaxTaskExecuteTimeMs = 100;
#if LSTG_ASSET_HOT_RELOAD
static const uint32_t kMaxHotReloadCheckTimeMs = 5;
static const size_t kMaxWatchTaskPerFrame = 3;  // 一帧最多检查 3 个资源
#endif

AssetSystem& AssetSystem::GetInstance() noexcept
{
    assert(s_pInstance);
    return *s_pInstance;
}

AssetSystem::AssetSystem(SubsystemContainer& container)
    : m_pVirtualFileSystem(container.Get<VirtualFileSystem>()), m_pRenderSystem(container.Get<RenderSystem>()),
    m_stAsyncLoadingThread(DetermineLoadingThreads())
{
    assert(s_pInstance == nullptr);
    s_pInstance = this;

    // 注册内建的 AssetFactory
    try
    {
        RegisterCoreAssetFactories();
    }
    catch (...)
    {
        // 干掉 s_pInstance
        assert(s_pInstance == this);
        s_pInstance = nullptr;
        throw;
    }
}

AssetSystem::~AssetSystem()
{
    assert(s_pInstance == this);
    s_pInstance = nullptr;
}

Result<VFS::StreamPtr> AssetSystem::OpenAssetStream(std::string_view path) noexcept
{
    try
    {
        auto fullPath = fmt::format("{0}/{1}", m_pVirtualFileSystem->GetAssetBaseDirectory(), path);
        return m_pVirtualFileSystem->OpenFile(fullPath, VFS::FileAccessMode::Read);
    }
    catch (...)
    {
        return make_error_code(errc::not_enough_memory);
    }
}

Result<VFS::FileAttribute> AssetSystem::GetAssetStreamAttribute(std::string_view path) noexcept
{
    try
    {
        auto fullPath = fmt::format("{0}/{1}", m_pVirtualFileSystem->GetAssetBaseDirectory(), path);
        return m_pVirtualFileSystem->GetFileAttribute(fullPath);
    }
    catch (...)
    {
        return make_error_code(errc::not_enough_memory);
    }
}

Result<void> AssetSystem::RegisterAssetFactory(Asset::AssetFactoryPtr factory) noexcept
{
    // 检查是否已经存在
    if (m_stAssetFactories.find(factory->GetAssetTypeId()) != m_stAssetFactories.end())
        return make_error_code(Asset::AssetError::AssetFactoryAlreadyRegistered);
    if (m_stAssetFactoryLookupTable.find(factory->GetAssetTypeName()) != m_stAssetFactoryLookupTable.end())
        return make_error_code(Asset::AssetError::AssetFactoryAlreadyRegistered);

    // 注册 Factory
    try
    {
        m_stAssetFactories.emplace(factory->GetAssetTypeId(), factory);
        m_stAssetFactoryLookupTable.emplace(factory->GetAssetTypeName(), factory->GetAssetTypeId());
    }
    catch (...)
    {
        // Rollback
        auto it = m_stAssetFactories.find(factory->GetAssetTypeId());
        if (it != m_stAssetFactories.end())
            m_stAssetFactories.erase(it);
        assert(m_stAssetFactoryLookupTable.find(factory->GetAssetTypeName()) == m_stAssetFactoryLookupTable.end());
        return make_error_code(std::errc::not_enough_memory);
    }
    return {};
}

Result<Asset::AssetPtr> AssetSystem::CreateAsset(Asset::AssetPoolPtr pool, std::string_view typeName, std::string_view name,
    const nlohmann::json& arguments) noexcept
{
    auto it = m_stAssetFactoryLookupTable.find(typeName);
    if (it == m_stAssetFactoryLookupTable.end())
        return make_error_code(Asset::AssetError::AssetFactoryNotRegistered);
    auto jt = m_stAssetFactories.find(it->second);
    if (jt == m_stAssetFactories.end())
        return make_error_code(Asset::AssetError::AssetFactoryNotRegistered);

    return CreateAsset(pool, jt->second, name, arguments);
}

void AssetSystem::OnUpdate(double /* elapsedTime */) noexcept
{
    // 刷新所有加载中任务的状态
    if (!m_stLoadingTasks.empty())
    {
        bool executeEnabled = true;
        auto begin = std::chrono::steady_clock::now();
        auto end = begin;

        // 检查所有加载任务
        for (int32_t i = 0; i < static_cast<int32_t>(m_stLoadingTasks.size()); )
        {
            auto task = m_stLoadingTasks[i];
            auto state = task->GetState();
            auto asset = task->GetAsset();
            assert(asset);

            // 如果资源上锁了，则跳过
            // 用于防止阻塞加载时的反复重入。
            if (task->IsLock())
            {
                ++i;
                continue;
            }

            // 如果关联的资产已经被从 Pool 删除，可以直接干掉加载任务
            if (state != Asset::AssetLoadingStates::Loading && asset->IsWildAsset())
            {
                LSTG_LOG_TRACE_CAT(AssetSystem, "Asset is already removed from pool, name={}", asset->GetName());
                goto ASSET_FAIL;
            }

            // 调用 Update
            {
#ifdef LSTG_DEVELOPMENT
                LSTG_PER_FRAME_PROFILE(AssetTask_Update);
#endif
                AssetLoaderLock lockGuard(task);
                task->Update();
            }

            // 等待依赖加载或者正在加载，此时跳过
            if (state == Asset::AssetLoadingStates::DependencyLoading || state == Asset::AssetLoadingStates::AsyncLoadCommitted ||
                state == Asset::AssetLoadingStates::Loading)
            {
                ++i;
                continue;
            }

            // 可以发起加载过程
            if (executeEnabled && state == Asset::AssetLoadingStates::Pending)
            {
#ifdef LSTG_DEVELOPMENT
                LSTG_PER_FRAME_PROFILE(AssetTask_PreLoad);
#endif

                AssetLoaderLock lockGuard(task);
                auto ret = task->PreLoad();
                state = task->GetState();

                // 刷新时间
                end = chrono::steady_clock::now();
                if (chrono::duration_cast<chrono::milliseconds>(end - begin).count() > kMaxTaskExecuteTimeMs)
                    executeEnabled = false;

                if (!ret)
                {
                    assert(state == Asset::AssetLoadingStates::Error);
                    LSTG_LOG_ERROR_CAT(AssetSystem, "Pre-load asset fail, err={}, asset={}", ret.GetError(), asset->GetName());
                    goto ASSET_FAIL;
                }
                assert(state == Asset::AssetLoadingStates::Preloaded || state == Asset::AssetLoadingStates::Loaded);
            }

            // 可以发起异步加载过程
            if (state == Asset::AssetLoadingStates::Preloaded)
            {
                // 这个状态不需要锁
                auto ret = CommitAsyncLoadTask(task);
                state = task->GetState();
                if (!ret)
                {
                    LSTG_LOG_TRACE_CAT(AssetSystem, "Commit async loading task fail, err={}, name={}", ret.GetError(), asset->GetName());
                    goto ASSET_FAIL;
                }
                assert(state == Asset::AssetLoadingStates::AsyncLoadCommitted || state == Asset::AssetLoadingStates::Loading ||
                    state == Asset::AssetLoadingStates::AsyncLoaded);
            }

            // 如果异步加载成功
            if (executeEnabled && state == Asset::AssetLoadingStates::AsyncLoaded)
            {
#ifdef LSTG_DEVELOPMENT
                LSTG_PER_FRAME_PROFILE(AssetTask_PostLoad);
#endif

                AssetLoaderLock lockGuard(task);
                auto ret = task->PostLoad();
                state = task->GetState();

                // 刷新时间
                end = chrono::steady_clock::now();
                if (chrono::duration_cast<chrono::milliseconds>(end - begin).count() > kMaxTaskExecuteTimeMs)
                    executeEnabled = false;

                if (!ret)
                {
                    assert(state == Asset::AssetLoadingStates::Error);
                    LSTG_LOG_ERROR_CAT(AssetSystem, "Post load asset fail, err={}, asset={}", ret.GetError(), asset->GetName());
                    goto ASSET_FAIL;
                }
                assert(state == Asset::AssetLoadingStates::Loaded);
            }

            // 如果加载成功
            if (state == Asset::AssetLoadingStates::Loaded)
            {
                if (asset->GetName().empty())
                    LSTG_LOG_TRACE_CAT(AssetSystem, "Asset #{} loaded", asset->m_uId);
                else
                    LSTG_LOG_TRACE_CAT(AssetSystem, "Asset \"{}\" loaded", asset->GetName());

                // 设置关联任务为 Loaded 状态
#if !LSTG_ASSET_HOT_RELOAD
                assert(asset->GetState() == Asset::AssetStates::Uninitialized);
#endif
                asset->SetState(Asset::AssetStates::Loaded);

#if LSTG_ASSET_HOT_RELOAD
                // 当热更新支持时，将任务丢到监控列表
                if (task->SupportHotReload())
                {
                    try
                    {
                        m_stWatchTasks.push_back(task);
                    }
                    catch (...)  // bad_alloc
                    {
                        LSTG_LOG_ERROR_CAT(AssetSystem, "Cannot alloc memory");
                    }
                }
#endif

                m_stLoadingTasks.erase(m_stLoadingTasks.begin() + i);
                continue;
            }

            // 如果状态不为失败（此时可能异步加载失败）
            if (state != Asset::AssetLoadingStates::Error)
            {
                ++i;
                continue;
            }

        ASSET_FAIL:
            // 设置关联任务为 Error 状态
#if LSTG_ASSET_HOT_RELOAD
            if (asset->GetState() == Asset::AssetStates::Uninitialized)  // 在热更新支持下，如果异步加载没有成功，则不对原对象发起变动
            {
                asset->SetState(Asset::AssetStates::Error);
                if (asset->GetName().empty())
                    LSTG_LOG_ERROR_CAT(AssetSystem, "Asset #{} load fail", asset->GetId());
                else
                    LSTG_LOG_ERROR_CAT(AssetSystem, "Asset {} load fail", asset->GetName());
            }
            else
            {
                if (asset->GetName().empty())
                    LSTG_LOG_ERROR_CAT(AssetSystem, "Asset #{} reload fail", asset->GetId());
                else
                    LSTG_LOG_ERROR_CAT(AssetSystem, "Asset {} reload fail", asset->GetName());
            }

            // 失败后也要放回监控任务列表
            if (task->SupportHotReload())
            {
                try
                {
                    m_stWatchTasks.push_back(task);
                }
                catch (...)  // bad_alloc
                {
                    LSTG_LOG_ERROR_CAT(AssetSystem, "Cannot alloc memory");
                }
            }
#else
            assert(asset->GetState() == Asset::AssetStates::Uninitialized);
            asset->SetState(Asset::AssetStates::Error);
            if (asset->GetName().empty())
                LSTG_LOG_ERROR_CAT(AssetSystem, "Asset #{} load fail", asset->GetId());
            else
                LSTG_LOG_ERROR_CAT(AssetSystem, "Asset {} load fail", asset->GetName());
#endif
            m_stLoadingTasks.erase(m_stLoadingTasks.begin() + i);
        }
    }

#if LSTG_ASSET_HOT_RELOAD
    // 刷新所有监控任务的状态
    if (!m_stWatchTasks.empty())
    {
#ifdef LSTG_DEVELOPMENT
        LSTG_PER_FRAME_PROFILE(AssetTask_WatchTasks);
#endif

        auto begin = std::chrono::steady_clock::now();
        auto end = begin;

        size_t watchCount = 0;
        if (m_uLastCheckedTask >= m_stWatchTasks.size())
            m_uLastCheckedTask = 0;

        // 检查所有任务
        while (m_uLastCheckedTask < m_stWatchTasks.size())
        {
            auto task = m_stWatchTasks[m_uLastCheckedTask];
            assert(!task->IsLock());

            // 如果关联资源已经卸载，则终止任务
            if (task->GetAsset()->IsWildAsset())
            {
                m_stWatchTasks.erase(m_stWatchTasks.begin() + static_cast<ptrdiff_t>(m_uLastCheckedTask));
                continue;
            }

            // 若资源已过期
            if (task->CheckIsOutdated())
            {
                try
                {
                    LSTG_LOG_INFO_CAT(AssetSystem, "Reload asset \"{}\"", task->GetAsset()->GetName());

                    // 先尝试加入任务队列
                    m_stLoadingTasks.emplace_back(task);

                    // 发起重新加载操作
                    {
#ifdef LSTG_DEVELOPMENT
                        LSTG_PER_FRAME_PROFILE(AssetTask_PrepareToReload);
#endif
                        task->PrepareToReload();
                    }

                    // 从队列删除
                    m_stWatchTasks.erase(m_stWatchTasks.begin() + static_cast<ptrdiff_t>(m_uLastCheckedTask));
                }
                catch (...)
                {
                    LSTG_LOG_ERROR_CAT(AssetSystem, "Cannot alloc memory");
                }
            }

            // 刷新时间
            end = chrono::steady_clock::now();
            if (chrono::duration_cast<chrono::milliseconds>(end - begin).count() > kMaxHotReloadCheckTimeMs)
                break;

            ++m_uLastCheckedTask;
            if (++watchCount > kMaxWatchTaskPerFrame)
                break;
        }
    }
#endif

    // 更新线程池
    {
#ifdef LSTG_DEVELOPMENT
        LSTG_PER_FRAME_PROFILE(AssetTask_ThreadUpdate);
#endif
        m_stAsyncLoadingThread.Update();
    }
}

void AssetSystem::RegisterCoreAssetFactories()
{
    auto ret = RegisterAssetFactory(make_shared<Asset::BasicTexture2DAssetFactory>());
    ret.ThrowIfError();
}

Result<Asset::AssetPtr> AssetSystem::CreateAsset(Asset::AssetPoolPtr pool, Asset::AssetFactoryPtr factory, std::string_view name,
    const nlohmann::json& arguments) noexcept
{
    assert(pool && factory);
    assert(m_pResolver);

    // 调用 Factory 创建资产
    auto ret = factory->CreateAsset(*this, pool, name, arguments, m_pResolver);
    if (!ret)
    {
        LSTG_LOG_TRACE_CAT(AssetSystem, "Create asset error, err={}, asset={}", ret.GetError(), name);
        return ret.GetError();
    }
    auto asset = std::move(std::get<0>(*ret));
    auto loader = std::move(std::get<1>(*ret));

    // 加入到队列
    if (loader)
    {
        try
        {
            m_stLoadingTasks.emplace_back(loader);
        }
        catch (...)
        {
            return make_error_code(errc::not_enough_memory);
        }
    }
    else
    {
        assert(asset->GetState() == Asset::AssetStates::Loaded);
    }

    // 加入到池子
    auto ret2 = pool->AddAsset(asset);
    if (!ret2)
    {
        // 从队列回滚
        if (loader)
        {
            assert(m_stLoadingTasks.back() == loader);
            m_stLoadingTasks.pop_back();
        }

        LSTG_LOG_ERROR_CAT(AssetSystem, "Add asset to pool error, err={}, asset={}", ret2.GetError(), name);
        return ret2.GetError();
    }

    // 如果没有启动异步加载，则阻塞等待所有任务完成
    if (!m_bAsyncLoadingEnabled)
        BlockUntilLoadingFinished(asset);

    return asset;
}

void AssetSystem::BlockUntilLoadingFinished(Asset::AssetPtr asset) noexcept
{
    while (asset->GetState() == Asset::AssetStates::Uninitialized)
        OnUpdate(0);
}

Result<void> AssetSystem::CommitAsyncLoadTask(Asset::AssetLoaderPtr loader) noexcept
{
    try
    {
        loader->SetState(Asset::AssetLoadingStates::AsyncLoadCommitted);  // 需要先执行

        m_stAsyncLoadingThread.Commit(ThreadJobCallback<void>([loader]() {
            auto ret = loader->AsyncLoad();
            if (!ret)
                LSTG_LOG_ERROR_CAT(AssetSystem, "Async load asset fail, ret={}, asset={}", ret.GetError(), loader->GetAsset()->GetName());
        }));
        return {};
    }
    catch (...)
    {
        return make_error_code(errc::not_enough_memory);
    }
}
