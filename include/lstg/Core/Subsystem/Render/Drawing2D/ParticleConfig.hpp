/**
 * @file
 * @date 2022/7/7
 * @author 9chu
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#pragma once
#include <nlohmann/json.hpp>
#include "../../VFS/IStream.hpp"
#include "Sprite.hpp"

namespace lstg::Subsystem::Render::Drawing2D
{
    using RandomRange = std::tuple<float, float>;

    /**
     * 粒子发射方向
     */
    enum class ParticleEmitDirection
    {
        Fixed = 0,  ///< @brief 固定
        RelativeToSpeed,  ///< @brief 相对于速度
        OppositeToEmitter,  ///< @brief 发射器反向
    };

    /**
     * 粒子配置
     */
    struct ParticleConfig
    {
        // 绑定的 Sprite
        const Sprite* ParticleSprite = nullptr;

        // 混合模式
        ColorBlendMode ColorBlend = ColorBlendMode::Alpha;  ///< @brief 混合模式
        bool IsVertexColorBlendByMultiply = false;  ///< @brief 是否使用乘算

        // 发射器参数
        uint32_t EmissionPerSecond = 0;  ///< @brief 每秒发射个数
        float LifeTime = 0.f;  ///< @brief 生命期
        float Direction = 0.f;  ///< @brief 发射方向
        float Spread = 0.f;  ///< @brief 扩散角度
        ParticleEmitDirection EmitDirection = ParticleEmitDirection::Fixed;  ///< @brief 发射方向选择

        // 粒子参数
        RandomRange ParticleLifeTime = { 0.f, 0.f };  ///< @brief 粒子生命期
        RandomRange Speed = { 0.f, 0.f };  ///< @brief 粒子初始速度
        RandomRange Gravity = { 0.f, 0.f };  ///< @brief 粒子重力加速度
        glm::vec2 GravityDirection = { 0, 1.f };  ///< @brief 重力方向
        RandomRange RadialAcceleration = { 0.f, 0.f };  ///< @brief 线加速度
        RandomRange TangentialAcceleration = { 0.f, 0.f };  ///< @brief 角加速度
        float SizeInitial = 0.f;  ///< @brief 起始大小
        float SizeFinal = 0.f;  ///< @brief 最终大小
        float SizeVariant = 0.f;  ///< @brief 大小抖动值
        float SpinInitial = 0.f;  ///< @brief 起始自旋
        float SpinFinal = 0.f;  ///< @brief 最终自旋
        float SpinVariant = 0.f;  ///< @brief 自旋抖动值
        glm::vec4 ColorInitial;  ///< @brief 起始颜色 RGBA
        glm::vec4 ColorFinal;  ///< @brief 最终颜色 RGBA
        float ColorVariant = 0.f;  ///< @brief 颜色抖动值
        float AlphaVariant = 0.f;  ///< @brief Alpha抖动值

        /**
         * 从 HGE 粒子数据流加载
         * @note 未定义的字段保留原有值
         * @param stream 流
         */
        Result<void> ReadFromHGE(VFS::IStream* stream) noexcept;

        /**
         * 从 JSON 加载
         * @note 未定义的字段保留原有值
         * @param json JSON
         */
        void ReadFrom(const nlohmann::json& json) noexcept;
    };
}
