#pragma once
#include "Global.h"
#include "ObjectPool.hpp"

namespace LuaSTGPlus
{
	/// @brief 游戏对象状态
	enum GAMEOBJECTSTATUS
	{
		STATUS_FREE = 0,  // 空闲状态、用于标识链表伪头部
		STATUS_DEFAULT,  // 正常状态
		STATUS_KILL,  // 被kill事件触发
		STATUS_DEL  // 被del事件触发
	};

	/// @brief 游戏对象
	struct GameObject
	{
		GAMEOBJECTSTATUS status;  // (不可见)对象状态
		size_t id;  // (不可见)对象在对象池中的id
		int64_t uid;  // (不可见)对象唯一id

		lua_Number x, y;  // 中心坐标
		lua_Number lastx, lasty;  // (不可见)上一帧中心坐标
		lua_Number dx, dy;  // (只读)上一帧中心坐标相对中心坐标的偏移量
		lua_Number rot, omiga;  // 旋转角度与角度增量
		lua_Number vx, vy;  // 速度
		lua_Number layer;  // 图层
		lua_Number a, b;  // 单位的横向、纵向碰撞大小的一半
		lua_Number hscale, vscale;  // 横向、纵向拉伸率，仅影响渲染

		bool colli;  // 是否参与碰撞
		bool rect;  // 是否为矩形碰撞盒
		bool bound;  // 是否越界清除
		bool hide;  // 是否隐藏
		bool navi;  // 是否自动转向
		
		lua_Integer group;  // 对象所在的碰撞组
		lua_Integer timer, ani_timer;  // 计数器

		// 链表域
		GameObject *pObjectPrev, *pObjectNext;
		GameObject *pRenderPrev, *pRenderNext;
		GameObject *pCollisionPrev, *pCollisionNext;
		
		void Reset()
		{
			status = STATUS_FREE;
			id = (size_t)-1;
			uid = 0;

			x = y = 0.;
			lastx = lasty = 0.;
			dx = dy = 0.;
			rot = omiga = 0.;
			vx = vy = 0.;
			layer = 0.;
			a = b = 0.;
			hscale = vscale = 1.;

			colli = bound = true;
			rect = hide = navi = false;

			group = LGOBJ_DEFAULTGROUP;
			timer = ani_timer = 0;

			pObjectPrev = pObjectNext = nullptr;
			pRenderPrev = pRenderNext = nullptr;
			pCollisionPrev = pCollisionNext = nullptr;
		}
	};

	/// @brief 游戏对象池
	class GameObjectPool
	{
	private:
		lua_State* L = nullptr;
		FixedObjectPool<GameObject, LGOBJ_MAXCNT> m_ObjectPool;

		// 链表伪头部
		uint64_t m_iUid = 0;
		GameObject m_pObjectListHeader, m_pObjectListTail;
		GameObject m_pRenderListHeader, m_pRenderListTail;
		GameObject m_pCollisionListHeader[LGOBJ_GROUPCNT], m_pCollisionListTail[LGOBJ_GROUPCNT];

		// 场景边界
		lua_Number m_BoundLeft = -100.f;
		lua_Number m_BoundRight = 100.f;
		lua_Number m_BoundTop = 100.f;
		lua_Number m_BoundBottom = -100.f;
	private:
		bool collisionCheck(GameObject* p1, GameObject* p2)LNOEXCEPT;
		GameObject* freeObject(GameObject* p)LNOEXCEPT;
	public:
		/// @brief 检查是否为主线程
		bool CheckIsMainThread(lua_State* pL)LNOEXCEPT { return pL == L; }

		/// @brief 获取已分配对象数量
		size_t GetObjectCount()LNOEXCEPT { return m_ObjectPool.Size(); }
		
		/// @brief 执行对象的Frame函数
		void DoFrame()LNOEXCEPT;

		/// @brief 执行对象的Render函数
		void DoRender()LNOEXCEPT;

		/// @brief 设置舞台边界
		void SetBound(lua_Number l, lua_Number r, lua_Number b, lua_Number t)LNOEXCEPT
		{
			m_BoundLeft = l;
			m_BoundRight = r;
			m_BoundTop = t;
			m_BoundBottom = b;
		}

		/// @brief 执行边界检查
		void BoundCheck()LNOEXCEPT;

		/// @brief 碰撞检查
		/// @param[in] groupA 对象组A
		/// @param[in] groupB 对象组B
		void CollisionCheck(size_t groupA, size_t groupB)LNOEXCEPT;

		/// @brief 更新对象的XY坐标偏移量
		void UpdateXY()LNOEXCEPT;

		/// @brief 帧末更新函数
		void AfterFrame()LNOEXCEPT;

		/// @brief 创建新对象
		int New(lua_State* L)LNOEXCEPT;

		/// @brief 通知对象删除
		int Del(lua_State* L)LNOEXCEPT;
		
		/// @brief 通知对象消亡
		int Kill(lua_State* L)LNOEXCEPT;

		/// @brief 检查对象是否有效
		int IsValid(lua_State* L)LNOEXCEPT;
		
		/// @brief 求夹角
		bool Angle(size_t idA, size_t idB, double& out)LNOEXCEPT;

		/// @brief 求距离
		bool Dist(size_t idA, size_t idB, double& out)LNOEXCEPT;

		/// @brief 设置速度方向和大小
		bool SetV(size_t id, double v, double a, bool updateRot)LNOEXCEPT;

		/// @brief 范围检查
		bool BoxCheck(size_t id, double left, double right, double top, double bottom, bool& ret)LNOEXCEPT;
		
		/// @brief 清空对象池
		void ResetPool()LNOEXCEPT;

		/// @brief 执行默认渲染
		bool DoDefauleRender(size_t id)LNOEXCEPT;

		/*
			static int ResetPool(lua_State* L)LNOEXCEPT
		{
			return 0;
		}
			static int DefaultRenderFunc(lua_State* L)LNOEXCEPT
		{
			return 0;
		}
			static int NextObject(lua_State* L)LNOEXCEPT
		{
			return 0;
		}
			static int ObjList(
			*/
	private:
		GameObjectPool& operator=(const GameObjectPool&);
		GameObjectPool(const GameObjectPool&);
	public:
		GameObjectPool(lua_State* pL);
	};
}
