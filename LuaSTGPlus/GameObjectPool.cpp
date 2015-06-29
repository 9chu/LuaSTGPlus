#include "GameObjectPool.h"
#include "GameObjectPropertyHash.inl"
#include "AppFrame.h"

#define METATABLE_OBJ "mt"

#ifdef min
#undef min
#endif
#ifdef max
#undef max
#endif

#define GETOBJTABLE \
	do { \
		lua_pushlightuserdata(L, (void*)&LAPP); \
		lua_gettable(L, LUA_REGISTRYINDEX); \
	} while (false)

#define LIST_INSERT_BEFORE(target, p, field) \
	do { \
		p->p##field##Prev = (target)->p##field##Prev; \
		p->p##field##Next = (target); \
		p->p##field##Prev->p##field##Next = p; \
		p->p##field##Next->p##field##Prev = p; \
	} while(false)

#define LIST_INSERT_AFTER(target, p, field) \
	do { \
		p->p##field##Prev = (target); \
		p->p##field##Next = (target)->p##field##Next; \
		p->p##field##Prev->p##field##Next = p; \
		p->p##field##Next->p##field##Prev = p; \
	} while(false)

#define LIST_REMOVE(p, field) \
	do { \
		p->p##field##Prev->p##field##Next = p->p##field##Next; \
		p->p##field##Next->p##field##Prev = p->p##field##Prev; \
	} while(false)

