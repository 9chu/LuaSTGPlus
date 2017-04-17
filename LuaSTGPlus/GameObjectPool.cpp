#include "GameObjectPool.h"
#include "GameObjectPropertyHash.inl"
#include "AppFrame.h"
#include "CollisionDetect.h"

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

static inline bool ObjectListSortFunc(GameObject* p1, GameObject* p2)LNOEXCEPT
{
	// 总是以uid为参照
	return p1->uid < p2->uid;
}

static inline bool RenderListSortFunc(GameObject* p1, GameObject* p2)LNOEXCEPT
{
	// layer小的靠前。若layer相同则参照uid。
	return (p1->layer < p2->layer) || ((p1->layer == p2->layer) && (p1->uid < p2->uid));
}

static inline bool CollisionCheck(GameObject* p1, GameObject* p2)LNOEXCEPT
{
	if (!p1->colli || !p2->colli)  // 忽略不碰撞对象
		return false;

	// 快速检测
	if ((p1->x - p1->col_r >= p2->x + p2->col_r) ||
		(p1->x + p1->col_r <= p2->x - p2->col_r) ||
		(p1->y - p1->col_r >= p2->y + p2->col_r) ||
		(p1->y + p1->col_r <= p2->y - p2->col_r))
	{
		return false;
	}

	fcyVec2 pos1((float)p1->x, (float)p1->y), pos2((float)p2->x, (float)p2->y);
	fcyVec2 size1((float)p1->a, (float)p1->b), size2((float)p2->a, (float)p2->b);  // half size
	float r1((float)p1->col_r), r2((float)p2->col_r);

	// 外接圆检查
	if (!CircleHitTest(pos1, r1, pos2, r2))
		return false;

	// 精确碰撞检查
	if (p1->rect)
	{
		if (p2->rect)
			return OBBHitTest(pos1, size1, (float)p1->rot, pos2, size2, (float)p2->rot);
		else
			return OBBCircleHitTest(pos1, size1, (float)p1->rot, pos2, r2);
	}
	else
	{
		if (p2->rect)
			return OBBCircleHitTest(pos2, size2, (float)p2->rot, pos1, r1);
		else
			return true;  // 外接圆检查通过了
	}

	/*
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
	*/
}

////////////////////////////////////////////////////////////////////////////////
/// GameObjectBentLaser
////////////////////////////////////////////////////////////////////////////////
static fcyMemPool<sizeof(GameObjectBentLaser)> s_GameObjectBentLaserPool(1024);

GameObjectBentLaser* GameObjectBentLaser::AllocInstance()
{
	// ! 潜在bad_alloc
	GameObjectBentLaser* pRet = new(s_GameObjectBentLaserPool.Alloc()) GameObjectBentLaser();
	return pRet;
}

void GameObjectBentLaser::FreeInstance(GameObjectBentLaser* p)
{
	p->~GameObjectBentLaser();
	s_GameObjectBentLaserPool.Free(p);
}

GameObjectBentLaser::GameObjectBentLaser()
{
}

GameObjectBentLaser::~GameObjectBentLaser()
{
}

bool GameObjectBentLaser::Update(size_t id, int length, float width)LNOEXCEPT
{
	GameObject* p = LPOOL.GetPooledObject(id);
	if (!p)
		return false;
	if (length <= 1)
	{
		LERROR("lstgBentLaserData: 无效的参数length");
		return false;
	}

	// 移除多余的节点，保证长度在length范围内
	while (m_Queue.IsFull() || m_Queue.Size() >= (size_t)length)
	{
		LaserNode tLastPop;
		m_Queue.Pop(tLastPop);

		// 减少总长度
		if (!m_Queue.IsEmpty())
		{
			LaserNode tFront = m_Queue.Front();
			m_fLength -= (tLastPop.pos - tFront.pos).Length();
		}
	}

	// 添加新节点
	if (m_Queue.Size() < (size_t)length)
	{
		LaserNode tNode;
		tNode.pos.Set((float)p->x, (float)p->y);
		tNode.half_width = width / 2.f;
		m_Queue.Push(tNode);

		// 增加总长度
		if (m_Queue.Size() > 1)
		{
			LaserNode& tNodeLast = m_Queue.Back();
			LaserNode& tNodeBeforeLast = m_Queue[m_Queue.Size() - 2];
			m_fLength += (tNodeBeforeLast.pos - tNodeLast.pos).Length();
		}
	}

	return true;
}

