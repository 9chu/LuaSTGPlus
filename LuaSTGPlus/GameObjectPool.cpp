#include "GameObjectPool.h"
#include "AppFrame.h"

#define METATABLE_OBJ "mt"

#define GETOBJTABLE \
	do { \
		lua_pushlightuserdata(L, (void*)&LAPP); \
		lua_gettable(L, LUA_REGISTRYINDEX); \
	} while (false)

using namespace std;
using namespace LuaSTGPlus;

GameObjectPool::GameObjectPool(lua_State* pL)
	: L(pL)
{
	// ��ʼ��αͷ������
	memset(&m_pObjectListHeader, 0, sizeof(GameObject));
	memset(&m_pRenderListHeader, 0, sizeof(GameObject));
	memset(m_pCollisionListHeader, 0, sizeof(m_pCollisionListHeader));
	memset(&m_pObjectListTail, 0, sizeof(GameObject));
	memset(&m_pRenderListTail, 0, sizeof(GameObject));
	memset(m_pCollisionListTail, 0, sizeof(m_pCollisionListTail));
	m_pObjectListHeader.pObjectNext = &m_pObjectListTail;
	m_pObjectListTail.pObjectPrev = &m_pObjectListHeader;
	m_pRenderListHeader.pRenderNext = &m_pRenderListTail;
	m_pRenderListTail.pRenderPrev = &m_pRenderListHeader;
	for (size_t i = 0; i < LGOBJ_GROUPCNT; ++i)
	{
		m_pCollisionListHeader[i].pCollisionNext = &m_pCollisionListTail[i];
		m_pCollisionListTail[i].pCollisionPrev = &m_pCollisionListHeader[i];
	}

	// ����һ��ȫ�ֱ����ڴ�����ж���
	lua_pushlightuserdata(L, (void*)&LAPP);  // p(ʹ��APPʵ��ָ���������Է�ֹ�û�����)
	lua_createtable(L, LGOBJ_MAXCNT, 0);  // p t(�����㹻���table���ڴ�����е���Ϸ������lua�еĶ�Ӧ����)

	// ȡ��lstg.GetAttr��lstg.SetAttr����Ԫ��
	lua_newtable(L);  // ... t
	lua_getglobal(L, "lstg");  // ... t t
	lua_pushstring(L, "GetAttr");  // ... t t s
	lua_gettable(L, -2);  // ... t t f(GetAttr)
	lua_pushstring(L, "SetAttr");  // ... t t f(GetAttr) s
	lua_gettable(L, -3);  // ... t t f(GetAttr) f(SetAttr)
	LASSERT(lua_iscfunction(L, -1) && lua_iscfunction(L, -2));
	lua_setfield(L, -4, "__newindex");  // ... t t f(GetAttr)
	lua_setfield(L, -3, "__index");  // ... t t
	lua_pop(L, 1);  // ... t(��������Ԫ��)
	
	// ����Ԫ�� register[app][mt]
	lua_setfield(L, -2, METATABLE_OBJ);  // p t
	lua_settable(L, LUA_REGISTRYINDEX);
}

bool GameObjectPool::collisionCheck(GameObject* p1, GameObject* p2)LNOEXCEPT
{
	if (!p1->colli || !p2->colli)  // ���Բ���ײ����
		return false;

	// ! ����luastg�Ĵ��� ԭ������
	double a = p2->a;
	double b = p2->b;
	double r = p1->a;
	double l = a + b + r;
	double dx = p2->x - p1->x;
	double dy = p2->y - p1->y;
	if (fabs(dx) > l || fabs(dy) > l)
		return false;
	double x = dx*cos(p1->rot) + dy*sin(p1->rot);
	double y = -dx*sin(p1->rot) + dy*cos(p1->rot);
	a += r;
	b += r;
	a = a*a; b = b*b;
	x = x*x; y = y*y;
	if (p2->rect)
		return x < a && y < b;
	else
		return (x * b + y * a) < a * b;
}

