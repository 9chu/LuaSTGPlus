/**
 * @file
 * @author 9chu
 * @date 2022/4/17
 * 此文件为 LuaSTGPlus 项目的一部分，版权与许可声明详见 COPYRIGHT.txt。
 */
#pragma once
#include "LSTGColor.hpp"
#include "LSTGRandomizer.hpp"
#include "LSTGBentLaserData.hpp"

namespace lstg::v2::Bridge
{
    /**
     * 游戏对象模块
     */
    LSTG_MODULE(lstg, GLOBAL)
    class GameObjectModule
    {
    public:
        using LuaStack = Subsystem::Script::LuaStack;
        using AbsIndex = LuaStack::AbsIndex;
        template <typename... TArgs>
        using Unpack = Subsystem::Script::Unpack<TArgs...>;

    public:
        /**
         * 获取对象表
         * @warning 慎用
         */
        LSTG_METHOD(ObjTable)
        static void GetObjectTable();

        /**
         * 获取对象池中对象个数
         */
        LSTG_METHOD(GetnObj)
        static int32_t GetObjectCount();

        /**
         * 更新对象池
         * 此时将所有对象排序并归类
         * 排序规则：id 越小越靠前
         * @deprecated 已弃用方法
         */
        LSTG_METHOD(UpdateObjList)
        static void UpdateObjectList();

        /**
         * 更新所有对象
         * 按照下列顺序更新这些属性：
         *  vx += ax
         *  vy += ay
         *  x += vx
         *  y += vy
         *  rot += omiga
         *  更新绑定的粒子系统（若有）
         * @warning 只能在Lua主线程调用
         */
        LSTG_METHOD(ObjFrame)
        static void UpdateObjects(LuaStack& stack);

        /**
         * 渲染所有对象
         * layer 小的对象优先渲染，相同 layer 按照 id 从小到大渲染。
         * @warning 只能在Lua主线程调用
         */
        LSTG_METHOD(ObjRender)
        static void RenderObjects(LuaStack& stack);

        /**
         * 设置边界
         * @param left 到左边距离
         * @param right 到右边距离
         * @param bottom 到底边距离
         * @param top 到顶边距离
         */
        LSTG_METHOD()
        static void SetBound(double left, double right, double bottom, double top);

        /**
         * 执行边界检查
         * @note BoundCheck只保证对象中心还在范围内，不进行碰撞盒检查
         * @warning 只能在Lua主线程调用
         */
        LSTG_METHOD()
        static void BoundCheck(LuaStack& stack);

        /**
         * 对组 A 和组 B 进行碰撞检测
         * 如果组A中对象与组B中对象发生碰撞，将执行A中对象的碰撞回调函数
         * @warning 只能在Lua主线程调用
         * @param groupIdA 组 A
         * @param groupIdB 组 B
         */
        LSTG_METHOD()
        static void CollisionCheck(LuaStack& stack, int32_t groupIdA, int32_t groupIdB);

        /**
         * 刷新对象的坐标
         * 更新下述属性：
         *  dx
         *  dy
         *  lastx
         *  lasty
         *  rot（若navi=true）
         * @warning 只能在Lua主线程调用
         */
        LSTG_METHOD()
        static void UpdateXY(LuaStack& stack);

        /**
         * 刷新对象的timer和ani_timer
         * 若对象被标记为del或kill将删除对象并回收资源
         * @note 对象只有在AfterFrame调用后才会被清理，在此之前可以通过设置对象的status字段取消删除标记
         * @warning 只能在Lua主线程调用
         */
        LSTG_METHOD()
        static void AfterFrame(LuaStack& stack);

