/**
 * @file
 * @author 9chu
 * @date 2022/4/18
 * 这个文件是 LuaSTGPlus 项目的一部分，请在项目所定义之授权许可范围内合规使用。
 */
#pragma once
#include <lstg/Core/Subsystem/Script/LuaRead.hpp>
#include <lstg/Core/Subsystem/Script/AutoBridgeHint.hpp>
#include "LSTGColor.hpp"

namespace lstg::v2::Bridge
{
    /**
     * 渲染模块
     */
    LSTG_MODULE(lstg, GLOBAL)
    class RenderModule
    {
    public:
        using LuaStack = Subsystem::Script::LuaStack;
        using AbsIndex = LuaStack::AbsIndex;
        template <typename... TArgs>
        using Unpack = Subsystem::Script::Unpack<TArgs...>;

    public:
        /**
         * 文字对齐方式
         */
        LSTG_ENUM()
        enum class TextAlignment
        {
            LSTG_FIELD(LEFT_TOP)
            LeftTop = 0,
            LSTG_FIELD(CENTER_TOP)
            CenterTop = 1,
            LSTG_FIELD(RIGHT_TOP)
            RightTop = 2,
            LSTG_FIELD(LEFT_MIDDLE)
            LeftMiddle = 4,
            LSTG_FIELD(CENTER_MIDDLE)
            CenterMiddle = 5,
            LSTG_FIELD(RIGHT_MIDDLE)
            RightMiddle = 6,
            LSTG_FIELD(LEFT_BOTTOM)
            LeftBottom = 8,
            LSTG_FIELD(CENTER_BOTTOM)
            CenterBottom = 9,
            LSTG_FIELD(RIGHT_BOTTOM)
            RightBottom = 10,
        };

    public:
        /**
         * 开始场景
         */
        LSTG_METHOD()
        static void BeginScene();

        /**
         * 结束场景
         */
        LSTG_METHOD()
        static void EndScene();

        /**
         * 清空屏幕和深度缓冲
         * @param color 颜色值
         */
        LSTG_METHOD()
        static void RenderClear(LSTGColor* color);

        /**
         * 设置视口
         * @param left 距离屏幕左边的距离
         * @param right 距离屏幕右边的距离
         * @param bottom 距离屏幕底边的距离
         * @param top 距离屏幕顶边的距离
         */
        LSTG_METHOD()
        static void SetViewport(double left, double right, double bottom, double top);

        /**
         * 设置正投影矩阵
         * @note nearPlane = 0, farPlane = 1
         * @param left X轴最小值
         * @param right X轴最大值
         * @param bottom Y轴最小值
         * @param top Y轴最大值
         */
        LSTG_METHOD()
        static void SetOrtho(double left, double right, double bottom, double top);

        /**
         * 设置透视投影矩阵和观察矩阵
         * @param eyeX 观察者X
         * @param eyeY 观察者Y
         * @param eyeZ 观察者Z
         * @param atX 观察目标X
         * @param atY 观察目标Y
         * @param atZ 观察目标Z
         * @param upX 观察者上方X
         * @param upY 观察者上方Y
         * @param upZ 观察者上方Z
         * @param fovy 视角范围（弧度制）
         * @param aspect 宽高比
         * @param zn Z轴近裁剪面
         * @param zf Z轴远裁剪面
         */
        LSTG_METHOD()
        static void SetPerspective(double eyeX, double eyeY, double eyeZ, double atX, double atY, double atZ, double upX, double upY,
            double upZ, double fovy, double aspect, double zn, double zf);

        /**
         * 设置雾
         * 如果不提供参数则关闭雾，否则依据 near 和 far 的取值设置线性雾或者指数雾
         * @param near 最近平面
         * @param far 最远平面
         * @param color 雾颜色
         */
        LSTG_METHOD()
        static void SetFog(std::optional<double> near, std::optional<double> far, std::optional<LSTGColor*> color /* =0xFFFFFF00 (RGBA) */);

        /**
         * 渲染精灵
         * @param stack 栈
         * @param imageName 图像名称
         * @param x 坐标X
         * @param y 坐标Y
         * @param rot 旋转
         * @param hscale 横向缩放
         * @param vscale 纵向缩放，若指定了 hscale 但是没有指定 vscale，则 vscale = hscale
         * @param z Z轴
         */
        LSTG_METHOD()
        static void Render(LuaStack& stack, const char* imageName, double x, double y, std::optional<double> rot /* =0 */,
            std::optional<double> hscale /* =1 */, std::optional<double> vscale /* =1 */, std::optional<double> z /* =0.5 */);

