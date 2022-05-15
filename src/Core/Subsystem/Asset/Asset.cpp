/**
 * @file
 * @date 2022/5/14
 * @author 9chu
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#include <lstg/Core/Subsystem/Asset/Asset.hpp>

#include <cassert>
#include "detail/WeakPtrTraits.hpp"

using namespace std;
using namespace lstg;
using namespace lstg::Subsystem::Asset;

Asset::Asset(std::string name)
    : m_stName(std::move(name))
{
}

Asset::~Asset() noexcept
{
    assert(m_uId == kEmptyAssetId && detail::IsWeakPtrUninitialized(m_pPool));
}

void Asset::SetState(AssetStates s) noexcept
{
    // 限制状态转换
    assert((m_iState == AssetStates::Uninitialized && (s == AssetStates::Loaded || s == AssetStates::Error)) ||
        (m_iState == AssetStates::Loaded && s == AssetStates::Error) ||
        (m_iState == AssetStates::Error && s == AssetStates::Error));
    m_iState = s;
}

void Asset::OnRemove() noexcept
{
}
