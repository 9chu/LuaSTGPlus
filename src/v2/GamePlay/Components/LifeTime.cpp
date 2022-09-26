/**
 * @file
 * @date 2022/7/26
 * @author 9chu
 * 此文件为 LuaSTGPlus 项目的一部分，版权与许可声明详见 COPYRIGHT.txt。
 */
#include <lstg/v2/GamePlay/Components/LifeTime.hpp>

#include <cstddef>

using namespace std;
using namespace lstg;
using namespace lstg::v2::GamePlay::Components;

const char* v2::GamePlay::Components::ToString(LifeTimeStatus status) noexcept
{
    switch (status)
    {
        case LifeTimeStatus::Alive:
            return "Alive";
        case LifeTimeStatus::Deleted:
            return "Deleted";
        case LifeTimeStatus::Killed:
            return "Killed";
        default:
            assert(false);
            return "<unknown>";
    }
}

// <editor-fold desc="LifeTime">

LifeTime* LifeTime::FromListNode(IntrusiveListNode* n) noexcept
{
    if (n)
    {
        auto ret = reinterpret_cast<LifeTime*>(reinterpret_cast<uint8_t*>(n) - offsetof(LifeTime, ListNode));
        assert(&ret->ListNode == n);
        return ret;
    }
    return nullptr;
}

LifeTime::LifeTime(LifeTime&& org) noexcept
    : Status(org.Status), OutOfBoundaryAutoRemove(org.OutOfBoundaryAutoRemove), Timer(org.Timer), UniqueId(org.UniqueId),
    BindingEntity(org.BindingEntity), ListNode(std::move(org.ListNode))
{
}

void LifeTime::Reset() noexcept
{
    assert(!BindingEntity || Status != LifeTimeStatus::Alive);

    // 从链表脱开
    ListRemove(&ListNode);

    // 重置
    Status = LifeTimeStatus::Alive;
    OutOfBoundaryAutoRemove = true;
    Timer = 0;
    UniqueId = 0;
    BindingEntity = {};
}

LifeTime* LifeTime::NextNode() noexcept
{
    auto n = ListNode.Next;
    return FromListNode(n);
}

LifeTime* LifeTime::PrevNode() noexcept
{
    auto n = ListNode.Prev;
    return FromListNode(n);
}

// </editor-fold>
// <editor-fold desc="LifeTimeRoot">

LifeTimeRoot::LifeTimeRoot() noexcept
{
    // Header <-> Tailer
    LifeTimeHeader.ListNode.Next = &LifeTimeTailer.ListNode;
    LifeTimeTailer.ListNode.Prev = &LifeTimeHeader.ListNode;
}

LifeTimeRoot::LifeTimeRoot(LifeTimeRoot&&) noexcept
{
    // 不应该发生内存迁移
    assert(false);
}

void LifeTimeRoot::Reset() noexcept
{
    // 将链表脱开
    LifeTimeHeader.Reset();
    LifeTimeTailer.Reset();

    // Header <-> Tailer
    LifeTimeHeader.ListNode.Next = &LifeTimeTailer.ListNode;
    LifeTimeTailer.ListNode.Prev = &LifeTimeHeader.ListNode;
}

// </editor-fold>