        /**
         * 渲染精灵
         * @param stack 栈
         * @note z = 0.5
         * @param imageName 图像名称
         * @param left 距离屏幕左边的距离
         * @param right 距离屏幕右边的距离
         * @param bottom 距离屏幕底边的距离
         * @param top 距离屏幕顶边的距离
         */
        LSTG_METHOD()
        static void RenderRect(LuaStack& stack, const char* imageName, double left, double right, double bottom, double top);

        /**
         * 渲染精灵
         * @param stack 栈
         * @param imageName 图像名称
         * @param x1 顶点1 X坐标
         * @param y1 顶点1 Y坐标
         * @param z1 顶点1 Z坐标
         * @param x2 顶点2 X坐标
         * @param y2 顶点2 Y坐标
         * @param z2 顶点2 Z坐标
         * @param x3 顶点3 X坐标
         * @param y3 顶点3 Y坐标
         * @param z3 顶点3 Z坐标
         * @param x4 顶点4 X坐标
         * @param y4 顶点4 Y坐标
         * @param z4 顶点4 Z坐标
         */
        LSTG_METHOD(Render4V)
        static void RenderVertex(LuaStack& stack, const char* imageName, double x1, double y1, double z1, double x2, double y2, double z2,
            double x3, double y3, double z3, double x4, double y4, double z4);

        /**
         * 直接渲染纹理
         * @param stack Lua栈
         * @param textureName 纹理资源名称
         * @param blend 混合模式
         * @param vertex1 顶点坐标1，指向 table: [ number, number, number, number, number, LSTGColor | number ]，表明 X, Y, Z, U, V, Color，下同
         * @param vertex2 顶点坐标2
         * @param vertex3 顶点坐标3
         * @param vertex4 顶点坐标4
         */
        LSTG_METHOD()
        static void RenderTexture(LuaStack& stack, const char* textureName, const char* blend, AbsIndex vertex1,
            AbsIndex vertex2, AbsIndex vertex3, AbsIndex vertex4);

        /**
         * 使用纹理字体渲染文本
         * @param name 纹理字体名称
         * @param text 字符串
         * @param x 坐标X
         * @param y 坐标Y
         * @param scale 缩放
         * @param align 对齐
         */
        LSTG_METHOD()
        static void RenderText(LuaStack& stack, const char* name, const char* text, double x, double y,
            std::optional<double> scale /* =1 */, std::optional<TextAlignment> align /* =5 */);

        /**
         * 渲染 TTF 字体
         * @param name 字体资源名
         * @param text 被渲染的文字
         * @param left 左边距离
         * @param right 右边距离
         * @param bottom 底边距离
         * @param top 顶边距离
         * @param fmt 格式
         * @param blend 混合颜色
         */
        LSTG_METHOD(RenderTTF)
        static void RenderTrueTypeFont(LuaStack& stack, const char* name, const char* text, double left, double right, double bottom,
            double top, int32_t fmt, LSTGColor* blend);

        /**
         * 推入RT到堆栈
         * @param name RT纹理资源名
         */
        LSTG_METHOD()
        static void PushRenderTarget(LuaStack& stack, const char* name);

        /**
         * 将RT从栈移除
         */
        LSTG_METHOD()
        static void PopRenderTarget(LuaStack& stack);

        /**
         * 执行后处理效果
         * @param stack Lua栈
         * @param name 被执行后处理的纹理资源名，总是会被填入 MainTexture 纹理变量
         * @param fx 效果资源名
         * @param blend 混合模式
         * @param args 参数表 table: Record<string, number|LSTGColor|string>
         */
        static void PostEffect(LuaStack& stack, const char* name, const char* fx, const char* blend, std::optional<AbsIndex> args);

        /**
         * 开始捕获后处理特效
         * 等价于：
         *   PushRenderTarget(InternalPostEffectBuffer)
         */
        LSTG_METHOD()
        static void PostEffectCapture(LuaStack& stack);

        /**
         * 结束捕获并施加后处理特效
         * 等价于：
         *   PopRenderTarget(InternalPostEffectBuffer)
         *   PostEffect(InternalPostEffectBuffer, fx, blend, args)
         * 必须配对 PostEffectCapture 使用。
         * @param stack Lua栈
         * @param fx 特效资源名
         * @param blend 混合模式
         * @param args 参数表 table: Record<string, number|LSTGColor|string>
         */
        LSTG_METHOD()
        static void PostEffectApply(LuaStack& stack, const char* fx, const char* blend, std::optional<AbsIndex> args);
    };
}