void GameObjectBentLaser::Release()LNOEXCEPT
{
}

bool GameObjectBentLaser::Render(const char* tex_name, BlendMode blend, fcyColor c, float tex_left, float tex_top, float tex_width, float tex_height, float scale)LNOEXCEPT
{
	// 忽略只有一个节点的情况
	if (m_Queue.Size() <= 1)
		return true;

	fcyRefPointer<ResTexture> pTex = LRES.FindTexture(tex_name);
	if (!pTex)
	{
		LERROR("lstgBentLaserData: 找不到纹理资源'%m'", tex_name);
		return false;
	}

	f2dGraphics2DVertex renderVertex[4] = {
		{ 0, 0, 0.5f, c.argb, 0, tex_top },
		{ 0, 0, 0.5f, c.argb, 0, tex_top },
		{ 0, 0, 0.5f, c.argb, 0, tex_top + tex_height },
		{ 0, 0, 0.5f, c.argb, 0, tex_top + tex_height }
	};

	float tVecLength = 0;
	for (size_t i = 0; i < m_Queue.Size() - 1; ++i)
	{
		LaserNode& cur = m_Queue[i];
		LaserNode& next = m_Queue[i + 1];

		// === 计算最左侧的两个点 ===
		// 计算从cur到next的向量
		fcyVec2 offsetA = cur.pos - next.pos;
		float lenOffsetA = offsetA.Length();
		if (lenOffsetA < 0.0001f && i + 1 != m_Queue.Size() - 1)
			continue;

		// 计算宽度上的扩展长度(旋转270度)
		fcyVec2 expandVec = offsetA.GetNormalize();
		std::swap(expandVec.x, expandVec.y);
		expandVec.y = -expandVec.y;

		if (i == 0)  // 如果是第一个节点，则其宽度扩展使用expandVec计算
		{
			float expX = expandVec.x * scale * cur.half_width;
			float expY = expandVec.y * scale * cur.half_width;
			renderVertex[0].x = cur.pos.x + expX;
			renderVertex[0].y = cur.pos.y + expY;
			renderVertex[0].u = tex_left;
			renderVertex[3].x = cur.pos.x - expX;
			renderVertex[3].y = cur.pos.y - expY;
			renderVertex[3].u = tex_left;
		}
		else  // 否则，拷贝1和2
		{
			renderVertex[0].x = renderVertex[1].x;
			renderVertex[0].y = renderVertex[1].y;
			renderVertex[0].u = renderVertex[1].u;
			renderVertex[3].x = renderVertex[2].x;
			renderVertex[3].y = renderVertex[2].y;
			renderVertex[3].u = renderVertex[2].u;
		}

		// === 计算最右侧的两个点 ===
		tVecLength += lenOffsetA;
		if (i == m_Queue.Size() - 2)  // 这是最后两个节点，则其宽度扩展使用expandVec计算
		{
			float expX = expandVec.x * scale * next.half_width;
			float expY = expandVec.y * scale * next.half_width;
			renderVertex[1].x = next.pos.x + expX;
			renderVertex[1].y = next.pos.y + expY;
			renderVertex[1].u = tex_left + tex_width;
			renderVertex[2].x = next.pos.x - expX;
			renderVertex[2].y = next.pos.y - expY;
			renderVertex[2].u = tex_left + tex_width;
		}
		else  // 否则，参考第三个点
		{
			float expX, expY;
			LaserNode& afterNext = m_Queue[i + 2];

			// 计算向量next->afterNext并规范化，相加offsetA和offsetB后得角平分线
			fcyVec2 offsetB = afterNext.pos - next.pos;
			fcyVec2 angleBisect = offsetA.GetNormalize() + offsetB.GetNormalize();
			float angleBisectLen = angleBisect.Length();

			if (angleBisectLen < 0.00002f || angleBisectLen > 1.99998f)  // 几乎在一条直线上
			{
				expX = expandVec.x * scale * next.half_width;
				expY = expandVec.y * scale * next.half_width;
			}
			else // 计算角平分线到角两边距离为next.half_width * scale的偏移量
			{
				angleBisect *= (1 / angleBisectLen);  // angleBisect.Normalize();
				float t = angleBisect * offsetA.GetNormalize();
				float l = scale * next.half_width;
				float expandDelta = sqrt(l * l / (1.f - t * t));
				expX = angleBisect.x * expandDelta;
				expY = angleBisect.y * expandDelta;
			}
			
			// 设置顶点
			float u = tex_left + tVecLength / m_fLength * tex_width;
			renderVertex[1].x = next.pos.x + expX;
			renderVertex[1].y = next.pos.y + expY;
			renderVertex[1].u = u;
			renderVertex[2].x = next.pos.x - expX;
			renderVertex[2].y = next.pos.y - expY;
			renderVertex[2].u = u;

			// 修正交叉的情况
			float cross1 = fcyVec2(renderVertex[1].x - renderVertex[0].x, renderVertex[1].y - renderVertex[0].y) *
				fcyVec2(renderVertex[2].x - renderVertex[3].x, renderVertex[2].y - renderVertex[3].y);
			float cross2 = fcyVec2(renderVertex[2].x - renderVertex[0].x, renderVertex[2].y - renderVertex[0].y) *
				fcyVec2(renderVertex[1].x - renderVertex[3].x, renderVertex[1].y - renderVertex[3].y);
			if (cross2 > cross1)
			{
				std::swap(renderVertex[1].x, renderVertex[2].x);
				std::swap(renderVertex[1].y, renderVertex[2].y);
			}	
		}

		// 绘制这一段
		if (!LAPP.RenderTexture(pTex, blend, renderVertex))
			return false;
	}
	return true;
}

