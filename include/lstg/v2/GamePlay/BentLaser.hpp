/**
* @file
* @date 2022/8/7
* @author 9chu
* 此文件为 LuaSTGPlus 项目的一部分，版权与许可声明详见 COPYRIGHT.txt。
*/
#pragma once
#include <lstg/Core/CircularQueue.hpp>
#include "ScriptObjectPool.hpp"
#include "../MathAlias.hpp"
#include "../BlendMode.hpp"

namespace lstg::v2::GamePlay
{
    /**
     * 曲线激光
     *
     * lstg 中的曲线激光对象，由于脱离 GamePlay 存在，使用上较为不便。
     * 将在未来版本中移除。
     */
    class BentLaser
    {
        struct LaserNode
        {
            Vec2 Location;
            double HalfWidth = 0.;
        };

        enum {
            kMaxLaserNodes = 512
        };

    public:
        bool Update(ScriptObjectId id, int32_t length, double width) noexcept;
        void Release() noexcept;
        bool Render(const char* textureName, BlendMode blend, Subsystem::Render::ColorRGBA32 c, Math::UVRectangle textureRect,
            double scale) const noexcept;
        bool CollisionCheck(double x, double y, double rot, double a, double b, bool rect) const noexcept;
        bool BoundCheck() const noexcept;

    private:
        CircularQueue<LaserNode, kMaxLaserNodes> m_stQueue;
        double m_dLength = 0.;  // 记录激光长度
    };
}
