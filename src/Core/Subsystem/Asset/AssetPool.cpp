/**
 * @file
 * @date 2022/5/14
 * @author 9chu
 * 此文件为 LuaSTGPlus 项目的一部分，版权与许可声明详见 COPYRIGHT.txt。
 */
#include <lstg/Core/Subsystem/Asset/AssetPool.hpp>

#include <lstg/Core/Subsystem/Asset/AssetError.hpp>
#include "detail/WeakPtrTraits.hpp"

using namespace std;
using namespace lstg;
using namespace lstg::Subsystem::Asset;

AssetPool::~AssetPool()
{
    Clear(AssetClearTypes::All);
}

Result<void> AssetPool::AddAsset(AssetPtr asset) noexcept
{
    assert(asset && asset->m_uId == kEmptyAssetId && asset->IsWildAsset());
    auto id = m_iNextAssetId++;

    // 允许资产为匿名资产，此时不能通过名称快速查找
    if (!asset->GetName().empty())
    {
        auto it = m_stLookupTable.find(asset->GetName());
        if (it != m_stLookupTable.end())
            return make_error_code(AssetError::AssetAlreadyExists);
    }
    assert(m_stAssets.find(id) == m_stAssets.end());

    try
    {
        if (!asset->GetName().empty())
            m_stLookupTable.emplace(asset->GetName(), id);
        m_stAssets.emplace(id, asset);
    }
    catch (...)  // bad_alloc
    {
        // 回滚状态，若 m_stAssets 插入失败只要判断 m_stLookupTable 是否有插入就行
        if (!asset->GetName().empty())
        {
            auto it = m_stLookupTable.find(asset->GetName());
            if (it != m_stLookupTable.end())
                m_stLookupTable.erase(it);
        }
        return make_error_code(errc::not_enough_memory);
    }

    asset->m_uId = id;
    asset->m_pPool = shared_from_this();
    return {};
}

bool AssetPool::ContainsAsset(AssetId id) noexcept
{
    if (id == kEmptyAssetId)
        return false;
    auto it = m_stAssets.find(id);
    return it != m_stAssets.end();
}

bool AssetPool::ContainsAsset(std::string_view name) noexcept
{
    auto it = m_stLookupTable.find(name);
    return it != m_stLookupTable.end();
}

Result<void> AssetPool::RemoveAsset(AssetId id) noexcept
{
    if (id == kEmptyAssetId)
        return make_error_code(AssetError::AssetNotFound);
    auto it = m_stAssets.find(id);
    if (it == m_stAssets.end())
        return make_error_code(AssetError::AssetNotFound);

    auto cnt = m_stAssets.size();

    // 调用 Asset::OnRemove 方法，通知回收子资源
    // 注意这里 Asset 上的 Pool 被 OnRemove 需要，不能在这里断开关联
    auto asset = it->second;
    assert(asset->m_uId != kEmptyAssetId && asset->m_pPool.lock().get() == this);
    asset->OnRemove();

    // 删除关联，注意 it 这个时候可能失效了
    if (cnt != m_stAssets.size())
        it = m_stAssets.find(id);
    assert(it != m_stAssets.end());
    m_stAssets.erase(it);
    if (!asset->GetName().empty())
    {
        auto jt = m_stLookupTable.find(asset->GetName());
        assert(jt != m_stLookupTable.end());
        m_stLookupTable.erase(jt);
    }

    asset->m_uId = kEmptyAssetId;
    asset->m_pPool.reset();
    return {};
}

Result<void> AssetPool::RemoveAsset(std::string_view name) noexcept
{
    auto it = m_stLookupTable.find(name);
    if (it == m_stLookupTable.end())
        return make_error_code(AssetError::AssetNotFound);
    auto ret = RemoveAsset(it->second);
    assert(ret || ret.GetError() != make_error_code(AssetError::AssetNotFound));
    return ret;
}

void AssetPool::Clear(AssetClearTypes type) noexcept
{
    auto cnt = m_stAssets.size();
    auto it = m_stAssets.begin();
    while (it != m_stAssets.end())
    {
        auto id = it->first;
        auto asset = it->second;

        if (type == AssetClearTypes::All || (type == AssetClearTypes::Unused && asset.use_count() == 1))
        {
            // 调用 Asset::OnRemove 方法，通知回收子资源
            // 注意这里 Asset 上的 Pool 被 OnRemove 需要，不能在这里断开关联
            assert(asset->m_uId != kEmptyAssetId);
            asset->OnRemove();

            // 删除关联，注意 it 这个时候可能失效了
            if (cnt != m_stAssets.size())
                it = m_stAssets.find(id);
            assert(it != m_stAssets.end());
            it = m_stAssets.erase(it);
            cnt = m_stAssets.size();
            if (!asset->GetName().empty())
            {
                auto jt = m_stLookupTable.find(asset->GetName());
                assert(jt != m_stLookupTable.end());
                m_stLookupTable.erase(jt);
            }

            asset->m_uId = kEmptyAssetId;
            asset->m_pPool.reset();
        }
        else
        {
            ++it;
        }
    }

    assert(type != AssetClearTypes::All || m_stAssets.empty());
}

AssetPtr AssetPool::GetAsset(std::string_view name) const noexcept
{
    auto it = m_stLookupTable.find(name);
    if (it != m_stLookupTable.end())
    {
        auto ret = GetAsset(it->second);
        assert(ret);
        return ret;
    }
    return nullptr;
}

AssetPtr AssetPool::GetAsset(AssetId id) const noexcept
{
    auto it = m_stAssets.find(id);
    if (it != m_stAssets.end())
        return it->second;
    return nullptr;
}
