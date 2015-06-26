#pragma once
#include "Global.h"
#include "ObjectPool.hpp"

namespace LuaSTGPlus
{
	/// @brief ��Ϸ����״̬
	enum GAMEOBJECTSTATUS
	{
		STATUS_FREE = 0,  // ����״̬�����ڱ�ʶ����αͷ��
		STATUS_DEFAULT,  // ����״̬
		STATUS_KILL,  // ��kill�¼�����
		STATUS_DEL  // ��del�¼�����
	};

	/// @brief ��Ϸ����
	struct GameObject
	{
		GAMEOBJECTSTATUS status;  // (���ɼ�)����״̬
		size_t id;  // (���ɼ�)�����ڶ�����е�id
		int64_t uid;  // (���ɼ�)����Ψһid

		lua_Number x, y;  // ��������
		lua_Number lastx, lasty;  // (���ɼ�)��һ֡��������
		lua_Number dx, dy;  // (ֻ��)��һ֡��������������������ƫ����
		lua_Number rot, omiga;  // ��ת�Ƕ���Ƕ�����
		lua_Number vx, vy;  // �ٶ�
		lua_Number layer;  // ͼ��
		lua_Number a, b;  // ��λ�ĺ���������ײ��С��һ��
		lua_Number hscale, vscale;  // �������������ʣ���Ӱ����Ⱦ

		bool colli;  // �Ƿ������ײ
		bool rect;  // �Ƿ�Ϊ������ײ��
		bool bound;  // �Ƿ�Խ�����
		bool hide;  // �Ƿ�����
		bool navi;  // �Ƿ��Զ�ת��
		
		lua_Integer group;  // �������ڵ���ײ��
		lua_Integer timer, ani_timer;  // ������

		// ������
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

	/// @brief ��Ϸ�����
	class GameObjectPool
	{
	private:
		lua_State* L = nullptr;
		FixedObjectPool<GameObject, LGOBJ_MAXCNT> m_ObjectPool;

		// ����αͷ��
		uint64_t m_iUid = 0;
		GameObject m_pObjectListHeader, m_pObjectListTail;
		GameObject m_pRenderListHeader, m_pRenderListTail;
		GameObject m_pCollisionListHeader[LGOBJ_GROUPCNT], m_pCollisionListTail[LGOBJ_GROUPCNT];

		// �����߽�
		lua_Number m_BoundLeft = -100.f;
		lua_Number m_BoundRight = 100.f;
		lua_Number m_BoundTop = 100.f;
		lua_Number m_BoundBottom = -100.f;
	private:
		bool collisionCheck(GameObject* p1, GameObject* p2)LNOEXCEPT;
		GameObject* freeObject(GameObject* p)LNOEXCEPT;
	public:
		/// @brief ����Ƿ�Ϊ���߳�
		bool CheckIsMainThread(lua_State* pL)LNOEXCEPT { return pL == L; }

		/// @brief ��ȡ�ѷ����������
		size_t GetObjectCount()LNOEXCEPT { return m_ObjectPool.Size(); }
		
		/// @brief ִ�ж����Frame����
		void DoFrame()LNOEXCEPT;

		/// @brief ִ�ж����Render����
		void DoRender()LNOEXCEPT;

		/// @brief ������̨�߽�
		void SetBound(lua_Number l, lua_Number r, lua_Number b, lua_Number t)LNOEXCEPT
		{
			m_BoundLeft = l;
			m_BoundRight = r;
			m_BoundTop = t;
			m_BoundBottom = b;
		}

		/// @brief ִ�б߽���
		void BoundCheck()LNOEXCEPT;

		/// @brief ��ײ���
		/// @param[in] groupA ������A
		/// @param[in] groupB ������B
		void CollisionCheck(size_t groupA, size_t groupB)LNOEXCEPT;

		/// @brief ���¶����XY����ƫ����
		void UpdateXY()LNOEXCEPT;

		/// @brief ֡ĩ���º���
		void AfterFrame()LNOEXCEPT;

		/// @brief �����¶���
		int New(lua_State* L)LNOEXCEPT;

		/// @brief ֪ͨ����ɾ��
		int Del(lua_State* L)LNOEXCEPT;
		
		/// @brief ֪ͨ��������
		int Kill(lua_State* L)LNOEXCEPT;

		/// @brief �������Ƿ���Ч
		int IsValid(lua_State* L)LNOEXCEPT;
		
		/// @brief ��н�
		bool Angle(size_t idA, size_t idB, double& out)LNOEXCEPT;

		/// @brief �����
		bool Dist(size_t idA, size_t idB, double& out)LNOEXCEPT;

		/// @brief �����ٶȷ���ʹ�С
		bool SetV(size_t id, double v, double a, bool updateRot)LNOEXCEPT;

		/// @brief ��Χ���
		bool BoxCheck(size_t id, double left, double right, double top, double bottom, bool& ret)LNOEXCEPT;
		
		/// @brief ��ն����
		void ResetPool()LNOEXCEPT;

		/// @brief ִ��Ĭ����Ⱦ
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
