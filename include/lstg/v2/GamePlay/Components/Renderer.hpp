/**
 * @file
 * @date 2022/7/26
 * @author 9chu
 * 此文件为 LuaSTGPlus 项目的一部分，版权与许可声明详见 COPYRIGHT.txt。
 */
#pragma once
#include <lstg/Core/IntrusiveSkipList.hpp>
#include "../../MathAlias.hpp"
#include "../../../Core/ECS/Entity.hpp"
#include "../../../Core/Subsystem/Render/Drawing2D/ParticlePool.hpp"
#include "../../Asset/SpriteAsset.hpp"
#include "../../Asset/SpriteSequenceAsset.hpp"
#include "../../Asset/HgeParticleAsset.hpp"

namespace lstg::v2::GamePlay::Components
{
    static constexpr size_t kRendererSkipListNodeDepth = 3;

    /**
     * 渲染
     */
    struct Renderer
    {
        static Renderer* FromSkipListNode(IntrusiveSkipListNode<kRendererSkipListNodeDepth>* n) noexcept;

        struct SpriteRenderer
        {
            Asset::SpriteAssetPtr Asset;
        };

        struct SpriteSequenceRenderer
        {
            Asset::SpriteSequenceAssetPtr Asset;
        };

        struct ParticleRenderer
        {
            Asset::HgeParticleAssetPtr Asset;
            std::shared_ptr<Subsystem::Render::Drawing2D::ParticlePool> Pool;
            Subsystem::Render::Drawing2D::ParticleEmitter* Emitter = nullptr;
        };

        /**
         * 是否可见
         */
        bool Invisible = false;

        /**
         * 缩放
         */
        Vec2 Scale { 1., 1. };

        /**
         * 渲染层
         */
        double Layer = 0;

        /**
         * 渲染资源 & 状态
         */
        std::variant<std::monostate, SpriteRenderer, SpriteSequenceRenderer, ParticleRenderer> RenderData;

        /**
         * 动画计时器
         */
        uint32_t AnimationTimer = 0;

        /**
         * 链表域
         * 用于保持渲染顺序。
         */
        ECS::Entity BindingEntity;
        IntrusiveSkipListNode<kRendererSkipListNodeDepth> SkipListNode;

        Renderer() noexcept {} /* = default; */  // g++ won't compile, make it happy
        Renderer(Renderer&& org) noexcept;

        void Reset() noexcept;
        std::string_view GetAssetName() noexcept;

        Renderer* NextNode() noexcept;
        Renderer* PrevNode() noexcept;
    };

    constexpr uint32_t GetComponentId(Renderer*) noexcept
    {
        return 4u;
    }

    /**
     * 根元素
     */
    struct RendererRoot
    {
        Renderer RendererHeader;
        Renderer RendererTailer;

        RendererRoot() noexcept;
        RendererRoot(RendererRoot&&) noexcept;
        void Reset() noexcept;
    };

    constexpr uint32_t GetComponentId(RendererRoot*) noexcept
    {
        return 5u;
    }
}
