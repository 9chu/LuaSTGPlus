/**
* @file
* @date 2022/7/7
* @author 9chu
* 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
*/
#pragma once
#include <vector>
#include <lstg/Core/Math/Randomizer.hpp>
#include "Particle.hpp"
#include "ParticleEmitter.hpp"

namespace lstg::Subsystem::Render::Drawing2D
{
    /**
     * 粒子池
     */
    class ParticlePool
    {
        friend class ParticleEmitter;

    public:
        ParticlePool();
        ~ParticlePool();

        ParticlePool(const ParticlePool&) = delete;
        ParticlePool(ParticlePool&&) noexcept = delete;

        ParticlePool& operator=(const ParticlePool&) = delete;
        ParticlePool& operator=(ParticlePool&&) noexcept = delete;

    public:
        /**
         * 获取粒子上限
         */
        size_t GetMaxParticles() const noexcept { return m_uMaxParticles; }

        /**
         * 设置粒子上限
         * @param particles 粒子数
         */
        Result<void> SetMaxParticles(size_t particles) noexcept;

        /**
         * 获取发射器
         * @param index 索引
         */
        ParticleEmitter& GetEmitter(size_t index) noexcept { return m_stEmitters[index]; }
        const ParticleEmitter& GetEmitter(size_t index) const noexcept { return m_stEmitters[index]; }

        /**
         * 增加发射器
         * @param config 配置
         */
        Result<void> AddEmitter(const ParticleConfig* config) noexcept;

        /**
         * 删除发射器
         * @param index 索引
         */
        void RemoveEmitter(size_t index) noexcept;

        /**
         * 更新状态
         * @param elapsed 流逝时间
         */
        void Update(double elapsed) noexcept;

        /**
         * 渲染
         * @param buffer 缓冲区
         */
        Result<void> Draw(CommandBuffer& buffer) noexcept;

    private:
        /**
         * 获取随机数发生器
         */
        Math::Randomizer& GetRandomizer() noexcept { return m_stRandomizer; }

        /**
         * 创建粒子
         * @return 粒子下标
         */
        Result<size_t> AllocParticle() noexcept;

        /**
         * 释放粒子
         */
        void FreeParticle(size_t index) noexcept;

        /**
         * 获取粒子
         */
        Particle& GetParticle(size_t index) noexcept;

    private:
        Math::Randomizer m_stRandomizer;
        size_t m_uMaxParticles = 500u;
        std::vector<Particle> m_stParticles;
        std::vector<size_t> m_stFreeParticles;
        std::vector<ParticleEmitter> m_stEmitters;
    };
}
