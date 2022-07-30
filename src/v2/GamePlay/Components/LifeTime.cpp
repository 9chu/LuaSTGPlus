/**
 * @file
 * @date 2022/7/26
 * @author 9chu
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#include <lstg/v2/GamePlay/Components/LifeTime.hpp>

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

void LifeTime::Reset() noexcept
{
    // 从链表脱开
    if (PrevInChain)
    {
        assert(PrevInChain->NextInChain == this);
        PrevInChain->NextInChain = NextInChain;
    }
    if (NextInChain)
    {
        assert(NextInChain->PrevInChain == this);
        NextInChain->PrevInChain = PrevInChain;
    }

    Status = LifeTimeStatus::Alive;
    OutOfBoundaryAutoRemove = true;
    Timer = 0;
    UniqueId = 0;
    BindingEntity = {};
    PrevInChain = nullptr;
    NextInChain = nullptr;
}

// </editor-fold>
// <editor-fold desc="LifeTimeRoot">

LifeTimeRoot::LifeTimeRoot() noexcept
{
    // Header <-> Tailer
    LifeTimeHeader.NextInChain = &LifeTimeTailer;
    LifeTimeTailer.PrevInChain = &LifeTimeHeader;
}

void LifeTimeRoot::Reset() noexcept
{
    // 将链表脱开
    LifeTimeHeader.Reset();
    LifeTimeTailer.Reset();

    // Header <-> Tailer
    LifeTimeHeader.NextInChain = &LifeTimeTailer;
    LifeTimeTailer.PrevInChain = &LifeTimeHeader;
}

// </editor-fold>