bool GameObjectBentLaser::CollisionCheck(float x, float y, float rot, float a, float b, bool rect)LNOEXCEPT
{
	// 忽略只有一个节点的情况
	if (m_Queue.Size() <= 1)
		return false;

	GameObject testObjA;
	testObjA.Reset();
	testObjA.rot = 0.;
	testObjA.rect = false;

	GameObject testObjB;
	testObjB.Reset();
	testObjB.x = x;
	testObjB.y = y;
	testObjB.rot = rot;
	testObjB.a = a;
	testObjB.b = b;
	testObjB.rect = rect;
	testObjB.UpdateCollisionCirclrRadius();

	for (size_t i = 0; i < m_Queue.Size(); ++i)
	{
		LaserNode& n = m_Queue[i];
		testObjA.x = n.pos.x;
		testObjA.y = n.pos.y;
		testObjA.a = testObjA.b = n.half_width;
		testObjA.UpdateCollisionCirclrRadius();
		if (::CollisionCheck(&testObjA, &testObjB))
			return true;
	}
	return false;
}

bool GameObjectBentLaser::BoundCheck()LNOEXCEPT
{
	fcyRect tBound = LPOOL.GetBound();
	for (size_t i = 0; i < m_Queue.Size(); ++i)
	{
		LaserNode& n = m_Queue[i];
		if (n.pos.x >= tBound.a.x && n.pos.x <= tBound.b.x && n.pos.y <= tBound.a.y && n.pos.y >= tBound.b.y)
			return true;
	}
	// 越界时返回false，只有当所有的弹幕越界才返回false
	return false;
}

