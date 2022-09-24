/**
 * @file
 * @author 9chu
 * @date 2022/7/16
 * 此文件为 LuaSTGPlus 项目的一部分，版权与许可声明详见 COPYRIGHT.txt。
 */
#include <lstg/v2/AssetPools.hpp>

#include <lstg/Core/Logging.hpp>

using namespace std;
using namespace lstg;
using namespace lstg::v2;

LSTG_DEF_LOG_CATEGORY(AssetPools);

AssetPools::AssetPools()
{
    // 初始化资源池
    m_pGlobalAssetPool = make_shared<Subsystem::Asset::AssetPool>();
    m_pStageAssetPool = make_shared<Subsystem::Asset::AssetPool>();
    m_pCurrentAssetPool = m_pGlobalAssetPool.get();  // 默认挂载在全局池上

    // 初始化名字缓冲区
    m_stTmpNameBuffer.reserve(64);
}

std::tuple<AssetPoolTypes, Subsystem::Asset::AssetPoolPtr> AssetPools::GetCurrentAssetPool() const noexcept
{
    if (m_pCurrentAssetPool == m_pGlobalAssetPool.get())
        return { AssetPoolTypes::Global, m_pGlobalAssetPool };
    else if (m_pCurrentAssetPool == m_pStageAssetPool.get())
        return { AssetPoolTypes::Stage, m_pStageAssetPool };

    assert(!m_pCurrentAssetPool);
    return { AssetPoolTypes::None, nullptr };
}

void AssetPools::SetCurrentAssetPool(AssetPoolTypes t) noexcept
{
    switch (t)
    {
        default:
            assert(false);
        case AssetPoolTypes::None:
            m_pCurrentAssetPool = nullptr;
            break;
        case AssetPoolTypes::Global:
            m_pCurrentAssetPool = m_pGlobalAssetPool.get();
            break;
        case AssetPoolTypes::Stage:
            m_pCurrentAssetPool = m_pStageAssetPool.get();
            break;
    }
}

Subsystem::Asset::AssetPtr AssetPools::FindAsset(AssetTypes type, std::string_view name) const noexcept
{
    auto ret = MakeFullAssetName(m_stTmpNameBuffer, type, name);
    if (!ret)
    {
        LSTG_LOG_ERROR_CAT(AssetPools, "Cannot alloc memory");
        return nullptr;
    }
    return OnResolveAsset(m_stTmpNameBuffer);
}

std::tuple<AssetPoolTypes, Subsystem::Asset::AssetPoolPtr> AssetPools::LocateAsset(AssetTypes type, std::string_view name) const noexcept
{
    auto ret = MakeFullAssetName(m_stTmpNameBuffer, type, name);
    if (!ret)
    {
        LSTG_LOG_ERROR_CAT(AssetPools, "Cannot alloc memory");
        return { AssetPoolTypes::None, nullptr };
    }

    auto found = m_pStageAssetPool->GetAsset(name);
    if (found)
        return { AssetPoolTypes::Stage, m_pStageAssetPool };
    found = m_pGlobalAssetPool->GetAsset(name);
    if (found)
        return { AssetPoolTypes::Global, m_pGlobalAssetPool };
    return { AssetPoolTypes::None, nullptr };
}

bool AssetPools::RemoveAsset(AssetPoolTypes pool, AssetTypes type, std::string_view name) noexcept
{
    Subsystem::Asset::AssetPool* assetPool = nullptr;
    switch (pool)
    {
        default:
            assert(false);
        case AssetPoolTypes::None:
            return false;  // None 池子总是没有资产
        case AssetPoolTypes::Global:
            assetPool = m_pGlobalAssetPool.get();
            break;
        case AssetPoolTypes::Stage:
            assetPool = m_pStageAssetPool.get();
            break;
    }

    auto ret = MakeFullAssetName(m_stTmpNameBuffer, type, name);
    if (!ret)
    {
        LSTG_LOG_ERROR_CAT(AssetPools, "Cannot alloc memory");
        return false;
    }

    assert(assetPool);
    auto ret2 = assetPool->RemoveAsset(m_stTmpNameBuffer);
    if (!ret2)
    {
        LSTG_LOG_WARN_CAT(AssetPools, "Remove asset {} fail: {}", m_stTmpNameBuffer, ret2.GetError());
        return false;
    }
    return true;
}

void AssetPools::ClearAssetPool(AssetPoolTypes pool) noexcept
{
    Subsystem::Asset::AssetPool* assetPool = nullptr;
    switch (pool)
    {
        default:
            assert(false);
        case AssetPoolTypes::None:
            return;  // None 池子总是没有资产
        case AssetPoolTypes::Global:
            assetPool = m_pGlobalAssetPool.get();
            break;
        case AssetPoolTypes::Stage:
            assetPool = m_pStageAssetPool.get();
            break;
    }

    assert(assetPool);
    assetPool->Clear();
}

Subsystem::Asset::AssetPtr AssetPools::OnResolveAsset(std::string_view name) const noexcept
{
    // OnResolveAsset 是内部使用的方法，故这里已经带上了前缀
    auto ret = m_pStageAssetPool->GetAsset(name);
    if (!ret)
        ret = m_pGlobalAssetPool->GetAsset(name);
    return ret;
}
