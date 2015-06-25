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
	// 初始化伪头部数据
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

	// 创建一个全局表用于存放所有对象
	lua_pushlightuserdata(L, (void*)&LAPP);  // p(使用APP实例指针作键用以防止用户访问)
	lua_createtable(L, LGOBJ_MAXCNT, 0);  // p t(创建足够大的table用于存放所有的游戏对象在lua中的对应对象)

	// 取出lstg.GetAttr和lstg.SetAttr创建元表
	lua_newtable(L);  // ... t
	lua_getglobal(L, "lstg");  // ... t t
	lua_pushstring(L, "GetAttr");  // ... t t s
	lua_gettable(L, -2);  // ... t t f(GetAttr)
	lua_pushstring(L, "SetAttr");  // ... t t f(GetAttr) s
	lua_gettable(L, -3);  // ... t t f(GetAttr) f(SetAttr)
	LASSERT(lua_iscfunction(L, -1) && lua_iscfunction(L, -2));
	lua_setfield(L, -4, "__newindex");  // ... t t f(GetAttr)
	lua_setfield(L, -3, "__index");  // ... t t
	lua_pop(L, 1);  // ... t(将被用作元表)
	
	// 保存元表到 register[app][mt]
	lua_setfield(L, -2, METATABLE_OBJ);  // p t
	lua_settable(L, LUA_REGISTRYINDEX);
}

bool GameObjectPool::collisionCheck(GameObject* p1, GameObject* p2)LNOEXCEPT
{
	if (!p1->colli || !p2->colli)  // 忽略不碰撞对象
		return false;

	// ! 来自luastg的代码 原理不明。
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

	// 从对象链表移除
	p->pObjectPrev->pObjectNext = p->pObjectNext;
	p->pObjectNext->pObjectPrev = p->pObjectPrev;

	// 从渲染链表移除
	p->pRenderPrev->pRenderNext = p->pRenderNext;
	p->pRenderNext->pRenderPrev = p->pRenderPrev;

	// 从碰撞链表移除
	p->pCollisionPrev->pCollisionNext = p->pCollisionNext;
	p->pCollisionNext->pCollisionPrev = p->pCollisionPrev;

	// 删除lua对象表中元素
	GETOBJTABLE;  // ot
	lua_pushnil(L);  // ot nil
	lua_rawseti(L, -2, p->id + 1);  // ot
	lua_pop(L, 1);

	// 回收到对象池
	m_ObjectPool.Free(p->id);

	return pRet;
}

void GameObjectPool::DoFrame()LNOEXCEPT
{
	GETOBJTABLE;  // ot
	
	GameObject* p = m_pObjectListHeader.pObjectNext;
	while (p && p != &m_pObjectListTail)
	{
		// 根据id获取对象的lua绑定table、拿到class再拿到framefunc
		lua_rawgeti(L, -1, p->id + 1);  // ot t(object)
		lua_rawgeti(L, -1, 1);  // ot t(object) t(class)
		lua_rawgeti(L, -1, LGOBJ_CC_FRAME);  // ot t(object) t(class) f(frame)
		lua_pushvalue(L, -3);  // ot t(object) t(class) f(frame) t(object)
		lua_call(L, 1, 0);  // ot t(object) t(class) 执行帧函数
		lua_pop(L, 2);  // ot

		// 更新对象状态
		p->x += p->vx;
		p->y += p->vy;
		p->rot += p->omiga;

		// ! TODO: 更新粒子系统（若有）
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
		if (!p->hide)  // 只渲染可见对象
		{
			// 根据id获取对象的lua绑定table、拿到class再拿到renderfunc
			lua_rawgeti(L, -1, p->id + 1);  // ot t(object)
			lua_rawgeti(L, -1, 1);  // ot t(object) t(class)
			lua_rawgeti(L, -1, LGOBJ_CC_RENDER);  // ot t(object) t(class) f(render)
			lua_pushvalue(L, -3);  // ot t(object) t(class) f(render) t(object)
			lua_call(L, 1, 0);  // ot t(object) t(class) 执行渲染函数
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
			// 越界设置为DEL状态
			p->status = STATUS_DEL;

			// 根据id获取对象的lua绑定table、拿到class再拿到delfunc
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
				// 根据id获取对象的lua绑定table、拿到class再拿到collifunc
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
	// 检查参数
	if (!lua_istable(L, 1))
		return luaL_error(L, "invalid argument #1, luastg object class required for 'New'.");
	lua_getfield(L, 1, "is_class");  // t(class) ... b
	if (!lua_toboolean(L, -1))
		return luaL_error(L, "invalid argument #1, luastg object class required for 'New'.");
	lua_pop(L, 1);  // t(class) ...

	// 分配一个对象
	size_t id = 0;
	if (!m_ObjectPool.Alloc(id))
		return luaL_error(L, "can't alloc object, object pool may be full.");
	
	// 设置对象
	GameObject* p = m_ObjectPool.Data(id);
	LASSERT(p);
	p->Reset();
	p->status = STATUS_DEFAULT;
	p->id = id;
	p->uid = m_iUid++;
	
	// 插入链表域
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
	lua_rawseti(L, -2, 1);  // t(class) ... ot t(object)  设置class
	lua_pushinteger(L, (lua_Integer)id);  // t(class) ... ot t(object) id
	lua_rawseti(L, -2, 2);  // t(class) ... ot t(object)  设置id
	lua_getfield(L, -2, METATABLE_OBJ);  // t(class) ... ot t(object) mt
	lua_setmetatable(L, -2);  // t(class) ... ot t(object)  设置元表
	lua_pushvalue(L, -1);  // t(class) ... ot t(object) t(object)
	lua_rawseti(L, -3, id + 1);  // t(class) ... ot t(object)  设置到全局表
	lua_insert(L, 1);  // t(object) t(class) ... ot
	lua_pop(L, 1);  // t(object) t(class) ...
	lua_rawgeti(L, 2, LGOBJ_CC_INIT);  // t(object) t(class) ... f(init)
	lua_insert(L, 3);  // t(object) t(class) f(init) ...
	lua_pushvalue(L, 1);  // t(object) t(class) f(init) ... t(object)
	lua_insert(L, 4);  // t(object) t(class) f(init) t(object) ...
	lua_call(L, lua_gettop(L) - 3, 0);  // t(object) t(class)  执行构造函数
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

		// 调用类中的回调方法
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

		// 调用类中的回调方法
		lua_rawgeti(L, 1, 1);  // t(object) ... class
		lua_rawgeti(L, -1, LGOBJ_CC_KILL);  // t(object) ... class f(kill)
		lua_insert(L, 1);  // f(kill) t(object) ... class
		lua_pop(L, 1);  // f(kill) t(object) ...
		lua_call(L, lua_gettop(L) - 1, 0);
	}
	return 0;
}
