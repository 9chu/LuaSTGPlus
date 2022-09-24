/**
 * @file
 * @date 2022/7/7
 * @author 9chu
 * 此文件为 LuaSTGPlus 项目的一部分，版权与许可声明详见 COPYRIGHT.txt。
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
         * 获取关联的粒子配置
         */
        const ParticleConfig* GetConfig() const noexcept { return m_pConfig; }

        /**
         * 获取粒子数
         */
        size_t GetParticleCount() const noexcept { return m_stParticles.size(); }

        /**
         * 是否存活
         */
        bool IsAlive() const noexcept { return m_bAlive; }

        /**
         * 获取存活时间
         */
        float GetAge() const noexcept { return m_fAge; }

        /**
         * 设置为存活
         */
        void SetAlive() noexcept
        {
            m_bAlive = true;
            m_fAge = 0;
        }

        /**
         * 设置失效
         */
        void SetDead() noexcept
        {
            m_bAlive = false;
        }

        /**
         * 发射密度覆盖量
         */
        std::optional<float> GetEmissionOverride() const noexcept { return m_stEmissionOverride; }

        /**
         * 设置发射密度覆盖量
         * @param v 值
         */
        void SetEmissionOverride(std::optional<float> v) noexcept
        {
            if (v)
                m_stEmissionOverride = std::max(0.f, *v);
            else
                m_stEmissionOverride = {};
        }

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
        std::optional<float> m_stEmissionOverride;  // 发射密度覆盖

        // 变换
        glm::vec2 m_stPosition { 0.f, 0.f };
        glm::vec2 m_stLastPosition { 0.f, 0.f };
        float m_fRotation = 0.f;  // 发射器旋转角度
        glm::vec2 m_stScale { 1.f, 1.f };  // 缩放
    };
}
