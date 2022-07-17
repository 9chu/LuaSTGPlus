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
#include "../AssetNaming.hpp"

namespace lstg::v2::Bridge
{
    /**
     * 资源管理模块
     */
    LSTG_MODULE(lstg, GLOBAL)
    class AssetManagerModule
    {
    public:
        using LuaStack = Subsystem::Script::LuaStack;
        using AbsIndex = LuaStack::AbsIndex;
        template <typename... TArgs>
        using Unpack = Subsystem::Script::Unpack<TArgs...>;

    public:
#if LSTG_AUTO_BRIDGE_HINT
        /**
         * 资产类型
         */
        LSTG_ENUM()
        enum class AssetTypes
        {
            LSTG_FIELD()
            Texture = 1,
            LSTG_FIELD()
            Image = 2,
            LSTG_FIELD()
            Animation = 3,
            LSTG_FIELD()
            Music = 4,
            LSTG_FIELD()
            Sound = 5,
            LSTG_FIELD()
            Particle = 6,
            LSTG_FIELD()
            TexturedFont = 7,
            LSTG_FIELD()
            TrueTypeFont = 8,
            LSTG_FIELD()
            Effect = 9,
        };
#else
        using AssetTypes = v2::AssetTypes;
#endif

    public:
        /**
         * 设置资源状态
         * @param pool 池子类型，可取 global、stage 或者 none
         */
        LSTG_METHOD()
        static void SetResourceStatus(LuaStack& stack, const char* pool);

        /**
         * 删除资产
         * 如果资产正在使用，则延迟释放直到其不被引用。
         * @param pool 资源池，可选 global 或者 stage
         * @param type 资产类型，若不指定，则删除整个池子
         * @param name 资产名称
         */
        LSTG_METHOD()
        static void RemoveResource(LuaStack& stack, const char* pool, std::optional<AssetTypes> type, std::optional<const char*> name);

        /**
         * 检查资产位于哪个池子
         * 资源查找按照顺序进行：stage -> global
         * @param type 资产类型
         * @param name 名称
         * @return 若不存在，则返回 nil，否则返回 stage 或 global
         */
        LSTG_METHOD()
        static std::optional<const char*> CheckRes(AssetTypes type, const char* name);

        /**
         * 枚举资源池中某种类型的资产
         * @param type 资产类型
         * @return 全局资源池资产名称, 关卡资源池资产名称: string[], string[]
         */
        LSTG_METHOD()
        static Unpack<AbsIndex, AbsIndex> EnumRes(LuaStack& stack, AssetTypes type);

        /**
         * 获取纹理的宽和高
         * @param name 纹理资产名
         * @return 宽和高
         */
        LSTG_METHOD()
        static Unpack<double, double> GetTextureSize(LuaStack& stack, const char* name);

        /**
         * 加载纹理
         * @param name 纹理资产名
         * @param path 资源路径
         * @param mipmap 是否生成 mipmap
         */
        LSTG_METHOD()
        static void LoadTexture(LuaStack& stack, const char* name, const char* path, std::optional<bool> mipmap /* = false */);

        /**
         * 从纹理创建图像
         * @param name 图像资产名
         * @param textureName 纹理资产名
         * @param x 相对于纹理左上角坐标X
         * @param y 相对于纹理左上角坐标Y
         * @param w 宽度
         * @param h 高度
         * @param a 横向碰撞大小
         * @param b 纵向碰撞大小
         * @param rect 是否为矩形判定
         */
        LSTG_METHOD()
        static void LoadImage(LuaStack& stack, const char* name, const char* textureName, double x, double y, double w, double h,
            std::optional<double> a, std::optional<double> b, std::optional<bool> rect);

        /**
         * 设置图像状态
         * @param name 图像资源名称
         * @param blend 混合模式，由分量 A+B 构成，A可选mul、add，B可选alpha add sub rev
         * @param vertexColor1 顶点1颜色
         * @param vertexColor2 顶点2颜色
         * @param vertexColor3 顶点3颜色
         * @param vertexColor4 顶点4颜色
         */
        LSTG_METHOD()
        static void SetImageState(LuaStack& stack, const char* name, const char* blend, std::optional<LSTGColor*> vertexColor1,
            std::optional<LSTGColor*> vertexColor2, std::optional<LSTGColor*> vertexColor3, std::optional<LSTGColor*> vertexColor4);

        /**
         * 设置图像中心
         * @param name 图像资源名称
         * @param x 坐标X
         * @param y 坐标Y
         */
        LSTG_METHOD()
        static void SetImageCenter(LuaStack& stack, const char* name, double x, double y);

        /**
         * 设置全局图像缩放
         * @deprecated 该方法已弃用
         * @param factor 缩放量
         */
        LSTG_METHOD()
        static void SetImageScale(double factor);

