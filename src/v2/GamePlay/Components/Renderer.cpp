/**
 * @file
 * @date 2022/7/26
 * @author 9chu
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#include <lstg/v2/GamePlay/Components/Renderer.hpp>

using namespace std;
using namespace lstg;
using namespace lstg::v2::GamePlay::Components;

// <editor-fold desc="Renderer">

Renderer::Renderer(Renderer&& org) noexcept
    : Invisible(org.Invisible), Scale(org.Scale), Layer(org.Layer), RenderData(std::move(org.RenderData)), BindingEntity(org.BindingEntity),
    PrevInChain(org.PrevInChain), NextInChain(org.NextInChain)
{
    // 调整链表指向
    if (PrevInChain)
        PrevInChain->NextInChain = this;
    if (NextInChain)
        NextInChain->PrevInChain = this;
    org.PrevInChain = nullptr;
    org.NextInChain = nullptr;
}

void Renderer::Reset() noexcept
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

    Invisible = false;
    Scale = { 1., 1. };
    Layer = 0.;
    RenderData = {};
    AnimationTimer = 0;
    BindingEntity = {};
    PrevInChain = nullptr;
    NextInChain = nullptr;
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

// </editor-fold>
// <editor-fold desc="RendererRoot">

RendererRoot::RendererRoot() noexcept
{
    // Header <-> Tailer
    RendererHeader.NextInChain = &RendererTailer;
    RendererTailer.PrevInChain = &RendererHeader;
}

void RendererRoot::Reset() noexcept
{
    // 将链表脱开
    RendererHeader.Reset();
    RendererTailer.Reset();

    // Header <-> Tailer
    RendererHeader.NextInChain = &RendererTailer;
    RendererTailer.PrevInChain = &RendererHeader;
}

// </editor-fold>