////////////////////////////////////////////////////////////////////////////////
/// GameObject
////////////////////////////////////////////////////////////////////////////////
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
		UpdateCollisionCirclrRadius();
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
		UpdateCollisionCirclrRadius();
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
		ps->SetInactive();
		ps->SetCenter(fcyVec2((float)x, (float)y));
		ps->SetRotation((float)rot);
		ps->SetActive();

		res->AddRef();
		a = tParticle->GetHalfSizeX() * LRES.GetGlobalImageScaleFactor();
		b = tParticle->GetHalfSizeY() * LRES.GetGlobalImageScaleFactor();
		rect = tParticle->IsRectangle();
		UpdateCollisionCirclrRadius();
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
		p->vx += p->ax;
		p->vy += p->ay;
		p->x += p->vx;
		p->y += p->vy;
		p->rot += p->omiga;

		// 更新粒子系统（若有）
		if (p->res && p->res->GetType() == ResourceType::Particle)
		{
			float gscale = LRES.GetGlobalImageScaleFactor();
			p->ps->SetRotation((float)p->rot);
			if (p->ps->IsActived())  // 兼容性处理
			{
				p->ps->SetInactive();
				p->ps->SetCenter(fcyVec2((float)p->x, (float)p->y));
				p->ps->SetActive();
			}
			else
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
			if (::CollisionCheck(pA, pB))
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

bool GameObjectPool::GetV(size_t id, double& v, double& a)LNOEXCEPT
{
	GameObject* p = m_ObjectPool.Data(id);
	if (!p)
		return false;
	v = sqrt(p->vx * p->vx + p->vy * p->vy);
	a = atan2(p->vy, p->vx) * LRAD2DEGREE;
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
	case GameObjectProperty::AX:
		lua_pushnumber(L, p->ax);
		break;
	case GameObjectProperty::AY:
		lua_pushnumber(L, p->ay);
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
	case GameObjectProperty::AX:
		p->ax = luaL_checknumber(L, 3);
		break;
	case GameObjectProperty::AY:
		p->ay = luaL_checknumber(L, 3);
		break;
	case GameObjectProperty::LAYER:
		p->layer = luaL_checknumber(L, 3);
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
		p->UpdateCollisionCirclrRadius();
		break;
	case GameObjectProperty::B:
		p->b = luaL_checknumber(L, 3) * LRES.GetGlobalImageScaleFactor();
		p->UpdateCollisionCirclrRadius();
		break;
	case GameObjectProperty::RECT:
		p->rect = lua_toboolean(L, 3) == 0 ? false : true;
		p->UpdateCollisionCirclrRadius();
		break;
	case GameObjectProperty::IMG:
		do
		{
			const char* name = luaL_checkstring(L, 3);
			if (!p->res || strcmp(name, p->res->GetResName().c_str()) != 0)
			{
				p->ReleaseResource();
				if (!p->ChangeResource(name))
					return luaL_error(L, "can't find resource '%s' in image/animation/particle pool.", luaL_checkstring(L, 3));
			}
		} while (false);
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
	p->ps->SetEmission((float)::max(0., luaL_checknumber(L, 2)));
	return 0;
}

void GameObjectPool::DrawGroupCollider(f2dGraphics2D* graph, f2dGeometryRenderer* grender, int groupId, fcyColor fillColor)
{
	GameObject* p = m_pCollisionListHeader[groupId].pCollisionNext;
	GameObject* pTail = &m_pCollisionListTail[groupId];
	while (p && p != pTail)
	{
		if (p->colli)
		{
			if (p->rect)
			{
				fcyVec2 tHalfSize((float)p->a, (float)p->b);

				// 计算出矩形的4个顶点
				f2dGraphics2DVertex tFinalPos[4] =
				{
					{ -tHalfSize.x, -tHalfSize.y, 0.5f, fillColor.argb, 0.0f, 0.0f },
					{ tHalfSize.x, -tHalfSize.y, 0.5f, fillColor.argb, 0.0f, 1.0f },
					{ tHalfSize.x, tHalfSize.y, 0.5f, fillColor.argb, 1.0f, 1.0f },
					{ -tHalfSize.x, tHalfSize.y, 0.5f, fillColor.argb, 1.0f, 0.0f }
				};

				// float tSin = sin(Angle), tCos = cos(Angle);
				float tSin, tCos;
				SinCos((float)p->rot, tSin, tCos);

				// 变换
				for (int i = 0; i < 4; i++)
				{
					fFloat tx = tFinalPos[i].x * tCos - tFinalPos[i].y * tSin,
						ty = tFinalPos[i].x * tSin + tFinalPos[i].y * tCos;
					tFinalPos[i].x = tx + (float)p->x; tFinalPos[i].y = ty + (float)p->y;
				}

				graph->DrawQuad(nullptr, tFinalPos);
			}
			else
			{
				grender->FillCircle(graph, fcyVec2((float)p->x, (float)p->y), (float)p->col_r, fillColor, fillColor,
					p->col_r < 10 ? 3 : (p->col_r < 20 ? 6 : 8));
			}
		}

		p = p->pCollisionNext;
	}
}
