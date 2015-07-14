#pragma once
#include "Global.h"
#include "ObjectPool.hpp"
#include "CirularQueue.hpp"
#include "ResourceMgr.h"

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
		lua_Number ax, ay;  // ���ٶ�
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

		Resource* res;  // ��Ⱦ��Դ
		ResParticle::ParticlePool* ps;  // ����ϵͳ

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
			ax = ay = 0.;
			layer = 0.;
			a = b = 0.;
			hscale = vscale = 1.;

			colli = bound = true;
			rect = hide = navi = false;

			group = LGOBJ_DEFAULTGROUP;
			timer = ani_timer = 0;

			res = nullptr;
			ps = nullptr;

			pObjectPrev = pObjectNext = nullptr;
			pRenderPrev = pRenderNext = nullptr;
			pCollisionPrev = pCollisionNext = nullptr;
		}

		void ReleaseResource()
		{
			if (res)
			{
				if (res->GetType() == ResourceType::Particle)
				{
					LASSERT(ps);
					static_cast<ResParticle*>(res)->FreeInstance(ps);
					ps = nullptr;
				}
				res->Release();
				res = nullptr;
			}
		}

		bool ChangeResource(const char* res_name);
	};

	/// @brief ���߼����ػ�ʵ��
	class GameObjectBentLaser
	{
	public:
		static GameObjectBentLaser* AllocInstance();
		static void FreeInstance(GameObjectBentLaser* p);
	private:
		struct LaserNode
		{
			fcyVec2 pos;
			float half_width;
		};
	private:
		CirularQueue<LaserNode, LGOBJ_MAXLASERNODE> m_Queue;
		float m_fLength = 0.f;  // ��¼���ⳤ��
	public:
		bool Update(size_t id, int length, float width)LNOEXCEPT;
		void Release()LNOEXCEPT;
		bool Render(const char* tex_name, BlendMode blend, fcyColor c, float tex_left, float tex_top, float tex_width, float tex_height, float scale)LNOEXCEPT;
		bool CollisionCheck(float x, float y, float rot, float a, float b, bool rect)LNOEXCEPT;
		bool BoundCheck()LNOEXCEPT;
	protected:
		GameObjectBentLaser();
		~GameObjectBentLaser();
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
		GameObject* freeObject(GameObject* p)LNOEXCEPT;
	public:
		/// @brief ����Ƿ�Ϊ���߳�
		bool CheckIsMainThread(lua_State* pL)LNOEXCEPT { return pL == L; }

		/// @brief ��ȡ�ѷ����������
		size_t GetObjectCount()LNOEXCEPT { return m_ObjectPool.Size(); }
		
		/// @brief ��ȡ����
		GameObject* GetPooledObject(size_t i)LNOEXCEPT { return m_ObjectPool.Data(i); }

		/// @brief ִ�ж����Frame����
		void DoFrame()LNOEXCEPT;

		/// @brief ִ�ж����Render����
		void DoRender()LNOEXCEPT;

		/// @brief ��ȡ��̨�߽�
		fcyRect GetBound()LNOEXCEPT
		{
			return fcyRect((float)m_BoundLeft, (float)m_BoundTop, (float)m_BoundRight, (float)m_BoundBottom);
		}

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
		bool GetV(size_t id, double& v, double& a)LNOEXCEPT;

		/// @brief �����ٶȷ���ʹ�С
		bool SetV(size_t id, double v, double a, bool updateRot)LNOEXCEPT;

		/// @brief ����Ԫ�ص�ͼ��״̬
		bool SetImgState(size_t id, BlendMode m, fcyColor c)LNOEXCEPT;

		/// @brief ��Χ���
		bool BoxCheck(size_t id, double left, double right, double top, double bottom, bool& ret)LNOEXCEPT;
		
		/// @brief ��ն����
		void ResetPool()LNOEXCEPT;

		/// @brief ִ��Ĭ����Ⱦ
		bool DoDefaultRender(size_t id)LNOEXCEPT;

		/// @brief ��ȡ��һ��Ԫ�ص�ID
		/// @return ����-1��ʾ��Ԫ��
		int NextObject(int groupId, int id)LNOEXCEPT;

		int NextObject(lua_State* L)LNOEXCEPT;

		/// @brief ��ȡ�б��еĵ�һ��Ԫ��ID
		/// @note Ϊ������ʹ��
		/// @return ����-1��ʾ��Ԫ��
		int FirstObject(int groupId)LNOEXCEPT;

		/// @brief ���Զ�����
		int GetAttr(lua_State* L)LNOEXCEPT;

		/// @brief ����д����
		int SetAttr(lua_State* L)LNOEXCEPT;

		/// @brief ����Ŀ�ģ���ȡ�����б�
		int GetObjectTable(lua_State* L)LNOEXCEPT;

		/// @brief �������ӳ���ز���
		int ParticleStop(lua_State* L)LNOEXCEPT;
		int ParticleFire(lua_State* L)LNOEXCEPT;
		int ParticleGetn(lua_State* L)LNOEXCEPT;
		int ParticleGetEmission(lua_State* L)LNOEXCEPT;
		int ParticleSetEmission(lua_State* L)LNOEXCEPT;
	private:
		GameObjectPool& operator=(const GameObjectPool&);
		GameObjectPool(const GameObjectPool&);
	public:
		GameObjectPool(lua_State* pL);
		~GameObjectPool();
	};
}