GameObject* GameObjectPool::freeObject(GameObject* p)LNOEXCEPT
{
	GameObject* pRet = p->pObjectNext;

	// �Ӷ��������Ƴ�
	p->pObjectPrev->pObjectNext = p->pObjectNext;
	p->pObjectNext->pObjectPrev = p->pObjectPrev;

	// ����Ⱦ�����Ƴ�
	p->pRenderPrev->pRenderNext = p->pRenderNext;
	p->pRenderNext->pRenderPrev = p->pRenderPrev;

	// ����ײ�����Ƴ�
	p->pCollisionPrev->pCollisionNext = p->pCollisionNext;
	p->pCollisionNext->pCollisionPrev = p->pCollisionPrev;

	// ɾ��lua�������Ԫ��
	GETOBJTABLE;  // ot
	lua_pushnil(L);  // ot nil
	lua_rawseti(L, -2, p->id + 1);  // ot
	lua_pop(L, 1);

	// ���յ������
	m_ObjectPool.Free(p->id);

	return pRet;
}

void GameObjectPool::DoFrame()LNOEXCEPT
{
	GETOBJTABLE;  // ot
	
	GameObject* p = m_pObjectListHeader.pObjectNext;
	while (p && p != &m_pObjectListTail)
	{
		// ����id��ȡ�����lua��table���õ�class���õ�framefunc
		lua_rawgeti(L, -1, p->id + 1);  // ot t(object)
		lua_rawgeti(L, -1, 1);  // ot t(object) t(class)
		lua_rawgeti(L, -1, LGOBJ_CC_FRAME);  // ot t(object) t(class) f(frame)
		lua_pushvalue(L, -3);  // ot t(object) t(class) f(frame) t(object)
		lua_call(L, 1, 0);  // ot t(object) t(class) ִ��֡����
		lua_pop(L, 2);  // ot

		// ���¶���״̬
		p->x += p->vx;
		p->y += p->vy;
		p->rot += p->omiga;

		// ! TODO: ��������ϵͳ�����У�
		// ..

		p = p->pObjectNext;
	}

	lua_pop(L, 1);
}

void GameObjectPool::DoRender()LNOEXCEPT
{
	GETOBJTABLE;  // ot

	GameObject* p = m_pRenderListHeader.pObjectNext;
	while (p && p != &m_pRenderListTail)
	{
		if (!p->hide)  // ֻ��Ⱦ�ɼ�����
		{
			// ����id��ȡ�����lua��table���õ�class���õ�renderfunc
			lua_rawgeti(L, -1, p->id + 1);  // ot t(object)
			lua_rawgeti(L, -1, 1);  // ot t(object) t(class)
			lua_rawgeti(L, -1, LGOBJ_CC_RENDER);  // ot t(object) t(class) f(render)
			lua_pushvalue(L, -3);  // ot t(object) t(class) f(render) t(object)
			lua_call(L, 1, 0);  // ot t(object) t(class) ִ����Ⱦ����
			lua_pop(L, 2);  // ot
		}
		p = p->pRenderNext;
	}

	lua_pop(L, 1);
}

void GameObjectPool::BoundCheck()LNOEXCEPT
{
	GETOBJTABLE;  // ot

	GameObject* p = m_pObjectListHeader.pObjectNext;
	while (p && p != &m_pObjectListTail)
	{
		if ((p->x < m_BoundLeft || p->x > m_BoundRight || p->y < m_BoundBottom || p->y > m_BoundTop) && p->bound)
		{
			// Խ������ΪDEL״̬
			p->status = STATUS_DEL;

			// ����id��ȡ�����lua��table���õ�class���õ�delfunc
			lua_rawgeti(L, -1, p->id + 1);  // ot t(object)
			lua_rawgeti(L, -1, 1);  // ot t(object) t(class)
			lua_rawgeti(L, -1, LGOBJ_CC_DEL);  // ot t(object) t(class) f(del)
			lua_pushvalue(L, -3);  // ot t(object) t(class) f(del) t(object)
			lua_call(L, 1, 0);  // ot t(object) t(class)
			lua_pop(L, 2);  // ot
		}
		p = p->pObjectNext;
	}

	lua_pop(L, 1);
}

