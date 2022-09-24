/**
 * @file
 * @author 9chu
 * @date 2022/3/4
 * 此文件为 LuaSTGPlus 项目的一部分，版权与许可声明详见 COPYRIGHT.txt。
 */
#pragma once
#include <lstg/Core/Subsystem/Script/LuaRead.hpp>
#include <lstg/Core/Subsystem/Script/AutoBridgeHint.hpp>
#include "../GamePlay/BentLaser.hpp"
#include "LSTGColor.hpp"

namespace lstg::v2::Bridge
{
    /**
     * 内建曲线激光
     */
    LSTG_CLASS()
    class LSTGBentLaserData
    {
    public:
        using LuaStack = Subsystem::Script::LuaStack;
        using AbsIndex = LuaStack::AbsIndex;

    public:
        LSTGBentLaserData() = default;

    public:
        /**
         * 更新对象
         * @param stack Lua栈
         * @param baseObject 基准对象，以基准对象为起始点构造曲光
         * @param length 节点数
         * @param width 曲光宽度，同时用于影响碰撞
         */
        LSTG_METHOD()
        void Update(LuaStack& stack, AbsIndex baseObject, int32_t length, uint32_t width);

        /**
         * 回收对象
         * @deprecated 总是交由 GC 完成回收
         */
        LSTG_METHOD()
        void Release();

        /**
         * 渲染曲线激光
         * @param texture 曲线激光纹理
         * @param blend 混合模式
         * @param color 混合颜色
         * @param texLeft 图像距离纹理左边的距离
         * @param texTop 图像具体纹理顶边的距离
         * @param texWidth 图像宽度
         * @param texHeight 图像高度
         * @param scale 缩放，仅控制图像的纵向缩放（我们假定曲光素材总是横向摆放的）
         */
        LSTG_METHOD()
        void Render(LuaStack& stack, const char* texture, const char* blend, LSTGColor* color, double texLeft, double texTop,
            double texWidth, double texHeight, std::optional<double> scale /* =1 */) const;

        /**
         * 碰撞检查
         * @param x 被检查对象坐标X
         * @param y 被检查对象坐标Y
         * @param rot 对象旋转
         * @param a 对象碰撞盒长轴/长
         * @param b 对象碰撞盒短轴/宽
         * @param rect 是否是矩形碰撞
         * @return 是否发生碰撞
         */
        LSTG_METHOD()
        bool CollisionCheck(double x, double y, std::optional<double> rot /* =0 */, std::optional<double> a /* =0 */,
            std::optional<double> b /* =0 */, std::optional<bool> rect /* =false */) const;

        /**
         * 检查曲线激光是否还在范围内
         * @return true，还在边界内，返回 false 提示整体已经越界
         */
        LSTG_METHOD()
        bool BoundCheck() const;

        /**
         * 对象转字符串表示
         */
        LSTG_METHOD(__tostring)
        std::string ToString() const;

    private:
        GamePlay::BentLaser m_stImplementation;
    };
}