        /**
         * 创建新对象
         *  interface Class {
         *    [1] = (Object, ...) -> void,  // OnInit
         *    [2] = (Object, ...) -> void,  // OnDel
         *    [3] = (Object) -> void,  // OnUpdate
         *    [4] = (Object) -> void,  // OnRender
         *    [5] = (Object, Object) -> void,  // OnCollide
         *    [6] = (Object, ...) -> void,  // OnKill
         *  }
         *  interface Object {
         *    x: number, y: number,  // 坐标
         *    dx: number, dy: number,  // 距离上一帧的偏移
         *    rot: number,  // 旋转
         *    omiga: number,  // 角度增量，废弃，应该使用omega
         *    omega: number,  // 角度增量
         *    timer: number,  // 计时器
         *    vx: number, vy: number,  // 速度
         *    ax: number, ay: number,  // 加速度
         *    layer: number,  // 渲染层级
         *    group: number,  // 碰撞组
         *    hide: boolean,  // 是否隐藏
         *    bound: boolean,  // 是否越界销毁
         *    navi: boolean,  // 是否自动更新朝向
         *    colli: boolean,  // 是否允许碰撞
         *    status: 'del' | 'kill' | 'normal',  // 对象状态
         *    hscale: number,  // 横向缩放
         *    vscale: number,  // 纵向缩放
         *    class: Class,  // 父类
         *    a: number,  // 碰撞盒横向宽度
         *    b: number,  // 碰撞盒纵向宽度
         *    rect: boolean,  // 是否OBB碰撞
         *    img: string,  // 引用的资产
         *    ani: integer,  // 动画计数器
         *  }
         * @note luastg+提供了至多32768个空间共object使用，超过这个大小后将报错
         * @param stack Lua栈
         * @param cls 类
         */
        LSTG_METHOD(New)
        static LuaStack::AbsIndex NewObject(LuaStack& stack, AbsIndex cls);

        /**
         * 通知删除一个对象
         * 将设置标志并调用回调函数
         * @param stack Lua栈
         * @param object 对象
         */
        LSTG_METHOD(Del)
        static void DelObject(LuaStack& stack, AbsIndex object);

        /**
         * 通知杀死一个对象
         * 将设置标志并调用回调函数
         * @param stack Lua栈
         * @param object 对象
         */
        LSTG_METHOD(Kill)
        static void KillObject(LuaStack& stack, AbsIndex object);

        /**
         * 检查对象是否有效
         * @param stack Lua栈
         * @param object 对象
         * @return 对象是否有效
         */
        LSTG_METHOD(IsValid)
        static bool IsObjectValid(LuaStack& stack, AbsIndex object);

        /**
         * 获取对象的速度
         * @param stack Lua栈
         * @param object 对象
         * @return 速度大小, 速度方向
         */
        LSTG_METHOD(GetV)
        static Unpack<double, double> GetObjectVelocity(LuaStack& stack, AbsIndex object);

        /**
         * 设置对象的速度
         * @param stack Lua栈
         * @param object 对象
         * @param velocity 速度
         * @param angle 角度
         * @param track 是否同时设置旋转
         */
        LSTG_METHOD(SetV)
        static void SetObjectVelocity(LuaStack& stack, AbsIndex object, double velocity, double angle, std::optional<bool> track);

        /**
         * 设置资源状态
         * @note 该函数将会设置和对象绑定的精灵、动画资源的混合模式，该设置对所有同名资源都有效果
         * @param stack Lua栈
         * @param object 对象
         * @param blend 混合模式
         * @param a 颜色A
         * @param r 颜色R
         * @param g 颜色G
         * @param b 颜色B
         */
        LSTG_METHOD(SetImgState)
        static void SetImageStateByObject(LuaStack& stack, AbsIndex object, const char* blend, int32_t a, int32_t r, int32_t g, int32_t b);

        /**
         * 计算角度
         * 若a,b为对象，则求向量(对象b.中心 - 对象a.中心)相对x轴正方向的夹角
         * 否则计算tan2(d-b, c-a)
         * @param stack Lua栈
         * @param a Object或数值
         * @param b Object或数值
         * @param c 数值（当A和B为数值时）
         * @param d 数值（当A和B为数值时）
         * @return 角度
         */
        LSTG_METHOD(Angle)
        static double CalculateAngle(LuaStack& stack, std::variant<double, AbsIndex> a, std::variant<double, AbsIndex> b,
            std::optional<double> c, std::optional<double> d);

