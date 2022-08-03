/**
 * @file
 * @date 2022/7/7
 * @author 9chu
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#include <lstg/Core/Subsystem/Render/Drawing2D/ParticleEmitter.hpp>

#include <lstg/Core/Logging.hpp>
#include <lstg/Core/Math/VectorHelper.hpp>
#include <lstg/Core/Subsystem/Render/Drawing2D/ParticlePool.hpp>
#include <lstg/Core/Subsystem/Render/Drawing2D/SpriteDrawing.hpp>

using namespace std;
using namespace lstg;
using namespace lstg::Subsystem::Render::Drawing2D;

LSTG_DEF_LOG_CATEGORY(ParticleEmitter);

namespace
{
    inline float GenRandomFloat(Math::Randomizer& randomizer, const RandomRange& range) noexcept
    {
        float lower = std::get<0>(range);
        float upper = std::get<1>(range);
        if (upper < lower)
            std::swap(lower, upper);
        return randomizer.NextFloat(lower, upper);
    }
}

ParticleEmitter::ParticleEmitter(ParticlePool* pool, const ParticleConfig* config) noexcept
    : m_pPool(pool), m_pConfig(config)
{
    assert(pool && config);
    m_stParticles.reserve(pool->GetMaxParticles());
}

ParticleEmitter::ParticleEmitter(ParticleEmitter&& rhs) noexcept
    : m_pPool(rhs.m_pPool), m_pConfig(rhs.m_pConfig), m_stParticles(std::move(rhs.m_stParticles)), m_bAlive(rhs.m_bAlive),
    m_fAge(rhs.m_fAge), m_fEmissionResidue(rhs.m_fEmissionResidue), m_stPosition(rhs.m_stPosition), m_stLastPosition(rhs.m_stLastPosition),
    m_fRotation(rhs.m_fRotation), m_stScale(rhs.m_stScale)
{
}

ParticleEmitter::~ParticleEmitter()
{
    // 从池中删除
    Clear();
}

ParticleEmitter& ParticleEmitter::operator=(ParticleEmitter&& rhs) noexcept
{
    Clear();

    m_pPool = rhs.m_pPool;
    m_pConfig = rhs.m_pConfig;
    m_stParticles = std::move(rhs.m_stParticles);
    m_bAlive = rhs.m_bAlive;
    m_fAge = rhs.m_fAge;
    m_fEmissionResidue = rhs.m_fEmissionResidue;
    m_stPosition = rhs.m_stPosition;
    m_stLastPosition = rhs.m_stLastPosition;
    m_fRotation = rhs.m_fRotation;
    m_stScale = rhs.m_stScale;
    return *this;
}

void ParticleEmitter::SetPosition(glm::vec2 pos) noexcept
{
    m_stPosition = pos;
    if (m_fAge == 0.f)
        m_stLastPosition = pos;
}

void ParticleEmitter::Update(double elapsed) noexcept
{
    auto delta = static_cast<float>(elapsed);

    // 刷新发射器状态
    if (m_bAlive)
    {
        m_fAge += delta;
        if (m_fAge >= m_pConfig->LifeTime && m_pConfig->LifeTime >= 0.f)
            m_bAlive = false;
    }

    // 更新所有粒子
    for (auto it = m_stParticles.begin(); it != m_stParticles.end(); )
    {
        auto index = *it;
        auto& particle = m_pPool->GetParticle(index);

        // 更新生命周期
        particle.Age += delta;
        if (particle.Age >= particle.AliveTime)
        {
            m_pPool->FreeParticle(index);
            it = m_stParticles.erase(it);
            continue;
        }

        // 计算线加速度和切向加速度
        glm::vec2 radialAcceleration = Math::Normalize(particle.Location - m_stPosition);
        glm::vec2 tangentialAcceleration = { -radialAcceleration.y, radialAcceleration.x };
        glm::vec2 gravityAcceleration = m_pConfig->GravityDirection;
        radialAcceleration *= particle.RadialAcceleration;
        tangentialAcceleration *= particle.TangentialAcceleration;
        gravityAcceleration *= particle.Gravity;

        // 计算速度
        particle.Velocity += ((radialAcceleration + tangentialAcceleration + gravityAcceleration) * delta);

        // 计算位置
        particle.Location += particle.Velocity * delta;

        // 计算自旋和大小
        particle.Spin += particle.SpinDelta * delta;
        particle.Size += particle.SizeDelta * delta;
        particle.Color += particle.ColorDelta * delta;

        ++it;
    }

    // 产生新的粒子
    if (m_bAlive)
    {
        float emission = m_stEmissionOverride ? *m_stEmissionOverride : static_cast<float>(m_pConfig->EmissionPerSecond);
        float particlesToCreate = emission * delta + m_fEmissionResidue;
        auto particlesToCreateFloor = ::floor(particlesToCreate);
        m_fEmissionResidue = particlesToCreate - particlesToCreateFloor;

        assert(particlesToCreateFloor >= 0);
        try
        {
            m_stParticles.reserve(m_stParticles.size() + static_cast<size_t>(particlesToCreateFloor));
        }
        catch (...)  // bad_alloc
        {
            LSTG_LOG_ERROR_CAT(ParticleEmitter, "Cannot alloc memory for particle pool");
            return;
        }

        for (size_t i = 0; i < static_cast<size_t>(particlesToCreateFloor); ++i)
        {
            // 创建新粒子
            size_t index = 0;
            {
                auto createdIndex = m_pPool->AllocParticle();
                if (!createdIndex)
                    break;
                index = *createdIndex;
                m_stParticles.push_back(index);
            }
            auto& particle = m_pPool->GetParticle(index);
            auto& randomizer = m_pPool->GetRandomizer();

            // 初始化生命期
            particle.Age = 0.f;
            particle.AliveTime = GenRandomFloat(randomizer, m_pConfig->ParticleLifeTime);

            // 初始化坐标，此时给一个较小的扰动
            particle.Location = m_stLastPosition + (m_stPosition - m_stLastPosition) * randomizer.NextFloat();
            particle.Location += glm::vec2 { randomizer.NextFloat() * 2.f, randomizer.NextFloat() * 2.f };

            // 计算发射角度和初速度
            auto ang = (randomizer.NextFloat() - 0.5f) * m_pConfig->Spread;
            switch (m_pConfig->EmitDirection)
            {
                case ParticleEmitDirection::Fixed:
                    ang += m_pConfig->Direction - glm::half_pi<float>();
                    break;
                case ParticleEmitDirection::RelativeToSpeed:
                    ang += m_pConfig->Direction + Math::Angle(m_stLastPosition - m_stPosition);
                    break;
                case ParticleEmitDirection::OppositeToEmitter:
                    ang += m_fRotation - glm::pi<float>();
                    break;
                default:
                    assert(false);
                    break;
            }
            particle.Velocity = glm::vec2 { ::cos(ang), ::sin(ang) } * GenRandomFloat(randomizer, m_pConfig->Speed);

            // 计算加速度
            particle.Gravity = GenRandomFloat(randomizer, m_pConfig->Gravity);
            particle.RadialAcceleration = GenRandomFloat(randomizer, m_pConfig->RadialAcceleration);
            particle.TangentialAcceleration = GenRandomFloat(randomizer, m_pConfig->TangentialAcceleration);

            // 自旋
            particle.Spin = GenRandomFloat(randomizer, { m_pConfig->SpinInitial, m_pConfig->SpinInitial +
                (m_pConfig->SpinFinal - m_pConfig->SpinInitial) * m_pConfig->SpinVariant });
            particle.SpinDelta = (m_pConfig->SpinFinal - particle.Spin) / particle.AliveTime;

            // 大小
            particle.Size = GenRandomFloat(randomizer, { m_pConfig->SizeInitial, m_pConfig->SizeInitial +
                (m_pConfig->SizeFinal - m_pConfig->SizeInitial) * m_pConfig->SizeVariant });
            particle.SizeDelta = (m_pConfig->SizeFinal - particle.Size) / particle.AliveTime;

            // 颜色
            particle.Color.x = GenRandomFloat(randomizer, { m_pConfig->ColorInitial.x, m_pConfig->ColorInitial.x +
                (m_pConfig->ColorFinal.x - m_pConfig->ColorInitial.x) * m_pConfig->ColorVariant });
            particle.Color.y = GenRandomFloat(randomizer, { m_pConfig->ColorInitial.y, m_pConfig->ColorInitial.y +
                (m_pConfig->ColorFinal.y - m_pConfig->ColorInitial.y) * m_pConfig->ColorVariant });
            particle.Color.z = GenRandomFloat(randomizer, { m_pConfig->ColorInitial.z, m_pConfig->ColorInitial.z +
                (m_pConfig->ColorFinal.z - m_pConfig->ColorInitial.z) * m_pConfig->ColorVariant });
            particle.Color.w = GenRandomFloat(randomizer, { m_pConfig->ColorInitial.w, m_pConfig->ColorInitial.w +
                (m_pConfig->ColorFinal.w - m_pConfig->ColorInitial.w) * m_pConfig->AlphaVariant });
            particle.ColorDelta.x = (m_pConfig->ColorFinal.x - particle.Color.x) / particle.AliveTime;
            particle.ColorDelta.y = (m_pConfig->ColorFinal.y - particle.Color.y) / particle.AliveTime;
            particle.ColorDelta.z = (m_pConfig->ColorFinal.z - particle.Color.z) / particle.AliveTime;
            particle.ColorDelta.w = (m_pConfig->ColorFinal.w - particle.Color.w) / particle.AliveTime;
        }
    }

    m_stLastPosition = m_stPosition;
}

Result<void> ParticleEmitter::Draw(CommandBuffer& buffer) noexcept
{
    for (auto index : m_stParticles)
    {
        auto& particle = m_pPool->GetParticle(index);

        auto draw = m_pConfig->ParticleSprite->Draw(buffer, m_pConfig->ColorBlend);
        if (!draw)
            return draw.GetError();

        // 调整顶点色
        if (m_pConfig->ColorInitial.x < 0)  // r < 0
        {
            // 使用原先的顶点色
            for (size_t i = 0; i < 4; ++i)
            {
                auto& color0 = draw->GetAdditiveColor(i);
                auto& color1 = draw->GetMultiplyColor(i);

                color0.a(static_cast<uint8_t>(clamp(particle.Color.w * 255.f, 0.f, 255.f)));
                color1.a(color0.a());
            }
        }
        else
        {
            // 覆盖顶点色
            for (size_t i = 0; i < 4; ++i)
            {
                auto& color0 = draw->GetAdditiveColor(i);
                auto& color1 = draw->GetMultiplyColor(i);

                if (m_pConfig->IsVertexColorBlendByMultiply)
                {
                    color0 = ColorRGBA32(0, 0, 0, 255);

                    color1.r(static_cast<uint8_t>(clamp(particle.Color.x * 255.f, 0.f, 255.f)));
                    color1.g(static_cast<uint8_t>(clamp(particle.Color.y * 255.f, 0.f, 255.f)));
                    color1.b(static_cast<uint8_t>(clamp(particle.Color.z * 255.f, 0.f, 255.f)));
                    color1.a(static_cast<uint8_t>(clamp(particle.Color.w * 255.f, 0.f, 255.f)));
                }
                else
                {
                    color0.r(static_cast<uint8_t>(clamp(particle.Color.x * 255.f, 0.f, 255.f)));
                    color0.g(static_cast<uint8_t>(clamp(particle.Color.y * 255.f, 0.f, 255.f)));
                    color0.b(static_cast<uint8_t>(clamp(particle.Color.z * 255.f, 0.f, 255.f)));
                    color0.a(static_cast<uint8_t>(clamp(particle.Color.w * 255.f, 0.f, 255.f)));

                    color1 = ColorRGBA32(0, 0, 0, 255);
                }
            }
        }

        // 设置位置
        auto offset = particle.Location - m_stPosition;
        offset *= m_stScale;
        offset += m_stPosition;
        draw->Transform(particle.Spin, particle.Size * m_stScale.x, particle.Size * m_stScale.y);
        draw->Translate(offset.x, offset.y, 0.5f);
    }
    return {};
}

void ParticleEmitter::Clear() noexcept
{
    for (auto it = m_stParticles.begin(); it != m_stParticles.end(); )
    {
        auto index = *it;
        m_pPool->FreeParticle(index);
        it = m_stParticles.erase(it);
    }
}