        /**
         * 装载动画
         * @note 动画总是循环播放的
         * @param name 动画资产名称
         * @param textureName 纹理资产名称
         * @param x 第一帧距离纹理左边的距离
         * @param y 第一帧距离纹理顶边的距离
         * @param w 整个动画序列的宽度
         * @param h 整个动画序列的高度
         * @param n 纵向分割数
         * @param m 横向分割数，列优先
         * @param interval 帧间隔
         * @param a 碰撞盒宽度
         * @param b 碰撞盒高度
         * @param rect 是否是矩形碰撞盒
         */
        LSTG_METHOD()
        static void LoadAnimation(LuaStack& stack, const char* name, const char* textureName, double x, double y, double w, double h,
            int32_t n, int32_t m, int32_t interval, std::optional<double> a, std::optional<double> b, std::optional<bool> rect);

        /**
         * 设置动画资产绘制状态
         * @param name 动画资产名称
         * @param blend 混合模式
         * @param vertexColor1 顶点1颜色
         * @param vertexColor2 顶点2颜色
         * @param vertexColor3 顶点3颜色
         * @param vertexColor4 顶点4颜色
         */
        LSTG_METHOD()
        static void SetAnimationState(LuaStack& stack, const char* name, const char* blend, std::optional<LSTGColor*> vertexColor1,
            std::optional<LSTGColor*> vertexColor2, std::optional<LSTGColor*> vertexColor3, std::optional<LSTGColor*> vertexColor4);

        /**
         * 设置动画中心
         * @param name 动画资产名称
         * @param x 中心坐标X
         * @param y 中心坐标Y
         */
        LSTG_METHOD()
        static void SetAnimationCenter(LuaStack& stack, const char* name, double x, double y);

        /**
         * 装载粒子发射器
         * @note 符合 HGE 所定义的粒子文件
         * @param name 资产名称
         * @param path 定义路径
         * @param imgName 图像资产名称
         * @param a 碰撞盒宽度
         * @param b 碰撞盒高度
         * @param rect 是否是矩形碰撞盒
         */
        LSTG_METHOD(LoadPS)
        static void LoadParticle(LuaStack& stack, const char* name, const char* path, const char* imgName, std::optional<double> a,
            std::optional<double> b, std::optional<bool> rect);

        /**
         * 装载纹理字体
         * @param name 资产名称
         * @param path 定义文件路径
         * @param textureNamePath 绑定的纹理资源路径，用于 f2d 格式的纹理字体。对于 HGE 字体，在同级目录中寻找纹理，无需指定该参数。
         * @param mipmap 是否生成 mipmap
         */
        LSTG_METHOD(LoadFont)
        static void LoadTexturedFont(LuaStack& stack, const char* name, const char* path, std::optional<bool> mipmap /* =true */);

        /**
         * 设置字体颜色
         * @param name 纹理字体资产名称
         * @param blend 混合模式
         * @param color 混合颜色
         */
        LSTG_METHOD(SetFontState)
        static void SetTexturedFontState(LuaStack& stack, const char* name, const char* blend, std::optional<LSTGColor*> color);

        /**
         * 设置 HGE 字体状态
         * @deprecated 已弃用方法
         */
        LSTG_METHOD(SetFontState2)
        static void SetTexturedFontState2();

        /**
         * 加载 TTF 字体
         * @param name 字体资产名称
         * @param path 路径
         * @param width 字形宽度
         * @param height 字形高度，建议设置同 width 相同的值
         */
        LSTG_METHOD(LoadTTF)
        static void LoadTrueTypeFont(LuaStack& stack, const char* name, const char* path, double width, std::optional<double> height);

        /**
         * 注册 TTF 字体
         * @deprecated 已弃用方法
         */
        LSTG_METHOD(RegTTF)
        static void RegTTF();

        /**
         * 装载音效
         * @note 音效总是被整体装载进入内存
         * @param name 资产名称
         * @param path 路径
         */
        LSTG_METHOD()
        static void LoadSound(const char* name, const char* path);

        /**
         * 装载音乐
         * @note 音乐总是流式装载
         * @param name 资产名称
         * @param path 路径
         * @param end 循环节终止时间（秒）
         * @param loop 循环节持续时间（秒）
         */
        LSTG_METHOD()
        static void LoadMusic(const char* name, const char* path, double end, double loop);

        /**
         * 装载Shader特效
         * @param name 资产名称
         * @param path 路径
         */
        LSTG_METHOD()
        static void LoadFX(LuaStack& stack, const char* name, const char* path);

        /**
         * 创建RT
         * @param name 纹理资产名
         */
        LSTG_METHOD()
        static void CreateRenderTarget(LuaStack& stack, const char* name);

        /**
         * 检查一个纹理是否为RT
         * @param name 被检查纹理名
         */
        LSTG_METHOD()
        static bool IsRenderTarget(LuaStack& stack, const char* name);
    };
}
