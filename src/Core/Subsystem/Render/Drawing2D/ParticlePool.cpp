/**
 * @file
 * @date 2022/7/7
 * @author 9chu
 * 此文件为 LuaSTGPlus 项目的一部分，版权与许可声明详见 COPYRIGHT.txt。
 */
#include <lstg/Core/Subsystem/Render/Drawing2D/ParticlePool.hpp>

#include <SDL_timer.h>

using namespace std;
using namespace lstg;
using namespace lstg::Subsystem::Render::Drawing2D;

ParticlePool::ParticlePool()
{
    // 初始化随机数发生器
    m_stRandomizer.SetSeed(::SDL_GetTicks());

    // 初始化粒子池
    m_stParticles.resize(m_uMaxParticles);
    m_stFreeParticles.resize(m_uMaxParticles);
    for (size_t i = 0; i < m_uMaxParticles; ++i)
        m_stFreeParticles[i] = i;
}

ParticlePool::~ParticlePool()
{
    m_stEmitters.clear();
    assert(m_stFreeParticles.size() == m_uMaxParticles);
}

Result<void> ParticlePool::SetMaxParticles(size_t particles) noexcept
{
    // 只处理变多的情况，缩小比较麻烦
    if (particles <= m_uMaxParticles)
        return make_error_code(errc::invalid_argument);

    try
    {
        auto orgSize = m_stParticles.size();

        // 先用 reserve 预分配
        m_stParticles.reserve(particles);
        m_stFreeParticles.reserve(particles);

        // 扩展粒子池
        m_stParticles.resize(particles);

        // 计入空闲粒子下标
        for (size_t i = orgSize; i < particles; ++i)
            m_stFreeParticles.push_back(i);
    }
    catch (...)  // bad_alloc
    {
        return make_error_code(errc::not_enough_memory);
    }

    m_uMaxParticles = particles;
    return {};
}

Result<void> ParticlePool::AddEmitter(const ParticleConfig* config) noexcept
{
    try
    {
        m_stEmitters.emplace_back(this, config);
        return {};
    }
    catch (...)  // bad_alloc
    {
        return make_error_code(errc::not_enough_memory);
    }
}

void ParticlePool::RemoveEmitter(size_t index) noexcept
{
    if (index >= m_stEmitters.size())
        return;
    auto it = m_stEmitters.begin() + static_cast<vector<ParticleEmitter>::difference_type>(index);
    m_stEmitters.erase(it);
}

void ParticlePool::Update(double elapsed) noexcept
{
    // 更新所有 Emitter
    for (auto& e : m_stEmitters)
        e.Update(elapsed);
}

Result<void> ParticlePool::Draw(CommandBuffer& buffer) noexcept
{
    // 渲染所有 Emitter
    for (auto& e : m_stEmitters)
    {
        auto ret = e.Draw(buffer);
        if (!ret)
            return ret.GetError();
    }
    return {};
}

Result<size_t> ParticlePool::AllocParticle() noexcept
{
    if (m_stFreeParticles.empty())
        return make_error_code(errc::not_enough_memory);

    auto freeIndex = m_stFreeParticles.back();
    m_stFreeParticles.pop_back();

    m_stParticles[freeIndex] = Particle {};
    return freeIndex;
}

void ParticlePool::FreeParticle(size_t index) noexcept
{
    assert(index < m_uMaxParticles);
    m_stFreeParticles.push_back(index);
    assert(m_stFreeParticles.size() <= m_uMaxParticles);
}

Particle& ParticlePool::GetParticle(size_t index) noexcept
{
    assert(index < m_uMaxParticles);
    return m_stParticles[index];
}
