/**
 * @file
 * @date 2022/5/14
 * @author 9chu
 * 此文件为 LuaSTGPlus 项目的一部分，版权与许可声明详见 COPYRIGHT.txt。
 */
#pragma once
#include <memory>

namespace lstg::Subsystem::Asset::detail
{
    // https://stackoverflow.com/questions/45507041/how-to-check-if-weak-ptr-is-empty-non-assigned
    template <typename T>
    bool IsWeakPtrUninitialized(const std::weak_ptr<T>& weak) noexcept
    {
        using wt = std::weak_ptr<T>;
        return !weak.owner_before(wt{}) && !wt{}.owner_before(weak);
    }
}