        /**
         * 计算距离
         * 若a与b为对象则计算a与b之间的距离
         * 否则计算向量(c,d)与(a,b)之间的距离
         * @param stack Lua栈
         * @param a Object或数值
         * @param b Object或数值
         * @param c 数值（当A和B为数值时）
         * @param d 数值（当A和B为数值时）
         * @return 距离
         */
        LSTG_METHOD(Dist)
        static double CalculateDistance(LuaStack& stack, std::variant<double, AbsIndex> a, std::variant<double, AbsIndex> b,
            std::optional<double> c, std::optional<double> d);

        /**
         * 检查对象中心是否在所给范围内
         * @param stack Lua栈
         * @param object 对象
         * @param left 左边
         * @param right 右边
         * @param bottom 底边
         * @param top 顶边
         * @return 是否在范围内
         */
        LSTG_METHOD()
        static bool BoxCheck(LuaStack& stack, AbsIndex object, double left, double right, double bottom, double top);

        /**
         * 清空并回收所有对象
         */
        LSTG_METHOD()
        static void ResetPool();

        /**
         * 在对象上调用默认渲染方法
         * @param stack Lua栈
         * @param object 对象
         */
        LSTG_METHOD()
        static void DefaultRenderFunc(LuaStack& stack, AbsIndex object);

        /**
         * 获取组中的下一个元素
         * @param groupId 组ID, 若groupid为无效的碰撞组则返回所有对象
         * @param id 元素ID
         * @return 返回的第一个参数为id（luastg中为idx），第二个参数为对象
         */
        LSTG_METHOD()
        static int NextObject(lua_State* L);

        /**
         * 产生组遍历迭代器
         * @param groupId 组ID
         * @return NextObject, groupId, id
         */
        LSTG_METHOD(ObjList)
        static Unpack<AbsIndex, int32_t, double> EnumerateObjectList(LuaStack& stack, int32_t groupId);

        /**
         * __index 方法
         * @param L 栈
         */
        LSTG_METHOD(GetAttr)
        static int GetObjectAttribute(lua_State* L);

        /**
         * __newindex 方法
         * @param L 栈
         */
        LSTG_METHOD(SetAttr)
        static int SetObjectAttribute(lua_State* L);

        /**
         * 启动绑定在对象上的粒子发射器
         * @param stack Lua栈
         * @param object 对象
         */
        LSTG_METHOD()
        static void ParticleFire(LuaStack& stack, AbsIndex object);

        /**
         * 停止绑定在对象上的粒子发射器
         * @param stack Lua栈
         * @param object 对象
         */
        LSTG_METHOD()
        static void ParticleStop(LuaStack& stack, AbsIndex object);

        /**
         * 返回绑定在对象上的粒子发射器的存活粒子数
         * @param stack Lua栈
         * @param object 对象
         * @return 粒子个数
         */
        LSTG_METHOD(ParticleGetn)
        static int32_t ParticleGetCount(LuaStack& stack, AbsIndex object);

        /**
         * 获取绑定在对象上粒子发射器的发射密度
         * @note luastg/luastg+更新粒子发射器的时钟始终为1/60s
         * @param stack Lua栈
         * @param object 对象
         * @return 个/秒
         */
        LSTG_METHOD()
        static float ParticleGetEmission(LuaStack& stack, AbsIndex object);

        /**
         * 设置绑定在对象上粒子发射器的发射密度
         * @param stack Lua栈
         * @param object 对象
         * @param count 发射密度（个/秒）
         */
        LSTG_METHOD()
        static void ParticleSetEmission(LuaStack& stack, AbsIndex object, float count);
    };
}
