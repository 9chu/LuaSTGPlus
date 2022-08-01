/**
 * @file
 * @date 2022/7/26
 * @author 9chu
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#pragma once
#include "../../MathAlias.hpp"
#include "../../../Core/ECS/Entity.hpp"
#include "../../../Core/Subsystem/Render/Drawing2D/ParticlePool.hpp"
#include "../../Asset/SpriteAsset.hpp"
#include "../../Asset/SpriteSequenceAsset.hpp"
#include "../../Asset/HgeParticleAsset.hpp"

namespace lstg::v2::GamePlay::Components
{
    /**
     * 渲染
     */
    struct Renderer
    {
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
        Renderer* PrevInChain = nullptr;
        Renderer* NextInChain = nullptr;

        Renderer() noexcept = default;
        Renderer(Renderer&& org) noexcept;

        void Reset() noexcept;
        std::string_view GetAssetName() noexcept;
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
        void Reset() noexcept;
    };

    constexpr uint32_t GetComponentId(RendererRoot*) noexcept
    {
        return 5u;
    }
}
