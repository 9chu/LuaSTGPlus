/**
 * @file
 * @date 2022/5/14
 * @author 9chu
 * 此文件为 LuaSTGPlus 项目的一部分，版权与许可声明详见 COPYRIGHT.txt。
 */
#include <lstg/Core/Subsystem/Asset/AssetLoader.hpp>

using namespace std;
using namespace lstg;
using namespace lstg::Subsystem::Asset;

AssetLoader::AssetLoader(AssetPtr asset)
    : m_pAsset(std::move(asset))
{
    m_iState.store(AssetLoadingStates::Uninitialized, std::memory_order_release);
}

AssetLoadingStates AssetLoader::GetState() const noexcept
{
    return m_iState.load(std::memory_order_acquire);
}

bool AssetLoader::IsLock() const noexcept
{
    return m_bLocked;
}

void AssetLoader::SetLock(bool v) noexcept
{
    m_bLocked = v;
}

void AssetLoader::SetState(AssetLoadingStates state) noexcept
{
    m_iState.store(state, std::memory_order_release);
}