#define LIST_INSERT_SORT(p, field, func) \
	do { \
		if (p->p##field##Next->p##field##Next && func(p->p##field##Next, p)) \
		{ \
			GameObject* pInsertBefore = p->p##field##Next->p##field##Next; \
			while (pInsertBefore->p##field##Next && func(pInsertBefore, p)) \
				pInsertBefore = pInsertBefore->p##field##Next; \
			LIST_REMOVE(p, field); \
			LIST_INSERT_BEFORE(pInsertBefore, p, field); \
		} \
		else if (p->p##field##Prev->p##field##Prev && func(p, p->p##field##Prev)) \
		{ \
			GameObject* pInsertAfter = p->p##field##Prev->p##field##Prev; \
			while (pInsertAfter->p##field##Prev && func(p, pInsertAfter)) \
				pInsertAfter = pInsertAfter->p##field##Prev; \
			LIST_REMOVE(p, field); \
			LIST_INSERT_AFTER(pInsertAfter, p, field); \
		} \
	} while (false)

using namespace std;
using namespace LuaSTGPlus;

inline bool ObjectListSortFunc(GameObject* p1, GameObject* p2)LNOEXCEPT
{
	// 总是以uid为参照
	return p1->uid < p2->uid;
}

inline bool RenderListSortFunc(GameObject* p1, GameObject* p2)LNOEXCEPT
{
	// layer小的靠前。若layer相同则参照uid。
	return (p1->layer < p2->layer) || ((p1->layer == p2->layer) && (p1->uid < p2->uid));
}

bool GameObject::ChangeResource(const char* res_name)
{
	LASSERT(!res);

	fcyRefPointer<ResSprite> tSprite = LRES.FindSprite(res_name);
	if (tSprite)
	{
		res = tSprite;
		res->AddRef();
		a = tSprite->GetHalfSizeX() * LRES.GetGlobalImageScaleFactor();
		b = tSprite->GetHalfSizeY() * LRES.GetGlobalImageScaleFactor();
		rect = tSprite->IsRectangle();
		return true;
	}

	fcyRefPointer<ResAnimation> tAnimation = LRES.FindAnimation(res_name);
	if (tAnimation)
	{
		res = tAnimation;
		res->AddRef();
		a = tAnimation->GetHalfSizeX() * LRES.GetGlobalImageScaleFactor();
		b = tAnimation->GetHalfSizeY() * LRES.GetGlobalImageScaleFactor();
		rect = tAnimation->IsRectangle();
		return true;
	}

	fcyRefPointer<ResParticle> tParticle = LRES.FindParticle(res_name);
	if (tParticle)
	{
		res = tParticle;
		if (!(ps = tParticle->AllocInstance()))
		{
			res = nullptr;
			LERROR("无法构造粒子池，内存不足");
			return false;
		}
		ps->SetCenter(fcyVec2((float)x, (float)y));
		ps->SetRotation(rot);

		res->AddRef();
		a = tParticle->GetHalfSizeX() * LRES.GetGlobalImageScaleFactor();
		b = tParticle->GetHalfSizeY() * LRES.GetGlobalImageScaleFactor();
		rect = tParticle->IsRectangle();
		return true;
	}

	return false;
}

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
	m_pObjectListHeader.uid = numeric_limits<uint64_t>::min();
	m_pObjectListTail.pObjectPrev = &m_pObjectListHeader;
	m_pObjectListTail.uid = numeric_limits<uint64_t>::max();
	m_pRenderListHeader.pRenderNext = &m_pRenderListTail;
	m_pRenderListHeader.uid = numeric_limits<uint64_t>::min();
	m_pRenderListHeader.layer = numeric_limits<lua_Number>::min();
	m_pRenderListTail.pRenderPrev = &m_pRenderListHeader;
	m_pRenderListTail.uid = numeric_limits<uint64_t>::max();
	m_pRenderListTail.layer = numeric_limits<lua_Number>::max();
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

GameObjectPool::~GameObjectPool()
{
	ResetPool();
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
	LIST_REMOVE(p, Object);

	// 从渲染链表移除
	LIST_REMOVE(p, Render);

	// 从碰撞链表移除
	LIST_REMOVE(p, Collision);

	// 删除lua对象表中元素
	GETOBJTABLE;  // ot
	lua_pushnil(L);  // ot nil
	lua_rawseti(L, -2, p->id + 1);  // ot
	lua_pop(L, 1);

	// 释放引用的资源
	p->ReleaseResource();

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

		// 更新粒子系统（若有）
		if (p->res && p->res->GetType() == ResourceType::Particle)
		{
			float gscale = LRES.GetGlobalImageScaleFactor();
			p->ps->SetRotation((float)p->rot);
			p->ps->SetCenter(fcyVec2((float)p->x, (float)p->y));
			p->ps->Update(1.0f / 60.f);
		}

		p = p->pObjectNext;
	}

	lua_pop(L, 1);
}

void GameObjectPool::DoRender()LNOEXCEPT
{
	GETOBJTABLE;  // ot

	GameObject* p = m_pRenderListHeader.pRenderNext;
	LASSERT(p != nullptr);
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

	GameObject* pA = m_pCollisionListHeader[groupA].pCollisionNext;
	GameObject* pATail = &m_pCollisionListTail[groupA];
	GameObject* pBHeader = m_pCollisionListHeader[groupB].pCollisionNext;
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
				lua_pushvalue(L, -3);  // ot t(object) t(class) f(colli) t(object)
				lua_rawgeti(L, -5, pB->id + 1);  // ot t(object) t(class) f(colli) t(object) t(object)
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
}

void GameObjectPool::AfterFrame()LNOEXCEPT
{
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
	LIST_INSERT_BEFORE(&m_pObjectListTail, p, Object);  // Object链表只与uid有关，因此总在末尾插入
	LIST_INSERT_BEFORE(&m_pRenderListTail, p, Render);  // Render链表在插入后还需要进行排序
	LIST_INSERT_BEFORE(&m_pCollisionListTail[p->group], p, Collision);  // 为保证兼容性，对Collision也做排序
	LIST_INSERT_SORT(p, Render, RenderListSortFunc);
	LIST_INSERT_SORT(p, Collision, ObjectListSortFunc);

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

int GameObjectPool::IsValid(lua_State* L)LNOEXCEPT
{
	if (lua_gettop(L) != 1)
		return luaL_error(L, "invalid argument count, 1 argument required for 'IsValid'.");
	if (!lua_istable(L, -1))
	{
		lua_pushboolean(L, 0);
		return 1;
	}
	lua_rawgeti(L, -1, 2);  // t(object) id
	if (!lua_isnumber(L, -1))
	{
		lua_pushboolean(L, 0);
		return 1;
	}

	// 在对象池中检查
	size_t id = (size_t)lua_tonumber(L, -1);
	lua_pop(L, 1);  // t(object)
if (!m_ObjectPool.Data(id))
{
	lua_pushboolean(L, 0);
	return 1;
}

GETOBJTABLE;  // t(object) ot
lua_rawgeti(L, -1, (lua_Integer)(id + 1));  // t(object) ot t(object)
if (lua_rawequal(L, -1, -3))
lua_pushboolean(L, 1);
else
lua_pushboolean(L, 0);
return 1;
}

bool GameObjectPool::Angle(size_t idA, size_t idB, double& out)LNOEXCEPT
{
	GameObject* pA = m_ObjectPool.Data(idA);
	GameObject* pB = m_ObjectPool.Data(idB);
	if (!pA || !pB)
		return false;
	out = LRAD2DEGREE * atan2(pB->y - pA->y, pB->x - pA->x);
	return true;
}

bool GameObjectPool::Dist(size_t idA, size_t idB, double& out)LNOEXCEPT
{
	GameObject* pA = m_ObjectPool.Data(idA);
	GameObject* pB = m_ObjectPool.Data(idB);
	if (!pA || !pB)
		return false;
	lua_Number dx = pB->x - pA->x;
	lua_Number dy = pB->y - pA->y;
	out = sqrt(dx*dx + dy*dy);
	return true;
}

bool GameObjectPool::SetV(size_t id, double v, double a, bool updateRot)LNOEXCEPT
{
	GameObject* p = m_ObjectPool.Data(id);
	if (!p)
		return false;
	a *= LDEGREE2RAD;
	p->vx = v*cos(a);
	p->vy = v*sin(a);
	if (updateRot)
		p->rot = a;
	return true;
}

bool GameObjectPool::SetImgState(size_t id, BlendMode m, fcyColor c)LNOEXCEPT
{
	GameObject* p = m_ObjectPool.Data(id);
	if (!p)
		return false;
	if (p->res)
	{
		switch (p->res->GetType())
		{
		case ResourceType::Sprite:
			static_cast<ResSprite*>(p->res)->SetBlendMode(m);
			static_cast<ResSprite*>(p->res)->GetSprite()->SetColor(c);
			break;
		case ResourceType::Animation:
			do {
				ResAnimation* ani = static_cast<ResAnimation*>(p->res);
				ani->SetBlendMode(m);
				for (size_t i = 0; i < ani->GetCount(); ++i)
					ani->GetSprite(i)->SetColor(c);
			} while (false);
			break;
		default:
			break;
		}
	}
	return true;
}

bool GameObjectPool::BoxCheck(size_t id, double left, double right, double top, double bottom, bool& ret)LNOEXCEPT
{
	GameObject* p = m_ObjectPool.Data(id);
	if (!p)
		return false;
	ret = (p->x > left) && (p->x < right) && (p->y > top) && (p->y < bottom);
	return true;
}

void GameObjectPool::ResetPool()LNOEXCEPT
{
	GameObject* p = m_pObjectListHeader.pObjectNext;
	while (p != &m_pObjectListTail)
		p = freeObject(p);
}

bool GameObjectPool::DoDefaultRender(size_t id)LNOEXCEPT
{
	GameObject* p = m_ObjectPool.Data(id);
	if (!p)
		return false;

	if (p->res)
	{
		switch (p->res->GetType())
		{
		case ResourceType::Sprite:
			LAPP.Render(
				static_cast<ResSprite*>(p->res),
				static_cast<float>(p->x),
				static_cast<float>(p->y),
				static_cast<float>(p->rot),
				static_cast<float>(p->hscale * LRES.GetGlobalImageScaleFactor()),
				static_cast<float>(p->vscale * LRES.GetGlobalImageScaleFactor())
			);
			break;
		case ResourceType::Animation:
			LAPP.Render(
				static_cast<ResAnimation*>(p->res),
				p->ani_timer,
				static_cast<float>(p->x),
				static_cast<float>(p->y),
				static_cast<float>(p->rot),
				static_cast<float>(p->hscale * LRES.GetGlobalImageScaleFactor()),
				static_cast<float>(p->vscale * LRES.GetGlobalImageScaleFactor())
			);
			break;
		case ResourceType::Particle:
			LAPP.Render(
				p->ps,
				static_cast<float>(p->hscale * LRES.GetGlobalImageScaleFactor()),
				static_cast<float>(p->vscale * LRES.GetGlobalImageScaleFactor())
			);
			break;
		default:
			break;
		}
	}
	
	return true;
}

int GameObjectPool::NextObject(int groupId, int id)LNOEXCEPT
{
	if (id < 0)
		return -1;

	GameObject* p = m_ObjectPool.Data(static_cast<size_t>(id));
	if (!p)
		return -1;

	// 如果不是一个有效的分组，则在整个对象表中遍历
	if (groupId < 0 || groupId >= LGOBJ_GROUPCNT)
	{
		p = p->pObjectNext;
		if (p == &m_pObjectListTail)
			return -1;
		else
			return static_cast<int>(p->id);
	}
	else
	{
		if (p->group != groupId)
			return -1;
		p = p->pCollisionNext;
		if (p == &m_pCollisionListTail[groupId])
			return -1;
		else
			return static_cast<int>(p->id);
	}
}

int GameObjectPool::NextObject(lua_State* L)LNOEXCEPT
{
	int g = luaL_checkinteger(L, 1);  // i(groupId)
	int id = luaL_checkinteger(L, 2);  // id
	if (id < 0)
		return 0;

	lua_pushinteger(L, NextObject(g, id));  // ??? i(next)
	GETOBJTABLE;  // ??? i(next) ot
	lua_rawgeti(L, -1, id + 1);  // ??? i(next) ot t(object)
	lua_remove(L, -2);  // ??? i(next) t(object)
	return 2;
}

int GameObjectPool::FirstObject(int groupId)LNOEXCEPT
{
	GameObject* p;

	// 如果不是一个有效的分组，则在整个对象表中遍历
	if (groupId < 0 || groupId >= LGOBJ_GROUPCNT)
	{
		p = m_pObjectListHeader.pObjectNext;
		if (p == &m_pObjectListTail)
			return -1;
		else
			return static_cast<int>(p->id);
	}
	else
	{
		p = m_pCollisionListHeader[groupId].pCollisionNext;
		if (p == &m_pCollisionListTail[groupId])
			return -1;
		else
			return static_cast<int>(p->id);
	}
}

int GameObjectPool::GetAttr(lua_State* L)LNOEXCEPT
{
	lua_rawgeti(L, 1, 2);  // t(object) s(key) ??? i(id)
	size_t id = static_cast<size_t>(lua_tonumber(L, -1));
	lua_pop(L, 1);  // t(object) s(key)

	GameObject* p = m_ObjectPool.Data(id);
	if (!p)
		return luaL_error(L, "invalid lstg object for '__index' meta operation.");
	
	// 查询属性
	const char* key = luaL_checkstring(L, 2);
	
	// 对x,y作特化处理
	if (key[0] == 'x' && key[1] == '\0')
	{
		lua_pushnumber(L, p->x);
		return 1;
	}
	else if (key[0] == 'y' && key[1] == '\0')
	{
		lua_pushnumber(L, p->y);
		return 1;
	}

	// 一般属性
	switch (GameObjectPropertyHash(key))
	{
	case GameObjectProperty::DX:
		lua_pushnumber(L, p->dx);
		break;
	case GameObjectProperty::DY:
		lua_pushnumber(L, p->dy);
		break;
	case GameObjectProperty::ROT:
		lua_pushnumber(L, p->rot * LRAD2DEGREE);
		break;
	case GameObjectProperty::OMIGA:
		lua_pushnumber(L, p->omiga * LRAD2DEGREE);
		break;
	case GameObjectProperty::TIMER:
		lua_pushinteger(L, p->timer);
		break;
	case GameObjectProperty::VX:
		lua_pushnumber(L, p->vx);
		break;
	case GameObjectProperty::VY:
		lua_pushnumber(L, p->vy);
		break;
	case GameObjectProperty::LAYER:
		lua_pushnumber(L, p->layer);
		break;
	case GameObjectProperty::GROUP:
		lua_pushinteger(L, p->group);
		break;
	case GameObjectProperty::HIDE:
		lua_pushboolean(L, p->hide);
		break;
	case GameObjectProperty::BOUND:
		lua_pushboolean(L, p->bound);
		break;
	case GameObjectProperty::NAVI:
		lua_pushboolean(L, p->navi);
		break;
	case GameObjectProperty::COLLI:
		lua_pushboolean(L, p->colli);
		break;
	case GameObjectProperty::STATUS:
		switch (p->status)
		{
		case STATUS_DEFAULT:
			lua_pushstring(L, "normal");
			break;
		case STATUS_KILL:
			lua_pushstring(L, "kill");
			break;
		case STATUS_DEL:
			lua_pushstring(L, "del");
			break;
		default:
			LASSERT(false);
			break;
		}
		break;
	case GameObjectProperty::HSCALE:
		lua_pushnumber(L, p->hscale);
		break;
	case GameObjectProperty::VSCALE:
		lua_pushnumber(L, p->vscale);
		break;
	case GameObjectProperty::CLASS:
		lua_rawgeti(L, 1, 1);
		break;
	case GameObjectProperty::A:
		lua_pushnumber(L, p->a / LRES.GetGlobalImageScaleFactor());
		break;
	case GameObjectProperty::B:
		lua_pushnumber(L, p->b / LRES.GetGlobalImageScaleFactor());
		break;
	case GameObjectProperty::RECT:
		lua_pushboolean(L, p->rect);
		break;
	case GameObjectProperty::IMG:
		if (p->res)
			lua_pushstring(L, p->res->GetResName().c_str());
		else
			lua_pushnil(L);
		break;
	case GameObjectProperty::ANI:
		lua_pushinteger(L, p->ani_timer);
		break;
	case GameObjectProperty::X:
	case GameObjectProperty::Y:
	default:
		lua_pushnil(L);
		break;
	}

	return 1;
}

int GameObjectPool::SetAttr(lua_State* L)LNOEXCEPT
{
	lua_rawgeti(L, 1, 2);  // t(object) s(key) any(v) i(id)
	size_t id = static_cast<size_t>(lua_tonumber(L, -1));
	lua_pop(L, 1);  // t(object) s(key) any(v)

	GameObject* p = m_ObjectPool.Data(id);
	if (!p)
		return luaL_error(L, "invalid lstg object for '__newindex' meta operation.");

	// 查询属性
	const char* key = luaL_checkstring(L, 2);

	// 对x,y作特化处理
	if (key[0] == 'x' && key[1] == '\0')
	{
		p->x = luaL_checknumber(L, 3);
		return 0;
	}
	else if (key[0] == 'y' && key[1] == '\0')
	{
		p->y = luaL_checknumber(L, 3);
		return 0;
	}	

	// 一般属性
	switch (GameObjectPropertyHash(key))
	{
	case GameObjectProperty::DX:
		return luaL_error(L, "property 'dx' is readonly.");
	case GameObjectProperty::DY:
		return luaL_error(L, "property 'dy' is readonly.");
	case GameObjectProperty::ROT:
		p->rot = luaL_checknumber(L, 3) * LDEGREE2RAD;
		break;
	case GameObjectProperty::OMIGA:
		p->omiga = luaL_checknumber(L, 3) * LDEGREE2RAD;
		break;
	case GameObjectProperty::TIMER:
		p->timer = luaL_checkinteger(L, 3);
		break;
	case GameObjectProperty::VX:
		p->vx = luaL_checknumber(L, 3);
		break;
	case GameObjectProperty::VY:
		p->vy = luaL_checknumber(L, 3);
		break;
	case GameObjectProperty::LAYER:
		p->layer = luaL_checkinteger(L, 3);
		LIST_INSERT_SORT(p, Render, RenderListSortFunc); // 刷新p的渲染层级
		LASSERT(m_pRenderListHeader.pRenderNext != nullptr);
		LASSERT(m_pRenderListTail.pRenderPrev != nullptr);
		break;
	case GameObjectProperty::GROUP:
		do
		{
			int group = luaL_checkinteger(L, 3);
			if (group != p->group)
			{
				if (0 <= p->group && p->group < LGOBJ_GROUPCNT)
					LIST_REMOVE(p, Collision);
				p->group = group;
				if (0 <= group && group < LGOBJ_GROUPCNT)
				{
					LIST_INSERT_BEFORE(&m_pCollisionListTail[group], p, Collision);
					LIST_INSERT_SORT(p, Collision, ObjectListSortFunc);  // 刷新p的碰撞次序
					LASSERT(m_pCollisionListHeader[group].pCollisionNext != nullptr);
					LASSERT(m_pCollisionListTail[group].pCollisionPrev != nullptr);
				}
			}
		} while (false);
		break;
	case GameObjectProperty::HIDE:
		p->hide = lua_toboolean(L, 3) == 0 ? false : true;
		break;
	case GameObjectProperty::BOUND:
		p->bound = lua_toboolean(L, 3) == 0 ? false : true;
		break;
	case GameObjectProperty::NAVI:
		p->navi = lua_toboolean(L, 3) == 0 ? false : true;
		break;
	case GameObjectProperty::COLLI:
		p->colli = lua_toboolean(L, 3) == 0 ? false : true;
		break;
	case GameObjectProperty::STATUS:
		do {
			const char* val = luaL_checkstring(L, 3);
			if (strcmp(val, "normal") == 0)
				p->status = STATUS_DEFAULT;
			else if (strcmp(val, "del") == 0)
				p->status = STATUS_DEL;
			else if (strcmp(val, "kill") == 0)
				p->status = STATUS_KILL;
			else
				return luaL_error(L, "invalid argument for property 'status'.");
		} while (false);
		break;
	case GameObjectProperty::HSCALE:
		p->hscale = luaL_checknumber(L, 3);
		break;
	case GameObjectProperty::VSCALE:
		p->vscale = luaL_checknumber(L, 3);
		break;
	case GameObjectProperty::CLASS:
		lua_rawseti(L, 1, 1);
		break;
	case GameObjectProperty::A:
		p->a = luaL_checknumber(L, 3) * LRES.GetGlobalImageScaleFactor();
		break;
	case GameObjectProperty::B:
		p->b = luaL_checknumber(L, 3) * LRES.GetGlobalImageScaleFactor();
		break;
	case GameObjectProperty::RECT:
		p->rect = lua_toboolean(L, 3) == 0 ? false : true;
		break;
	case GameObjectProperty::IMG:
		p->ReleaseResource();
		if (!p->ChangeResource(luaL_checkstring(L, 3)))
			return luaL_error(L, "can't find resource '%s' in image/animation/particle pool.", luaL_checkstring(L, 3));
		break;
	case GameObjectProperty::ANI:
		return luaL_error(L, "property 'ani' is readonly.");
	case GameObjectProperty::X:
	case GameObjectProperty::Y:
		break;
	default:
		lua_rawset(L, 1);
		break;
	}
	return 0;
}

int GameObjectPool::GetObjectTable(lua_State* L)LNOEXCEPT
{
	GETOBJTABLE;
	return 1;
}

int GameObjectPool::ParticleStop(lua_State* L)LNOEXCEPT
{
	if (!lua_istable(L, 1))
		return luaL_error(L, "invalid lstg object for 'ParticleStop'.");
	lua_rawgeti(L, 1, 2);  // t(object) ??? id
	size_t id = (size_t)luaL_checkinteger(L, -1);
	lua_pop(L, 1);

	GameObject* p = m_ObjectPool.Data(id);
	if (!p)
		return luaL_error(L, "invalid lstg object for 'ParticleStop'.");
	if (!p->res || p->res->GetType() != ResourceType::Particle)
	{
		LWARNING("ParticleStop: 试图停止一个不带有粒子发射器的对象的粒子发射过程(uid=%d)", m_iUid);
		return 0;
	}	
	p->ps->SetInactive();
	return 0;
}

int GameObjectPool::ParticleFire(lua_State* L)LNOEXCEPT
{
	if (!lua_istable(L, 1))
	return luaL_error(L, "invalid lstg object for 'ParticleFire'.");
	lua_rawgeti(L, 1, 2);  // t(object) ??? id
	size_t id = (size_t)luaL_checkinteger(L, -1);
	lua_pop(L, 1);

	GameObject* p = m_ObjectPool.Data(id);
	if (!p)
		return luaL_error(L, "invalid lstg object for 'ParticleFire'.");
	if (!p->res || p->res->GetType() != ResourceType::Particle)
	{
		LWARNING("ParticleFire: 试图启动一个不带有粒子发射器的对象的粒子发射过程(uid=%d)", m_iUid);
		return 0;
	}	
	p->ps->SetActive();
	return 0;
}

int GameObjectPool::ParticleGetn(lua_State* L)LNOEXCEPT
{
	if (!lua_istable(L, 1))
	return luaL_error(L, "invalid lstg object for 'ParticleFire'.");
	lua_rawgeti(L, 1, 2);  // t(object) ??? id
	size_t id = (size_t)luaL_checkinteger(L, -1);
	lua_pop(L, 1);

	GameObject* p = m_ObjectPool.Data(id);
	if (!p)
		return luaL_error(L, "invalid lstg object for 'ParticleFire'.");
	if (!p->res || p->res->GetType() != ResourceType::Particle)
	{
		lua_pushinteger(L, 0);
		return 1;
	}
	lua_pushinteger(L, (int)p->ps->GetAliveCount());
	return 1;
}

int GameObjectPool::ParticleGetEmission(lua_State* L)LNOEXCEPT
{
	if (!lua_istable(L, 1))
	return luaL_error(L, "invalid lstg object for 'ParticleGetEmission'.");
	lua_rawgeti(L, 1, 2);  // t(object) ??? id
	size_t id = (size_t)luaL_checkinteger(L, -1);
	lua_pop(L, 1);

	GameObject* p = m_ObjectPool.Data(id);
	if (!p)
		return luaL_error(L, "invalid lstg object for 'ParticleGetEmission'.");
	if (!p->res || p->res->GetType() != ResourceType::Particle)
	{
		LWARNING("ParticleGetEmission: 试图获取一个不带有粒子发射器的对象的粒子发射密度(uid=%d)", m_iUid);
		lua_pushinteger(L, 0);
		return 1;
	}
	lua_pushnumber(L, p->ps->GetEmission());
	return 1;
}

int GameObjectPool::ParticleSetEmission(lua_State* L)LNOEXCEPT
{
	if (!lua_istable(L, 1))
	return luaL_error(L, "invalid lstg object for 'ParticleGetEmission'.");
	lua_rawgeti(L, 1, 2);  // t(object) ??? id
	size_t id = (size_t)luaL_checkinteger(L, -1);
	lua_pop(L, 1);

	GameObject* p = m_ObjectPool.Data(id);
	if (!p)
		return luaL_error(L, "invalid lstg object for 'ParticleGetEmission'.");
	if (!p->res || p->res->GetType() != ResourceType::Particle)
	{
		LWARNING("ParticleSetEmission: 试图设置一个不带有粒子发射器的对象的粒子发射密度(uid=%d)", m_iUid);
		return 0;
	}
	p->ps->SetEmission((float)::max(0., luaL_checknumber(L, -1)));
	return 0;
}