void GameObjectPool::CollisionCheck(size_t groupA, size_t groupB)LNOEXCEPT
{
	if (groupA >= LGOBJ_MAXCNT || groupB >= LGOBJ_MAXCNT)
		luaL_error(L, "Invalid collision group.");

	GETOBJTABLE;  // ot

	GameObject* pA = m_pCollisionListHeader[groupA].pObjectNext;
	GameObject* pATail = &m_pCollisionListTail[groupA];
	GameObject* pBHeader = m_pCollisionListHeader[groupB].pObjectNext;
	GameObject* pBTail = &m_pCollisionListTail[groupB];
	while (pA && pA != pATail)
	{
		GameObject* pB = pBHeader;
		while (pB && pB != pBTail)
		{
			if (collisionCheck(pA, pB))
			{
				// ����id��ȡ�����lua��table���õ�class���õ�collifunc
				lua_rawgeti(L, -1, pA->id + 1);  // ot t(object)
				lua_rawgeti(L, -1, 1);  // ot t(object) t(class)
				lua_rawgeti(L, -1, LGOBJ_CC_COLLI);  // ot t(object) t(class) f(colli)
				lua_pushvalue(L, -3);  // ot t(object) t(class) f(del) t(object)
				lua_rawgeti(L, -1, pB->id + 1);  // ot t(object) t(class) f(del) t(object) t(object)
				lua_call(L, 2, 0);  // ot t(object) t(class)
				lua_pop(L, 2);  // ot
			}
			pB = pB->pCollisionNext;
		}
		pA = pA->pCollisionNext;
	}

	lua_pop(L, 1);
}

void GameObjectPool::UpdateXY()LNOEXCEPT
{
	GETOBJTABLE;  // ot

	GameObject* p = m_pObjectListHeader.pObjectNext;
	while (p && p != &m_pObjectListTail)
	{
		p->dx = p->x - p->lastx;
		p->dy = p->y - p->lasty;
		p->lastx = p->x;
		p->lasty = p->y;
		if (p->navi && (p->dx != 0 || p->dy != 0))
			p->rot = atan2(p->dy, p->dx);

		p = p->pObjectNext;
	}

	lua_pop(L, 1);
}

void GameObjectPool::AfterFrame()LNOEXCEPT
{
	GETOBJTABLE;  // ot

	GameObject* p = m_pObjectListHeader.pObjectNext;
	while (p && p != &m_pObjectListTail)
	{
		p->timer++;
		p->ani_timer++;
		if (p->status != STATUS_DEFAULT)
			p = freeObject(p);
		else
			p = p->pObjectNext;
	}

	lua_pop(L, 1);
}

