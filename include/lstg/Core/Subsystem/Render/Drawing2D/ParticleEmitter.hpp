/**
 * @file
 * @date 2022/7/7
 * @author 9chu
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#pragma once
#include "Particle.hpp"
#include "ParticleConfig.hpp"

namespace lstg::Subsystem::Render::Drawing2D
{
    class ParticlePool;

    /**
     * 粒子发射器
     */
    class ParticleEmitter
    {
    public:
        ParticleEmitter(ParticlePool* pool, const ParticleConfig* config) noexcept;
        ParticleEmitter(const ParticleEmitter&) = delete;
        ParticleEmitter(ParticleEmitter&& rhs) noexcept;
        ~ParticleEmitter();

        ParticleEmitter& operator=(const ParticleEmitter&) = delete;
        ParticleEmitter& operator=(ParticleEmitter&& rhs) noexcept;

    public:
        /**
         * 是否存活
         */
        bool IsAlive() const noexcept { return m_bAlive; }

        /**
         * 获取当前位置
         */
        [[nodiscard]] glm::vec2 GetPosition() const noexcept { return m_stPosition; }

        /**
         * 设置当前位置
         * @param pos 位置
         */
        void SetPosition(glm::vec2 pos) noexcept;

        /**
         * 获取旋转
         */
        [[nodiscard]] float GetRotation() const noexcept { return m_fRotation; }

        /**
         * 设置旋转
         * @param rot 旋转
         */
        void SetRotation(float rot) noexcept { m_fRotation = rot; }

        /**
         * 获取缩放
         */
        [[nodiscard]] glm::vec2 GetScale() const noexcept { return m_stScale; }

        /**
         * 设置缩放
         * @param scale 缩放量
         */
        void SetScale(glm::vec2 scale) noexcept { m_stScale = scale; }

        /**
         * 更新状态
         * @param elapsed 流逝时间
         */
        void Update(double elapsed) noexcept;

        /**
         * 绘制
         * @param buffer 缓冲区
         */
        Result<void> Draw(CommandBuffer& buffer) noexcept;

        /**
         * 删除所有粒子
         */
        void Clear() noexcept;

    private:
        ParticlePool* m_pPool = nullptr;
        const ParticleConfig* m_pConfig = nullptr;
        std::vector<size_t> m_stParticles;  // 管理的粒子

        bool m_bAlive = true;
        float m_fAge = 0.;
        float m_fEmissionResidue = 0.f;  // 发射量缺口

        // 变换
        glm::vec2 m_stPosition { 0.f, 0.f };
        glm::vec2 m_stLastPosition { 0.f, 0.f };
        float m_fRotation = 0.f;  // 发射器旋转角度
        glm::vec2 m_stScale { 1.f, 1.f };  // 缩放
    };
}
