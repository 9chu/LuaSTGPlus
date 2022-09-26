/**
 * @file
 * @date 2022/7/26
 * @author 9chu
 * 此文件为 LuaSTGPlus 项目的一部分，版权与许可声明详见 COPYRIGHT.txt。
 */
#include <lstg/v2/GamePlay/Components/Renderer.hpp>

using namespace std;
using namespace lstg;
using namespace lstg::v2::GamePlay::Components;

// <editor-fold desc="Renderer">

Renderer* Renderer::FromSkipListNode(IntrusiveSkipListNode<kRendererSkipListNodeDepth>* n) noexcept
{
    if (n)
    {
        auto ret = reinterpret_cast<Renderer*>(reinterpret_cast<uint8_t*>(n) - offsetof(Renderer, SkipListNode));
        assert(&ret->SkipListNode == n);
        return ret;
    }
    return nullptr;
}

Renderer::Renderer(Renderer&& org) noexcept
    : Invisible(org.Invisible), Scale(org.Scale), Layer(org.Layer), RenderData(std::move(org.RenderData)), BindingEntity(org.BindingEntity),
    SkipListNode(std::move(org.SkipListNode))
{
}

void Renderer::Reset() noexcept
{
    // 从链表脱开
    SkipListRemove(&SkipListNode);

    // 重置
    Invisible = false;
    Scale = { 1., 1. };
    Layer = 0.;
    RenderData = {};
    AnimationTimer = 0;
    BindingEntity = {};
}

std::string_view Renderer::GetAssetName() noexcept
{
    switch (RenderData.index())
    {
        case 0:
            return {};
        case 1:
            assert(std::get<1>(RenderData).Asset);
            return std::get<1>(RenderData).Asset->GetName();
        case 2:
            assert(std::get<2>(RenderData).Asset);
            return std::get<2>(RenderData).Asset->GetName();
        case 3:
            assert(std::get<3>(RenderData).Asset);
            return std::get<3>(RenderData).Asset->GetName();
        default:
            assert(false);
            return {};
    }
}

Renderer* Renderer::NextNode() noexcept
{
    auto n = SkipListNode.Adj[0].Next;
    return FromSkipListNode(n);
}

Renderer* Renderer::PrevNode() noexcept
{
    auto n = SkipListNode.Adj[0].Prev;
    return FromSkipListNode(n);
}

// </editor-fold>
// <editor-fold desc="RendererRoot">

RendererRoot::RendererRoot() noexcept
{
    // Header <-> Tailer
    for (size_t i = 0; i < kRendererSkipListNodeDepth; ++i)
    {
        RendererHeader.SkipListNode.Adj[i].Next = &RendererTailer.SkipListNode;
        RendererTailer.SkipListNode.Adj[i].Prev = &RendererHeader.SkipListNode;
    }
}

RendererRoot::RendererRoot(RendererRoot&&) noexcept
{
    // 不应该发生内存迁移
    assert(false);
}

void RendererRoot::Reset() noexcept
{
    // 将链表脱开
    RendererHeader.Reset();
    RendererTailer.Reset();

    // Header <-> Tailer
    for (size_t i = 0; i < kRendererSkipListNodeDepth; ++i)
    {
        RendererHeader.SkipListNode.Adj[i].Next = &RendererTailer.SkipListNode;
        RendererTailer.SkipListNode.Adj[i].Prev = &RendererHeader.SkipListNode;
    }
}

// </editor-fold>