int GameObjectPool::New(lua_State* L)LNOEXCEPT
{
	// ������
	if (!lua_istable(L, 1))
		return luaL_error(L, "invalid argument #1, luastg object class required for 'New'.");
	lua_getfield(L, 1, "is_class");  // t(class) ... b
	if (!lua_toboolean(L, -1))
		return luaL_error(L, "invalid argument #1, luastg object class required for 'New'.");
	lua_pop(L, 1);  // t(class) ...

	// ����һ������
	size_t id = 0;
	if (!m_ObjectPool.Alloc(id))
		return luaL_error(L, "can't alloc object, object pool may be full.");
	
	// ���ö���
	GameObject* p = m_ObjectPool.Data(id);
	LASSERT(p);
	p->Reset();
	p->status = STATUS_DEFAULT;
	p->id = id;
	p->uid = m_iUid++;
	
	// ����������
	// ! TODO
	p->pObjectPrev = m_pObjectListTail.pObjectPrev;
	p->pObjectPrev->pObjectNext = p;
	p->pObjectNext = &m_pObjectListTail;
	m_pObjectListTail.pObjectPrev = p;
	p->pRenderPrev = m_pRenderListTail.pRenderPrev;
	p->pRenderPrev->pRenderNext = p;
	p->pRenderNext = &m_pRenderListTail;
	m_pRenderListTail.pRenderPrev = p;
	p->pCollisionPrev = m_pCollisionListTail[p->group].pCollisionPrev;
	p->pCollisionPrev->pCollisionNext = p;
	p->pCollisionNext = &m_pCollisionListTail[p->group];
	m_pCollisionListTail[p->group].pCollisionPrev = p;
	
	GETOBJTABLE;  // t(class) ... ot
	lua_createtable(L, 2, 0);  // t(class) ... ot t(object)
	lua_pushvalue(L, 1);  // t(class) ... ot t(object) class
	lua_rawseti(L, -2, 1);  // t(class) ... ot t(object)  ����class
	lua_pushinteger(L, (lua_Integer)id);  // t(class) ... ot t(object) id
	lua_rawseti(L, -2, 2);  // t(class) ... ot t(object)  ����id
	lua_getfield(L, -2, METATABLE_OBJ);  // t(class) ... ot t(object) mt
	lua_setmetatable(L, -2);  // t(class) ... ot t(object)  ����Ԫ��
	lua_pushvalue(L, -1);  // t(class) ... ot t(object) t(object)
	lua_rawseti(L, -3, id + 1);  // t(class) ... ot t(object)  ���õ�ȫ�ֱ�
	lua_insert(L, 1);  // t(object) t(class) ... ot
	lua_pop(L, 1);  // t(object) t(class) ...
	lua_rawgeti(L, 2, LGOBJ_CC_INIT);  // t(object) t(class) ... f(init)
	lua_insert(L, 3);  // t(object) t(class) f(init) ...
	lua_pushvalue(L, 1);  // t(object) t(class) f(init) ... t(object)
	lua_insert(L, 4);  // t(object) t(class) f(init) t(object) ...
	lua_call(L, lua_gettop(L) - 3, 0);  // t(object) t(class)  ִ�й��캯��
	lua_pop(L, 1);  // t(object)

	p->lastx = p->x;
	p->lasty = p->y;
	return 1;
}

int GameObjectPool::Del(lua_State* L)LNOEXCEPT
{
	if (!lua_istable(L, 1))
		return luaL_error(L, "invalid argument #1, luastg object required for 'Del'.");
	lua_rawgeti(L, 1, 2);  // t(object) ... id
	GameObject* p = m_ObjectPool.Data((size_t)luaL_checknumber(L, -1));
	lua_pop(L, 1);  // t(object) ...
	if (!p)
		return luaL_error(L, "invalid argument #1, invalid luastg object.");
	
	if (p->status == STATUS_DEFAULT)
	{
		p->status = STATUS_DEL;

		// �������еĻص�����
		lua_rawgeti(L, 1, 1);  // t(object) ... class
		lua_rawgeti(L, -1, LGOBJ_CC_DEL);  // t(object) ... class f(del)
		lua_insert(L, 1);  // f(del) t(object) ... class
		lua_pop(L, 1);  // f(del) t(object) ...
		lua_call(L, lua_gettop(L) - 1, 0);
	}
	return 0;
}

int GameObjectPool::Kill(lua_State* L)LNOEXCEPT
{
	if (!lua_istable(L, 1))
		return luaL_error(L, "invalid argument #1, luastg object required for 'Kill'.");
	lua_rawgeti(L, 1, 2);  // t(object) ... id
	GameObject* p = m_ObjectPool.Data((size_t)luaL_checknumber(L, -1));
	lua_pop(L, 1);  // t(object) ...
	if (!p)
		return luaL_error(L, "invalid argument #1, invalid luastg object.");

	if (p->status == STATUS_DEFAULT)
	{
		p->status = STATUS_KILL;

		// �������еĻص�����
		lua_rawgeti(L, 1, 1);  // t(object) ... class
		lua_rawgeti(L, -1, LGOBJ_CC_KILL);  // t(object) ... class f(kill)
		lua_insert(L, 1);  // f(kill) t(object) ... class
		lua_pop(L, 1);  // f(kill) t(object) ...
		lua_call(L, lua_gettop(L) - 1, 0);
	}
	return 0;
}
