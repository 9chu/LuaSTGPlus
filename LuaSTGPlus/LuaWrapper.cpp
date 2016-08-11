#include "LuaWrapper.h"
#include "AppFrame.h"

#define TYPENAME_COLOR "lstgColor"
#define TYPENAME_RANDGEN "lstgRand"
#define TYPENAME_BENTLASER "lstgBentLaserData"

#ifdef min
#undef min
#endif

#ifdef max
#undef max
#endif

#ifdef LoadImage
#undef LoadImage
#endif

#ifdef PlaySound
#undef PlaySound
#endif

using namespace std;
using namespace LuaSTGPlus;

static inline BlendMode TranslateBlendMode(lua_State* L, int argnum)
{
	const char* s = luaL_checkstring(L, argnum);
	if (strcmp(s, "mul+add") == 0)
		return BlendMode::MulAdd;
	else if (strcmp(s, "") == 0)
		return BlendMode::MulAlpha;
	else if (strcmp(s, "mul+alpha") == 0)
		return BlendMode::MulAlpha;
	else if (strcmp(s, "add+add") == 0)
		return BlendMode::AddAdd;
	else if (strcmp(s, "add+alpha") == 0)
		return BlendMode::AddAlpha;
	else if (strcmp(s, "add+rev") == 0)
		return BlendMode::AddRev;
	else if (strcmp(s, "mul+rev") == 0)
		return BlendMode::MulRev;
	else if (strcmp(s, "add+sub") == 0)
		return BlendMode::AddSub;
	else if (strcmp(s, "mul+sub") == 0)
		return BlendMode::MulSub;
	else
		luaL_error(L, "invalid blend mode '%s'.", s);
	return BlendMode::MulAlpha;
}

static inline void TranslateAlignMode(lua_State* L, int argnum, ResFont::FontAlignHorizontal& halign, ResFont::FontAlignVertical& valign)
{
	int e = luaL_checkinteger(L, argnum);
	switch (e & 0x03)  // HGETEXT_HORZMASK
	{
	case 0:  // HGETEXT_LEFT
		halign = ResFont::FontAlignHorizontal::Left;
		break;
	case 1:  // HGETEXT_CENTER
		halign = ResFont::FontAlignHorizontal::Center;
		break;
	case 2:  // HGETEXT_RIGHT
		halign = ResFont::FontAlignHorizontal::Right;
		break;
	default:
		luaL_error(L, "invalid align mode.");
		return;
	}
	switch (e & 0x0C)  // HGETEXT_VERTMASK
	{
	case 0:  // HGETEXT_TOP
		valign = ResFont::FontAlignVertical::Top;
		break;
	case 4:  // HGETEXT_MIDDLE
		valign = ResFont::FontAlignVertical::Middle;
		break;
	case 8:  // HGETEXT_BOTTOM
		valign = ResFont::FontAlignVertical::Bottom;
		break;
	default:
		luaL_error(L, "invalid align mode.");
		return;
	}
}

////////////////////////////////////////////////////////////////////////////////
/// ColorWrapper
////////////////////////////////////////////////////////////////////////////////
#pragma region ColorWrapper
void ColorWrapper::Register(lua_State* L)LNOEXCEPT
{
	struct WrapperImplement
	{
		static int ARGB(lua_State* L)LNOEXCEPT
		{
			fcyColor* p = static_cast<fcyColor*>(luaL_checkudata(L, 1, TYPENAME_COLOR));
			lua_pushnumber(L, p->a);
			lua_pushnumber(L, p->r);
			lua_pushnumber(L, p->g);
			lua_pushnumber(L, p->b);
			return 4;
		}
		static int Meta_Eq(lua_State* L)LNOEXCEPT
		{
			fcyColor* pA = static_cast<fcyColor*>(luaL_checkudata(L, 1, TYPENAME_COLOR));
			fcyColor* pB = static_cast<fcyColor*>(luaL_checkudata(L, 2, TYPENAME_COLOR));
			lua_pushboolean(L, pA->argb == pB->argb);
			return 1;
		}
		static int Meta_Add(lua_State* L)LNOEXCEPT
		{
			fcyColor* pA = static_cast<fcyColor*>(luaL_checkudata(L, 1, TYPENAME_COLOR));
			fcyColor* pB = static_cast<fcyColor*>(luaL_checkudata(L, 2, TYPENAME_COLOR));
			fcyColor* pResult = CreateAndPush(L);
			pResult->Set(
				::min((int)pA->a + pB->a, 255),
				::min((int)pA->r + pB->r, 255),
				::min((int)pA->g + pB->g, 255),
				::min((int)pA->b + pB->b, 255)
			);
			return 1;
		}
		static int Meta_Sub(lua_State* L)LNOEXCEPT
		{
			fcyColor* pA = static_cast<fcyColor*>(luaL_checkudata(L, 1, TYPENAME_COLOR));
			fcyColor* pB = static_cast<fcyColor*>(luaL_checkudata(L, 2, TYPENAME_COLOR));
			fcyColor* pResult = CreateAndPush(L);
			pResult->Set(
				::max((int)pA->a - pB->a, 0),
				::max((int)pA->r - pB->r, 0),
				::max((int)pA->g - pB->g, 0),
				::max((int)pA->b - pB->b, 0)
			);
			return 1;
		}
		static int Meta_Mul(lua_State* L)LNOEXCEPT
		{
			lua_Number tFactor;
			fcyColor *p = nullptr, *pResult = nullptr;
			if (lua_isnumber(L, 1))  // arg1为数字，则arg2必为lstgColor
			{
				tFactor = luaL_checknumber(L, 1);
				p = static_cast<fcyColor*>(luaL_checkudata(L, 2, TYPENAME_COLOR));
			}
			else if (lua_isnumber(L, 2))  // arg2为数字，则arg1必为lstgColor
			{
				tFactor = luaL_checknumber(L, 2);
				p = static_cast<fcyColor*>(luaL_checkudata(L, 1, TYPENAME_COLOR));
			}
			else  // arg1和arg2都必为lstgColor
			{
				fcyColor* pA = static_cast<fcyColor*>(luaL_checkudata(L, 1, TYPENAME_COLOR));
				fcyColor* pB = static_cast<fcyColor*>(luaL_checkudata(L, 2, TYPENAME_COLOR));
				pResult = CreateAndPush(L);
				pResult->Set(
					::min((int)pA->a * pB->a, 255),
					::min((int)pA->r * pB->r, 255),
					::min((int)pA->g * pB->g, 255),
					::min((int)pA->b * pB->b, 255)
				);
				return 1;
			}
			pResult = CreateAndPush(L);
			pResult->Set(
				(int)::min((double)p->a * tFactor, 255.),
				(int)::min((double)p->r * tFactor, 255.),
				(int)::min((double)p->g * tFactor, 255.),
				(int)::min((double)p->b * tFactor, 255.)
			);
			return 1;
		}
		static int Meta_ToString(lua_State* L)LNOEXCEPT
		{
			fcyColor* p = static_cast<fcyColor*>(luaL_checkudata(L, 1, TYPENAME_COLOR));
			lua_pushfstring(L, "lstg.Color(%d,%d,%d,%d)", p->a, p->r, p->g, p->b);
			return 1;
		}
	};

	luaL_Reg tMethods[] =
	{
		{ "ARGB", &WrapperImplement::ARGB },
		{ NULL, NULL }
	};
	luaL_Reg tMetaTable[] =
	{
		{ "__eq", &WrapperImplement::Meta_Eq },
		{ "__add", &WrapperImplement::Meta_Add },
		{ "__sub", &WrapperImplement::Meta_Sub },
		{ "__mul", &WrapperImplement::Meta_Mul },
		{ "__tostring", &WrapperImplement::Meta_ToString },
		{ NULL, NULL }
	};

	luaL_openlib(L, TYPENAME_COLOR, tMethods, 0);  // t
	luaL_newmetatable(L, TYPENAME_COLOR);  // t mt
	luaL_openlib(L, 0, tMetaTable, 0);  // t mt
	lua_pushliteral(L, "__index");  // t mt s
	lua_pushvalue(L, -3);  // t mt s t
	lua_rawset(L, -3);  // t mt (mt["__index"] = t)
	lua_pushliteral(L, "__metatable");  // t mt s
	lua_pushvalue(L, -3);  // t mt s t
	lua_rawset(L, -3);  // t mt (mt["__metatable"] = t)  保护metatable不被修改
	lua_pop(L, 2);
}

fcyColor* ColorWrapper::CreateAndPush(lua_State* L)
{
	fcyColor* p = static_cast<fcyColor*>(lua_newuserdata(L, sizeof(fcyColor)));
	new(p) fcyColor();  // 构造
	luaL_getmetatable(L, TYPENAME_COLOR);
	lua_setmetatable(L, -2);
	return p;
}
#pragma endregion

////////////////////////////////////////////////////////////////////////////////
/// RandomizerWrapper
////////////////////////////////////////////////////////////////////////////////
#pragma region RandomizerWrapper
void RandomizerWrapper::Register(lua_State* L)LNOEXCEPT
{
	struct WrapperImplement
	{
		static int Seed(lua_State* L)LNOEXCEPT
		{
			fcyRandomWELL512* p = static_cast<fcyRandomWELL512*>(luaL_checkudata(L, 1, TYPENAME_RANDGEN));
			p->SetSeed((fuInt)luaL_checknumber(L, 2));
			return 0;
		}
		static int GetSeed(lua_State* L)LNOEXCEPT
		{
			fcyRandomWELL512* p = static_cast<fcyRandomWELL512*>(luaL_checkudata(L, 1, TYPENAME_RANDGEN));
			lua_pushnumber(L, (lua_Number)p->GetRandSeed());
			return 1;
		}
		static int Int(lua_State* L)LNOEXCEPT
		{
			fcyRandomWELL512* p = static_cast<fcyRandomWELL512*>(luaL_checkudata(L, 1, TYPENAME_RANDGEN));
			int a = luaL_checkinteger(L, 2), b = luaL_checkinteger(L, 3);
			lua_pushinteger(L, a + static_cast<fInt>(p->GetRandUInt(::max(static_cast<fuInt>(b - a), 0U))));
			return 1;
		}
		static int Float(lua_State* L)LNOEXCEPT
		{
			fcyRandomWELL512* p = static_cast<fcyRandomWELL512*>(luaL_checkudata(L, 1, TYPENAME_RANDGEN));
			double a = luaL_checknumber(L, 2), b = luaL_checknumber(L, 3);
			lua_pushnumber(L, p->GetRandFloat((float)a, (float)b));
			return 1;
		}
		static int Sign(lua_State* L)LNOEXCEPT
		{
			fcyRandomWELL512* p = static_cast<fcyRandomWELL512*>(luaL_checkudata(L, 1, TYPENAME_RANDGEN));
			lua_pushinteger(L, p->GetRandUInt(1) * 2 - 1);
			return 1;
		}
		static int Meta_ToString(lua_State* L)LNOEXCEPT
		{
			fcyRandomWELL512* p = static_cast<fcyRandomWELL512*>(luaL_checkudata(L, 1, TYPENAME_RANDGEN));
			lua_pushfstring(L, "lstg.Rand object");
			return 1;
		}
	};

	luaL_Reg tMethods[] =
	{
		{ "Seed", &WrapperImplement::Seed },
		{ "GetSeed", &WrapperImplement::GetSeed },
		{ "Int", &WrapperImplement::Int },
		{ "Float", &WrapperImplement::Float },
		{ "Sign", &WrapperImplement::Sign },
		{ NULL, NULL }
	};
	luaL_Reg tMetaTable[] =
	{
		{ "__tostring", &WrapperImplement::Meta_ToString },
		{ NULL, NULL }
	};

	luaL_openlib(L, TYPENAME_RANDGEN, tMethods, 0);  // t
	luaL_newmetatable(L, TYPENAME_RANDGEN);  // t mt
	luaL_openlib(L, 0, tMetaTable, 0);  // t mt
	lua_pushliteral(L, "__index");  // t mt s
	lua_pushvalue(L, -3);  // t mt s t
	lua_rawset(L, -3);  // t mt (mt["__index"] = t)
	lua_pushliteral(L, "__metatable");  // t mt s
	lua_pushvalue(L, -3);  // t mt s t
	lua_rawset(L, -3);  // t mt (mt["__metatable"] = t)  保护metatable不被修改
	lua_pop(L, 2);
}

fcyRandomWELL512* RandomizerWrapper::CreateAndPush(lua_State* L)
{
	fcyRandomWELL512* p = static_cast<fcyRandomWELL512*>(lua_newuserdata(L, sizeof(fcyRandomWELL512)));
	new(p) fcyRandomWELL512();  // 构造
	luaL_getmetatable(L, TYPENAME_RANDGEN);
	lua_setmetatable(L, -2);
	return p;
}
#pragma endregion

////////////////////////////////////////////////////////////////////////////////
/// BentLaserDataWrapper
////////////////////////////////////////////////////////////////////////////////
#pragma region BentLaserDataWrapper
void BentLaserDataWrapper::Register(lua_State* L)LNOEXCEPT
{
	struct WrapperImplement
	{
		static int Update(lua_State* L)LNOEXCEPT
		{
			Wrapper* p = static_cast<Wrapper*>(luaL_checkudata(L, 1, TYPENAME_BENTLASER));
			if (!p->handle)
				return luaL_error(L, "lstgBentLaserData was released.");
			if (!lua_istable(L, 2))
				return luaL_error(L, "invalid lstg object for 'Update'.");
			lua_rawgeti(L, 2, 2);  // self t(object) ??? id
			size_t id = (size_t)luaL_checkinteger(L, -1);
			lua_pop(L, 1);
			if (!p->handle->Update(id, luaL_checkinteger(L, 3), (float)luaL_checknumber(L, 4)))
				return luaL_error(L, "invalid lstg object for 'Update'.");
			return 0;
		}
		static int Release(lua_State* L)LNOEXCEPT
		{
			Wrapper* p = static_cast<Wrapper*>(luaL_checkudata(L, 1, TYPENAME_BENTLASER));
			if (p->handle)
			{
				GameObjectBentLaser::FreeInstance(p->handle);
				p->handle = nullptr;
			}
			return 0;
		}
		static int Render(lua_State* L)LNOEXCEPT
		{
			Wrapper* p = static_cast<Wrapper*>(luaL_checkudata(L, 1, TYPENAME_BENTLASER));
			if (!p->handle)
				return luaL_error(L, "lstgBentLaserData was released.");
			if (!p->handle->Render(
				luaL_checkstring(L, 2),
				TranslateBlendMode(L, 3),
				*static_cast<fcyColor*>(luaL_checkudata(L, 4, TYPENAME_COLOR)),
				(float)luaL_checknumber(L, 5),
				(float)luaL_checknumber(L, 6),
				(float)luaL_checknumber(L, 7),
				(float)luaL_checknumber(L, 8),
				(float)luaL_optnumber(L, 9, 1.) * LRES.GetGlobalImageScaleFactor()
				))
			{
				return luaL_error(L, "can't render object with texture '%s'.", luaL_checkstring(L, 2));
			}
			return 0;
		}
		static int CollisionCheck(lua_State* L)LNOEXCEPT
		{
			Wrapper* p = static_cast<Wrapper*>(luaL_checkudata(L, 1, TYPENAME_BENTLASER));
			if (!p->handle)
				return luaL_error(L, "lstgBentLaserData was released.");
			bool r = p->handle->CollisionCheck(
				(float)luaL_checknumber(L, 2),
				(float)luaL_checknumber(L, 3),
				(float)luaL_optnumber(L, 4, 0),
				(float)luaL_optnumber(L, 5, 0),
				(float)luaL_optnumber(L, 6, 0),
				lua_toboolean(L, 7) == 0 ? false : true
				);
			lua_pushboolean(L, r);
			return 1;
		}
		static int BoundCheck(lua_State* L)LNOEXCEPT
		{
			Wrapper* p = static_cast<Wrapper*>(luaL_checkudata(L, 1, TYPENAME_BENTLASER));
			if (!p->handle)
				return luaL_error(L, "lstgBentLaserData was released.");
			bool r = p->handle->BoundCheck();
			lua_pushboolean(L, r);
			return 1;
		}
		static int Meta_ToString(lua_State* L)LNOEXCEPT
		{
			Wrapper* p = static_cast<Wrapper*>(luaL_checkudata(L, 1, TYPENAME_BENTLASER));
			lua_pushfstring(L, "lstg.BentLaserData object");
			return 1;
		}
		static int Meta_GC(lua_State* L)LNOEXCEPT
		{
			Wrapper* p = static_cast<Wrapper*>(luaL_checkudata(L, 1, TYPENAME_BENTLASER));
			if (p->handle)
			{
				GameObjectBentLaser::FreeInstance(p->handle);
				p->handle = nullptr;
			}
			return 0;
		}
	};

	luaL_Reg tMethods[] =
	{
		{ "Update", &WrapperImplement::Update },
		{ "Release", &WrapperImplement::Release },
		{ "Render", &WrapperImplement::Render },
		{ "CollisionCheck", &WrapperImplement::CollisionCheck },
		{ "BoundCheck", &WrapperImplement::BoundCheck },
		{ NULL, NULL }
	};
	luaL_Reg tMetaTable[] =
	{
		{ "__tostring", &WrapperImplement::Meta_ToString },
		{ "__gc", &WrapperImplement::Meta_GC },
		{ NULL, NULL }
	};

	luaL_openlib(L, TYPENAME_BENTLASER, tMethods, 0);  // t
	luaL_newmetatable(L, TYPENAME_BENTLASER);  // t mt
	luaL_openlib(L, 0, tMetaTable, 0);  // t mt
	lua_pushliteral(L, "__index");  // t mt s
	lua_pushvalue(L, -3);  // t mt s t
	lua_rawset(L, -3);  // t mt (mt["__index"] = t)
	lua_pushliteral(L, "__metatable");  // t mt s
	lua_pushvalue(L, -3);  // t mt s t
	lua_rawset(L, -3);  // t mt (mt["__metatable"] = t)  保护metatable不被修改
	lua_pop(L, 2);
}

GameObjectBentLaser* BentLaserDataWrapper::CreateAndPush(lua_State* L)
{
	Wrapper* p = static_cast<Wrapper*>(lua_newuserdata(L, sizeof(Wrapper)));
	p->handle = GameObjectBentLaser::AllocInstance();
	luaL_getmetatable(L, TYPENAME_BENTLASER);
	lua_setmetatable(L, -2);
	return p->handle;
}
#pragma endregion

////////////////////////////////////////////////////////////////////////////////
/// BuiltInFunctionWrapper
////////////////////////////////////////////////////////////////////////////////
#pragma region BuiltInFunctionWrapper
void BuiltInFunctionWrapper::Register(lua_State* L)LNOEXCEPT
{
	struct WrapperImplement
	{
		// 框架函数
		static int SetWindowed(lua_State* L)LNOEXCEPT
		{
			LAPP.SetWindowed(lua_toboolean(L, 1) == 0 ? false : true);
			return 0;
		}
		static int SetFPS(lua_State* L)LNOEXCEPT
		{
			int v = luaL_checkinteger(L, 1);
			if (v <= 0)
				v = 60;
			LAPP.SetFPS(static_cast<fuInt>(v));
			return 0;
		}
		static int GetFPS(lua_State* L)LNOEXCEPT
		{
			lua_pushnumber(L, LAPP.GetFPS());
			return 1;
		}
		static int SetVsync(lua_State* L)LNOEXCEPT
		{
			LAPP.SetVsync(lua_toboolean(L, 1) == 0 ? false : true);
			return 0;
		}
		static int SetResolution(lua_State* L)LNOEXCEPT
		{
			LAPP.SetResolution(static_cast<fuInt>(::max(luaL_checkinteger(L, 1), 0)),
				static_cast<fuInt>(::max(luaL_checkinteger(L, 2), 0)));
			return 0;
		}
		static int ChangeVideoMode(lua_State* L)LNOEXCEPT
		{
			lua_pushboolean(L, LAPP.ChangeVideoMode(
				luaL_checkinteger(L, 1),
				luaL_checkinteger(L, 2),
				lua_toboolean(L, 3) == 0 ? false : true,
				lua_toboolean(L, 4) == 0 ? false : true
			));
			return 1;
		}
		static int SetSplash(lua_State* L)LNOEXCEPT
		{
			LAPP.SetSplash(lua_toboolean(L, 1) == 0 ? false : true);
			return 0;
		}
		static int SetTitle(lua_State* L)LNOEXCEPT
		{
			LAPP.SetTitle(luaL_checkstring(L, 1));
			return 0;
		}
		static int SystemLog(lua_State* L)LNOEXCEPT
		{
			LINFO("脚本日志：%m", luaL_checkstring(L, 1));
			return 0;
		}
		static int Print(lua_State* L)LNOEXCEPT
		{
			int n = lua_gettop(L);
			lua_getglobal(L, "tostring"); // ... f
			lua_pushstring(L, ""); // ... f s
			for (int i = 1; i <= n; i++)
			{
				if (i > 1)
				{
					lua_pushstring(L, "\t"); // ... f s s
					lua_concat(L, 2); // ... f s
				}
				lua_pushvalue(L, -2); // ... f s f
				lua_pushvalue(L, i); // ... f s f arg[i]
				lua_call(L, 1, 1); // ... f s ret
				const char* x = luaL_checkstring(L, -1);
				lua_concat(L, 2); // ... f s
			}
			LINFO("脚本日志：%m", luaL_checkstring(L, -1));
			lua_pop(L, 2);
			return 0;
		}
		static int LoadPack(lua_State* L)LNOEXCEPT
		{
			const char* p = luaL_checkstring(L, 1);
			const char* pwd = nullptr;
			if (lua_isstring(L, 2))
				pwd = luaL_checkstring(L, 2);
			if (!LRES.LoadPack(p, pwd))
				return luaL_error(L, "failed to load resource pack '%s'.", p);
			return 0;
		}
		static int UnloadPack(lua_State* L)LNOEXCEPT
		{
			const char* p = luaL_checkstring(L, 1);
			LRES.UnloadPack(p);
			return 0;
		}
		static int ExtractRes(lua_State* L)LNOEXCEPT
		{
			const char* pArgPath = luaL_checkstring(L, 1);
			const char* pArgTarget = luaL_checkstring(L, 2);
			if (!LRES.ExtractRes(pArgPath, pArgTarget))
				return luaL_error(L, "failed to extract resource '%s' to '%s'.", pArgPath, pArgTarget);
			return 0;
		}
		static int DoFile(lua_State* L)LNOEXCEPT
		{
			LAPP.LoadScript(luaL_checkstring(L, 1));
			return 0;
		}
		static int ShowSplashWindow(lua_State* L)LNOEXCEPT
		{
			if (lua_gettop(L) == 0)
				LAPP.ShowSplashWindow();
			else
				LAPP.ShowSplashWindow(luaL_checkstring(L, 1));
			return 0;
		}

		// 对象控制函数（这些方法将被转发到对象池）
		static int GetnObj(lua_State* L)LNOEXCEPT
		{
			lua_pushinteger(L, (lua_Integer)LPOOL.GetObjectCount());
			return 1;
		}
		static int UpdateObjList(lua_State* L)LNOEXCEPT
		{
			// ! 该函数已被否决
			return 0;
		}
		static int ObjFrame(lua_State* L)LNOEXCEPT
		{
			LPOOL.CheckIsMainThread(L);
			LPOOL.DoFrame();
			return 0;
		}
		static int ObjRender(lua_State* L)LNOEXCEPT
		{
			LPOOL.CheckIsMainThread(L);
			LPOOL.DoRender();
			return 0;
		}
		static int BoundCheck(lua_State* L)LNOEXCEPT
		{
			LPOOL.CheckIsMainThread(L);
			LPOOL.BoundCheck();
			return 0;
		}
		static int SetBound(lua_State* L)LNOEXCEPT
		{
			LPOOL.SetBound(
				luaL_checkinteger(L, 1),
				luaL_checkinteger(L, 2),
				luaL_checkinteger(L, 3),
				luaL_checkinteger(L, 4)
			);
			return 0;
		}
		static int CollisionCheck(lua_State* L)LNOEXCEPT
		{
			LPOOL.CheckIsMainThread(L);
			LPOOL.CollisionCheck(luaL_checkinteger(L, 1), luaL_checkinteger(L, 2));
			return 0;
		}
		static int UpdateXY(lua_State* L)LNOEXCEPT
		{
			LPOOL.CheckIsMainThread(L);
			LPOOL.UpdateXY();
			return 0;
		}
		static int AfterFrame(lua_State* L)LNOEXCEPT
		{
			LPOOL.CheckIsMainThread(L);
			LPOOL.AfterFrame();
			return 0;
		}
		static int New(lua_State* L)LNOEXCEPT
		{
			return LPOOL.New(L);
		}
		static int Del(lua_State* L)LNOEXCEPT
		{
			return LPOOL.Del(L);
		}
		static int Kill(lua_State* L)LNOEXCEPT
		{
			return LPOOL.Kill(L);
		}
		static int IsValid(lua_State* L)LNOEXCEPT
		{
			return LPOOL.IsValid(L);
		}
		static int Angle(lua_State* L)LNOEXCEPT
		{
			if (lua_gettop(L) == 2)
			{
				if (!lua_istable(L, 1) || !lua_istable(L, 2))
					return luaL_error(L, "invalid lstg object for 'Angle'.");
				lua_rawgeti(L, 1, 2);  // t(object) t(object) ??? id
				lua_rawgeti(L, 2, 2);  // t(object) t(object) ??? id id
				double tRet;
				if (!LPOOL.Angle((size_t)luaL_checkint(L, -2), (size_t)luaL_checkint(L, -1), tRet))
					return luaL_error(L, "invalid lstg object for 'Angle'.");
				lua_pushnumber(L, tRet);
			}
			else
			{
				lua_pushnumber(L, 
					atan2(luaL_checknumber(L, 4) - luaL_checknumber(L, 2), luaL_checknumber(L, 3) - luaL_checknumber(L, 1)) * LRAD2DEGREE
				);
			}
			return 1;
		}
		static int Dist(lua_State* L)LNOEXCEPT
		{
			if (lua_gettop(L) == 2)
			{
				if (!lua_istable(L, 1) || !lua_istable(L, 2))
					return luaL_error(L, "invalid lstg object for 'Dist'.");
				lua_rawgeti(L, 1, 2);  // t(object) t(object) id
				lua_rawgeti(L, 2, 2);  // t(object) t(object) id id
				double tRet;
				if (!LPOOL.Dist((size_t)luaL_checkint(L, -2), (size_t)luaL_checkint(L, -1), tRet))
					return luaL_error(L, "invalid lstg object for 'Dist'.");
				lua_pushnumber(L, tRet);
			}
			else
			{
				lua_Number dx = luaL_checknumber(L, 3) - luaL_checknumber(L, 1);
				lua_Number dy = luaL_checknumber(L, 4) - luaL_checknumber(L, 2);
				lua_pushnumber(L, sqrt(dx*dx + dy*dy));
			}
			return 1;
		}
		static int GetV(lua_State* L)LNOEXCEPT
		{
			if (!lua_istable(L, 1))
				return luaL_error(L, "invalid lstg object for 'GetV'.");
			double v, a;
			lua_rawgeti(L, 1, 2);  // t(object) ??? id
			if (!LPOOL.GetV((size_t)luaL_checkinteger(L, -1), v, a))
				return luaL_error(L, "invalid lstg object for 'GetV'.");
			lua_pushnumber(L, v);
			lua_pushnumber(L, a);
			return 2;
		}
		static int SetV(lua_State* L)LNOEXCEPT
		{
			if (!lua_istable(L, 1))
				return luaL_error(L, "invalid lstg object for 'SetV'.");
			if (lua_gettop(L) == 3)
			{
				lua_rawgeti(L, 1, 2);  // t(object) 'v' 'a' ??? id
				if (!LPOOL.SetV((size_t)luaL_checkinteger(L, -1), luaL_checknumber(L, 2), luaL_checknumber(L, 3), false))
					return luaL_error(L, "invalid lstg object for 'SetV'.");
			}
			else if (lua_gettop(L) == 4)
			{
				lua_rawgeti(L, 1, 2);  // t(object) 'v' 'a' 'rot' ??? id
				if (!LPOOL.SetV((size_t)luaL_checkinteger(L, -1), luaL_checknumber(L, 2), luaL_checknumber(L, 3), lua_toboolean(L, 4) == 0 ? false : true))
					return luaL_error(L, "invalid lstg object for 'SetV'.");
			}
			else
				return luaL_error(L, "invalid argument count for 'SetV'.");
			return 0;
		}
		static int SetImgState(lua_State* L)LNOEXCEPT
		{
			if (!lua_istable(L, 1))
				return luaL_error(L, "invalid lstg object for 'SetImgState'.");
			lua_rawgeti(L, 1, 2);  // t(object) ??? id
			size_t id = (size_t)luaL_checkinteger(L, -1);
			lua_pop(L, 1);

			BlendMode m = TranslateBlendMode(L, 2);
			fcyColor c(luaL_checkinteger(L, 3), luaL_checkinteger(L, 4), luaL_checkinteger(L, 5), luaL_checkinteger(L, 6));
			if (!LPOOL.SetImgState(id, m, c))
				return luaL_error(L, "invalid lstg object for 'SetImgState'.");
			return 0;
		}
		static int BoxCheck(lua_State* L)LNOEXCEPT
		{
			if (!lua_istable(L, 1))
				return luaL_error(L, "invalid lstg object for 'BoxCheck'.");
			lua_rawgeti(L, 1, 2);  // t(object) 'l' 'r' 't' 'b' ??? id
			bool tRet;
			if (!LPOOL.BoxCheck(
				(size_t)luaL_checkinteger(L, -1),
				luaL_checknumber(L, 2),
				luaL_checknumber(L, 3),
				luaL_checknumber(L, 4),
				luaL_checknumber(L, 5),
				tRet))
			{
				return luaL_error(L, "invalid lstg object for 'BoxCheck'.");
			}	
			lua_pushboolean(L, tRet);
			return 1;
		}
		static int ResetPool(lua_State* L)LNOEXCEPT
		{
			LPOOL.ResetPool();
			return 0;
		}
		static int DefaultRenderFunc(lua_State* L)LNOEXCEPT
		{
			if (!lua_istable(L, 1))
				return luaL_error(L, "invalid lstg object for 'DefaultRenderFunc'.");
			lua_rawgeti(L, 1, 2);  // t(object) ??? id
			if (!LPOOL.DoDefaultRender(luaL_checkinteger(L, -1)))
				return luaL_error(L, "invalid lstg object for 'DefaultRenderFunc'.");
			return 0;
		}
		static int NextObject(lua_State* L)LNOEXCEPT
		{
			return LPOOL.NextObject(L);
		}
		static int ObjList(lua_State* L)LNOEXCEPT
		{
			int g = luaL_checkinteger(L, 1);  // i(groupId)
			lua_pushcfunction(L, WrapperImplement::NextObject);
			lua_pushinteger(L, g);
			lua_pushinteger(L, LPOOL.FirstObject(g));
			return 3;
		}
		static int ObjMetaIndex(lua_State* L)LNOEXCEPT
		{
			return LPOOL.GetAttr(L);
		}
		static int ObjMetaNewIndex(lua_State* L)LNOEXCEPT
		{
			return LPOOL.SetAttr(L);
		}
		static int ParticleStop(lua_State* L)LNOEXCEPT
		{
			return LPOOL.ParticleStop(L);
		}
		static int ParticleFire(lua_State* L)LNOEXCEPT
		{
			return LPOOL.ParticleFire(L);
		}
		static int ParticleGetn(lua_State* L)LNOEXCEPT
		{
			return LPOOL.ParticleGetn(L);
		}
		static int ParticleGetEmission(lua_State* L)LNOEXCEPT
		{
			return LPOOL.ParticleGetEmission(L);
		}
		static int ParticleSetEmission(lua_State* L)LNOEXCEPT
		{
			return LPOOL.ParticleSetEmission(L);
		}

		// 资源控制函数
		static int SetResourceStatus(lua_State* L)LNOEXCEPT
		{
			const char* s = luaL_checkstring(L, 1);
			if (strcmp(s, "global") == 0)
				LRES.SetActivedPoolType(ResourcePoolType::Global);
			else if (strcmp(s, "stage") == 0)
				LRES.SetActivedPoolType(ResourcePoolType::Stage);
			else if (strcmp(s, "none") == 0)
				LRES.SetActivedPoolType(ResourcePoolType::None);
			else
				return luaL_error(L, "invalid argument #1 for 'SetResourceStatus', requires 'stage', 'global' or 'none'.");
			return 0;
		}
		static int LoadTexture(lua_State* L)LNOEXCEPT
		{
			const char* name = luaL_checkstring(L, 1);
			const char* path = luaL_checkstring(L, 2);

			ResourcePool* pActivedPool = LRES.GetActivedPool();
			if (!pActivedPool)
				return luaL_error(L, "can't load resource at this time.");
			if (!pActivedPool->LoadTexture(name, path, lua_toboolean(L, 3) == 0 ? false : true))
				return luaL_error(L, "can't load texture from file '%s'.", path);
			return 0;
		}
		static int LoadImage(lua_State* L)LNOEXCEPT
		{
			const char* name = luaL_checkstring(L, 1);
			const char* texname = luaL_checkstring(L, 2);

			ResourcePool* pActivedPool = LRES.GetActivedPool();
			if (!pActivedPool)
				return luaL_error(L, "can't load resource at this time.");

			if (!pActivedPool->LoadImage(
				name,
				texname,
				luaL_checknumber(L, 3),
				luaL_checknumber(L, 4),
				luaL_checknumber(L, 5),
				luaL_checknumber(L, 6),
				luaL_optnumber(L, 7, 0.),
				luaL_optnumber(L, 8, 0.),
				lua_toboolean(L, 9) == 0 ? false : true
			))
			{
				return luaL_error(L, "load image failed (name='%s', tex='%s').", name, texname);
			}
			return 0;
		}
		static int LoadAnimation(lua_State* L)LNOEXCEPT
		{
			const char* name = luaL_checkstring(L, 1);
			const char* texname = luaL_checkstring(L, 2);

			ResourcePool* pActivedPool = LRES.GetActivedPool();
			if (!pActivedPool)
				return luaL_error(L, "can't load resource at this time.");
			
			if (!pActivedPool->LoadAnimation(
				name,
				texname,
				luaL_checknumber(L, 3),
				luaL_checknumber(L, 4),
				luaL_checknumber(L, 5),
				luaL_checknumber(L, 6),
				luaL_checkinteger(L, 7),
				luaL_checkinteger(L, 8),
				luaL_checkinteger(L, 9),
				luaL_optnumber(L, 10, 0.0f),
				luaL_optnumber(L, 11, 0.0f),
				lua_toboolean(L, 12) == 0 ? false : true
			))
			{
				return luaL_error(L, "load animation failed (name='%s', tex='%s').", name, texname);
			}

			return 0;
		}
		static int LoadPS(lua_State* L)LNOEXCEPT
		{
			const char* name = luaL_checkstring(L, 1);
			const char* path = luaL_checkstring(L, 2);
			const char* img_name = luaL_checkstring(L, 3);

			ResourcePool* pActivedPool = LRES.GetActivedPool();
			if (!pActivedPool)
				return luaL_error(L, "can't load resource at this time.");

			if (!pActivedPool->LoadParticle(
				name,
				path,
				img_name,
				luaL_optnumber(L, 4, 0.0f),
				luaL_optnumber(L, 5, 0.0f),
				lua_toboolean(L, 6) == 0 ? false : true
			))
			{
				return luaL_error(L, "load particle failed (name='%s', file='%s', img='%s').", name, path, img_name);
			}

			return 0;
		}
		static int LoadSound(lua_State* L)LNOEXCEPT
		{
			const char* name = luaL_checkstring(L, 1);
			const char* path = luaL_checkstring(L, 2);

			ResourcePool* pActivedPool = LRES.GetActivedPool();
			if (!pActivedPool)
				return luaL_error(L, "can't load resource at this time.");

			if (!pActivedPool->LoadSound(name, path))
				return luaL_error(L, "load sound failed (name=%s, path=%s)", name, path);
			return 0;
		}
		static int LoadMusic(lua_State* L)LNOEXCEPT
		{
			const char* name = luaL_checkstring(L, 1);
			const char* path = luaL_checkstring(L, 2);

			ResourcePool* pActivedPool = LRES.GetActivedPool();
			if (!pActivedPool)
				return luaL_error(L, "can't load resource at this time.");

			double loop_end = luaL_checknumber(L, 3);
			double loop_duration = luaL_checknumber(L, 4);
			double loop_start = max(0., loop_end - loop_duration);

			if (!pActivedPool->LoadMusic(
				name,
				path,
				loop_start,
				loop_end
				))
			{
				return luaL_error(L, "load music failed (name=%s, path=%s, loop=%f~%f)", name, path, loop_start, loop_end);
			}
			return 0;
		}
		static int LoadFont(lua_State* L)LNOEXCEPT
		{
			bool bSucceed = false;
			const char* name = luaL_checkstring(L, 1);
			const char* path = luaL_checkstring(L, 2);

			ResourcePool* pActivedPool = LRES.GetActivedPool();
			if (!pActivedPool)
				return luaL_error(L, "can't load resource at this time.");

			if (lua_gettop(L) == 2)
			{
				// HGE字体 mipmap=true
				bSucceed = pActivedPool->LoadSpriteFont(name, path);
			}
			else
			{
				if (lua_isboolean(L, 3))
				{
					// HGE字体 mipmap=user_defined
					bSucceed = pActivedPool->LoadSpriteFont(name, path, lua_toboolean(L, 3) == 0 ? false : true);
				}
				else
				{
					// fancy2d字体
					const char* texpath = luaL_checkstring(L, 3);
					if (lua_gettop(L) == 4)
						bSucceed = pActivedPool->LoadSpriteFont(name, path, texpath, lua_toboolean(L, 4) == 0 ? false : true);
					else
						bSucceed = pActivedPool->LoadSpriteFont(name, path, texpath);
				}
			}

			if (!bSucceed)
				return luaL_error(L, "can't load font from file '%s'.", path);
			return 0;
		}
		static int LoadTTF(lua_State* L)LNOEXCEPT
		{
			const char* name = luaL_checkstring(L, 1);
			const char* path = luaL_checkstring(L, 2);

			ResourcePool* pActivedPool = LRES.GetActivedPool();
			if (!pActivedPool)
				return luaL_error(L, "can't load resource at this time.");

			if (!pActivedPool->LoadTTFFont(name, path, (float)luaL_checknumber(L, 3), (float)luaL_checknumber(L, 4)))
				return luaL_error(L, "load ttf font failed (name=%s, path=%s)", name, path);
			return 0;
		}
		static int LoadFX(lua_State* L)LNOEXCEPT
		{
			const char* name = luaL_checkstring(L, 1);
			const char* path = luaL_checkstring(L, 2);

			ResourcePool* pActivedPool = LRES.GetActivedPool();
			if (!pActivedPool)
				return luaL_error(L, "can't load resource at this time.");

			if (!pActivedPool->LoadFX(name, path))
				return luaL_error(L, "load fx failed (name=%s, path=%s)", name, path);
			return 0;
		}
		static int CreateRenderTarget(lua_State* L)LNOEXCEPT
		{
			const char* name = luaL_checkstring(L, 1);

			ResourcePool* pActivedPool = LRES.GetActivedPool();
			if (!pActivedPool)
				return luaL_error(L, "can't load resource at this time.");

			if (!pActivedPool->CreateRenderTarget(name))
				return luaL_error(L, "can't create render target with name '%s'.", name);
			return 0;
		}
		static int IsRenderTarget(lua_State* L)LNOEXCEPT
		{
			ResTexture* p = LRES.FindTexture(luaL_checkstring(L, 1));
			if (!p)
				return luaL_error(L, "render target '%s' not found.", luaL_checkstring(L, 1));
			lua_pushboolean(L, p->IsRenderTarget());
			return 1;
		}
		static int GetTextureSize(lua_State* L)LNOEXCEPT
		{
			const char* name = luaL_checkstring(L, 1);
			fcyVec2 size;
			if (!LRES.GetTextureSize(name, size))
				return luaL_error(L, "texture '%s' not found.", name);
			lua_pushnumber(L, size.x);
			lua_pushnumber(L, size.y);
			return 2;
		}
		static int RemoveResource(lua_State* L)LNOEXCEPT
		{
			ResourcePoolType t;
			const char* s = luaL_checkstring(L, 1);
			if (strcmp(s, "global") == 0)
				t = ResourcePoolType::Global;
			else if (strcmp(s, "stage") == 0)
				t = ResourcePoolType::Stage;
			else if (strcmp(s, "none") != 0)
				t = ResourcePoolType::None;
			else
				return luaL_error(L, "invalid argument #1 for 'RemoveResource', requires 'stage', 'global' or 'none'.");

			if (lua_gettop(L) == 1)
			{
				switch (t)
				{
				case ResourcePoolType::Stage:
					LRES.GetResourcePool(ResourcePoolType::Stage)->Clear();
					LINFO("关卡资源池已清空");
					break;
				case ResourcePoolType::Global:
					LRES.GetResourcePool(ResourcePoolType::Global)->Clear();
					LINFO("全局资源池已清空");
					break;
				default:
					break;
				}
			}
			else
			{
				ResourceType tResourceType = static_cast<ResourceType>(luaL_checkint(L, 2));
				const char* tResourceName = luaL_checkstring(L, 3);

				switch (t)
				{
				case ResourcePoolType::Stage:
					LRES.GetResourcePool(ResourcePoolType::Stage)->RemoveResource(tResourceType, tResourceName);
					break;
				case ResourcePoolType::Global:
					LRES.GetResourcePool(ResourcePoolType::Global)->RemoveResource(tResourceType, tResourceName);
					break;
				default:
					break;
				}
			}
			
			return 0;
		}
		static int CheckRes(lua_State* L)LNOEXCEPT
		{
			ResourceType tResourceType = static_cast<ResourceType>(luaL_checkint(L, 1));
			const char* tResourceName = luaL_checkstring(L, 2);
			// 先在全局池中寻找再到关卡池中找
			if (LRES.GetResourcePool(ResourcePoolType::Global)->CheckResourceExists(tResourceType, tResourceName))
				lua_pushstring(L, "global");
			else if (LRES.GetResourcePool(ResourcePoolType::Stage)->CheckResourceExists(tResourceType, tResourceName))
				lua_pushstring(L, "stage");
			else
				lua_pushnil(L);
			return 1;
		}
		static int EnumRes(lua_State* L)LNOEXCEPT
		{
			ResourceType tResourceType = static_cast<ResourceType>(luaL_checkint(L, 1));
			LRES.GetResourcePool(ResourcePoolType::Global)->ExportResourceList(L, tResourceType);
			LRES.GetResourcePool(ResourcePoolType::Stage)->ExportResourceList(L, tResourceType);
			return 2;
		}
		static int SetImageScale(lua_State* L)LNOEXCEPT
		{
			float x = static_cast<float>(luaL_checknumber(L, 1));
			if (x == 0.f)
				return luaL_error(L, "invalid argument #1 for 'SetImageScale'.");
			LRES.SetGlobalImageScaleFactor(x);
			return 0;
		}
		static int SetImageState(lua_State* L)LNOEXCEPT
		{
			ResSprite* p = LRES.FindSprite(luaL_checkstring(L, 1));
			if (!p)
				return luaL_error(L, "image '%s' not found.", luaL_checkstring(L, 1));

			p->SetBlendMode(TranslateBlendMode(L, 2));
			if (lua_gettop(L) == 3)
				p->GetSprite()->SetColor(*static_cast<fcyColor*>(luaL_checkudata(L, 3, TYPENAME_COLOR)));
			else if (lua_gettop(L) == 6)
			{
				fcyColor tColors[] = {
					*static_cast<fcyColor*>(luaL_checkudata(L, 3, TYPENAME_COLOR)),
					*static_cast<fcyColor*>(luaL_checkudata(L, 4, TYPENAME_COLOR)),
					*static_cast<fcyColor*>(luaL_checkudata(L, 5, TYPENAME_COLOR)),
					*static_cast<fcyColor*>(luaL_checkudata(L, 6, TYPENAME_COLOR))
				};
				p->GetSprite()->SetColor(tColors);
			}
			return 0;
		}
		static int SetFontState(lua_State* L)LNOEXCEPT
		{
			ResFont* p = LRES.FindSpriteFont(luaL_checkstring(L, 1));
			if (!p)
				return luaL_error(L, "sprite font '%s' not found.", luaL_checkstring(L, 1));

			p->SetBlendMode(TranslateBlendMode(L, 2));
			if (lua_gettop(L) == 3)
			{
				fcyColor c = *static_cast<fcyColor*>(luaL_checkudata(L, 3, TYPENAME_COLOR));
				p->SetBlendColor(c);
			}
			return 0;
		}
		static int SetAnimationState(lua_State* L)LNOEXCEPT
		{
			ResAnimation* p = LRES.FindAnimation(luaL_checkstring(L, 1));
			if (!p)
				return luaL_error(L, "animation '%s' not found.", luaL_checkstring(L, 1));

			p->SetBlendMode(TranslateBlendMode(L, 2));
			if (lua_gettop(L) == 3)
			{
				fcyColor c = *static_cast<fcyColor*>(luaL_checkudata(L, 3, TYPENAME_COLOR));
				for (size_t i = 0; i < p->GetCount(); ++i)
					p->GetSprite(i)->SetColor(c);
			}
			else if (lua_gettop(L) == 6)
			{
				fcyColor tColors[] = {
					*static_cast<fcyColor*>(luaL_checkudata(L, 3, TYPENAME_COLOR)),
					*static_cast<fcyColor*>(luaL_checkudata(L, 4, TYPENAME_COLOR)),
					*static_cast<fcyColor*>(luaL_checkudata(L, 5, TYPENAME_COLOR)),
					*static_cast<fcyColor*>(luaL_checkudata(L, 6, TYPENAME_COLOR))
				};
				for (size_t i = 0; i < p->GetCount(); ++i)
					p->GetSprite(i)->SetColor(tColors);
			}
			return 0;
		}
		static int SetImageCenter(lua_State* L)LNOEXCEPT
		{
			ResSprite* p = LRES.FindSprite(luaL_checkstring(L, 1));
			if (!p)
				return luaL_error(L, "image '%s' not found.", luaL_checkstring(L, 1));
			p->GetSprite()->SetHotSpot(fcyVec2(
				static_cast<float>(luaL_checknumber(L, 2) + p->GetSprite()->GetTexRect().a.x),
				static_cast<float>(luaL_checknumber(L, 3) + p->GetSprite()->GetTexRect().a.y)));
			return 0;
		}
		static int SetAnimationCenter(lua_State* L)LNOEXCEPT
		{
			ResAnimation* p = LRES.FindAnimation(luaL_checkstring(L, 1));
			if (!p)
				return luaL_error(L, "animation '%s' not found.", luaL_checkstring(L, 1));
			for (size_t i = 0; i < p->GetCount(); ++i)
			{
				p->GetSprite(i)->SetHotSpot(fcyVec2(
					static_cast<float>(luaL_checknumber(L, 2) + p->GetSprite(i)->GetTexRect().a.x),
					static_cast<float>(luaL_checknumber(L, 3) + p->GetSprite(i)->GetTexRect().a.y)));
			}
			return 0;
		}

		// 绘图函数
		static int BeginScene(lua_State* L)LNOEXCEPT
		{
			if (!LAPP.BeginScene())
				return luaL_error(L, "can't invoke 'BeginScene'.");
			return 0;
		}
		static int EndScene(lua_State* L)LNOEXCEPT
		{
			if (!LAPP.EndScene())
				return luaL_error(L, "can't invoke 'EndScene'.");
			return 0;
		}
		static int RenderClear(lua_State* L)LNOEXCEPT
		{
			fcyColor* c = static_cast<fcyColor*>(luaL_checkudata(L, 1, TYPENAME_COLOR));
			LAPP.ClearScreen(*c);
			return 0;
		}
		static int SetViewport(lua_State* L)LNOEXCEPT
		{
			if (!LAPP.SetViewport(
				luaL_checknumber(L, 1),
				luaL_checknumber(L, 2),
				luaL_checknumber(L, 3),
				luaL_checknumber(L, 4)
			))
			{
				return luaL_error(L, "invalid arguments for 'SetViewport'.");
			}
			return 0;
		}
		static int SetOrtho(lua_State* L)LNOEXCEPT
		{
			LAPP.SetOrtho(
				static_cast<float>(luaL_checknumber(L, 1)),
				static_cast<float>(luaL_checknumber(L, 2)),
				static_cast<float>(luaL_checknumber(L, 3)),
				static_cast<float>(luaL_checknumber(L, 4))
			);
			return 0;
		}
		static int SetPerspective(lua_State* L)LNOEXCEPT
		{
			LAPP.SetPerspective(
				static_cast<float>(luaL_checknumber(L, 1)),
				static_cast<float>(luaL_checknumber(L, 2)),
				static_cast<float>(luaL_checknumber(L, 3)),
				static_cast<float>(luaL_checknumber(L, 4)),
				static_cast<float>(luaL_checknumber(L, 5)),
				static_cast<float>(luaL_checknumber(L, 6)),
				static_cast<float>(luaL_checknumber(L, 7)),
				static_cast<float>(luaL_checknumber(L, 8)),
				static_cast<float>(luaL_checknumber(L, 9)),
				static_cast<float>(luaL_checknumber(L, 10)),
				static_cast<float>(luaL_checknumber(L, 11)),
				static_cast<float>(luaL_checknumber(L, 12)),
				static_cast<float>(luaL_checknumber(L, 13))
			);
			return 0;
		}
		static int Render(lua_State* L)LNOEXCEPT
		{
			if (!LAPP.Render(
				luaL_checkstring(L, 1),
				static_cast<float>(luaL_checknumber(L, 2)),
				static_cast<float>(luaL_checknumber(L, 3)),
				static_cast<float>(luaL_optnumber(L, 4, 0.) * LDEGREE2RAD),
				static_cast<float>(luaL_optnumber(L, 5, 1.) * LRES.GetGlobalImageScaleFactor()),
				static_cast<float>(luaL_optnumber(L, 6, luaL_optnumber(L, 5, 1.)) * LRES.GetGlobalImageScaleFactor()),
				static_cast<float>(luaL_optnumber(L, 7, 0.5))
			))
			{
				return luaL_error(L, "can't render '%m'", luaL_checkstring(L, 1));
			}
			return 0;
		}
		static int RenderRect(lua_State* L)LNOEXCEPT
		{
			if (!LAPP.RenderRect(
				luaL_checkstring(L, 1),
				static_cast<float>(luaL_checknumber(L, 2)),
				static_cast<float>(luaL_checknumber(L, 5)),
				static_cast<float>(luaL_checknumber(L, 3)),
				static_cast<float>(luaL_checknumber(L, 4))
			))
			{
				return luaL_error(L, "can't render '%m'", luaL_checkstring(L, 1));
			}
			return 0;
		}
		static int Render4V(lua_State* L)LNOEXCEPT
		{
			if (!LAPP.Render4V(
				luaL_checkstring(L, 1),
				static_cast<float>(luaL_checknumber(L, 2)),
				static_cast<float>(luaL_checknumber(L, 3)),
				static_cast<float>(luaL_checknumber(L, 4)),
				static_cast<float>(luaL_checknumber(L, 5)),
				static_cast<float>(luaL_checknumber(L, 6)),
				static_cast<float>(luaL_checknumber(L, 7)),
				static_cast<float>(luaL_checknumber(L, 8)),
				static_cast<float>(luaL_checknumber(L, 9)),
				static_cast<float>(luaL_checknumber(L, 10)),
				static_cast<float>(luaL_checknumber(L, 11)),
				static_cast<float>(luaL_checknumber(L, 12)),
				static_cast<float>(luaL_checknumber(L, 13))
			))
			{
				return luaL_error(L, "can't render '%m'.", luaL_checkstring(L, 1));
			}
			return 0;
		}
		static int RenderText(lua_State* L)LNOEXCEPT
		{
			ResFont::FontAlignHorizontal halign = ResFont::FontAlignHorizontal::Center;
			ResFont::FontAlignVertical valign = ResFont::FontAlignVertical::Middle;
			if (lua_gettop(L) == 6)
				TranslateAlignMode(L, 6, halign, valign);
			if (!LAPP.RenderText(
				luaL_checkstring(L, 1),
				luaL_checkstring(L, 2),
				(float)luaL_checknumber(L, 3),
				(float)luaL_checknumber(L, 4),
				(float)(luaL_optnumber(L, 5, 1.0) * LRES.GetGlobalImageScaleFactor()),
				halign,
				valign
				))
			{
				return luaL_error(L, "can't draw text '%m'.", luaL_checkstring(L, 1));
			}	
			return 0;
		}
		static int RenderTexture(lua_State* L)LNOEXCEPT
		{
			const char* tex_name = luaL_checkstring(L, 1);
			BlendMode blend = TranslateBlendMode(L, 2);
			f2dGraphics2DVertex vertex[4];

			for (int i = 0; i < 4; ++i)
			{
				lua_pushinteger(L, 1);
				lua_gettable(L, 3 + i);
				vertex[i].x = (float)lua_tonumber(L, -1);

				lua_pushinteger(L, 2);
				lua_gettable(L, 3 + i);
				vertex[i].y = (float)lua_tonumber(L, -1);
				
				lua_pushinteger(L, 3);
				lua_gettable(L, 3 + i);
				vertex[i].z = (float)lua_tonumber(L, -1);

				lua_pushinteger(L, 4);
				lua_gettable(L, 3 + i);
				vertex[i].u = (float)lua_tonumber(L, -1);

				lua_pushinteger(L, 5);
				lua_gettable(L, 3 + i);
				vertex[i].v = (float)lua_tonumber(L, -1);

				lua_pushinteger(L, 6);
				lua_gettable(L, 3 + i);
				vertex[i].color = static_cast<fcyColor*>(luaL_checkudata(L, -1, TYPENAME_COLOR))->argb;

				lua_pop(L, 6);
			}

			if (!LAPP.RenderTexture(tex_name, blend, vertex))
				return luaL_error(L, "can't render texture '%s'.", tex_name);
			return 0;
		}
		static int RenderTTF(lua_State* L)LNOEXCEPT
		{
			if (!LAPP.RenderTTF(
				luaL_checkstring(L, 1),
				luaL_checkstring(L, 2),
				(float)luaL_checknumber(L, 3),
				(float)luaL_checknumber(L, 4),
				(float)luaL_checknumber(L, 5),
				(float)luaL_checknumber(L, 6),
				LRES.GetGlobalImageScaleFactor() * (float)luaL_optnumber(L, 9, 1.0),
				luaL_checkinteger(L, 7),
				*static_cast<fcyColor*>(luaL_checkudata(L, 8, TYPENAME_COLOR))
			))
			{
				return luaL_error(L, "can't render font '%s'.", luaL_checkstring(L, 1));
			}	
			return 0;
		}
		static int RegTTF(lua_State* L)LNOEXCEPT
		{
			// 否决的方法
			return 0;
		}
		static int SetFog(lua_State* L)LNOEXCEPT
		{
			if (lua_gettop(L) == 3)
				LAPP.SetFog(
					static_cast<float>(luaL_checknumber(L, 1)),
					static_cast<float>(luaL_checknumber(L, 2)),
					*(static_cast<fcyColor*>(luaL_checkudata(L, 3, TYPENAME_COLOR)))
				);
			else if (lua_gettop(L) == 2)
				LAPP.SetFog(
					static_cast<float>(luaL_checknumber(L, 1)),
					static_cast<float>(luaL_checknumber(L, 2)),
					0xFF000000
				);
			else
				LAPP.SetFog(0.0f, 0.0f, 0x00FFFFFF);
			return 0;
		}
		static int PushRenderTarget(lua_State* L)LNOEXCEPT
		{
			ResTexture* p = LRES.FindTexture(luaL_checkstring(L, 1));
			if (!p)
				return luaL_error(L, "rendertarget '%s' not found.", luaL_checkstring(L, 1));
			if (!p->IsRenderTarget())
				return luaL_error(L, "'%s' is a texture.", luaL_checkstring(L, 1));

			if (!LAPP.PushRenderTarget(p))
				return luaL_error(L, "push rendertarget '%s' failed.", luaL_checkstring(L, 1));
			return 0;
		}
		static int PopRenderTarget(lua_State* L)LNOEXCEPT
		{
			if (!LAPP.PopRenderTarget())
				return luaL_error(L, "pop rendertarget failed.");
			return 0;
		}
		static int PostEffect(lua_State* L)LNOEXCEPT
		{
			const char* texture = luaL_checkstring(L, 1);
			const char* name = luaL_checkstring(L, 2);
			BlendMode blend = TranslateBlendMode(L, 3);

			// 获取纹理
			ResTexture* rt = LRES.FindTexture(luaL_checkstring(L, 1));
			if (!rt)
				return luaL_error(L, "texture '%s' not found.", texture);

			// 获取fx
			ResFX* p = LRES.FindFX(name);
			if (!p)
				return luaL_error(L, "PostEffect: can't find effect '%s'.", name);
			if (lua_istable(L, 4))
			{
				// 设置table上的参数到fx
				lua_pushnil(L);  // s s t ... nil
				while (0 != lua_next(L, 4))
				{
					// s s t ... nil key value
					const char* key = luaL_checkstring(L, -2);
					if (lua_isnumber(L, -1))
						p->SetValue(key, (float)lua_tonumber(L, -1));
					else if (lua_isstring(L, -1))
					{
						ResTexture* pTex = LRES.FindTexture(lua_tostring(L, -1));
						if (!pTex)
							return luaL_error(L, "PostEffect: can't find texture '%s'.", lua_tostring(L, -1));
						p->SetValue(key, pTex->GetTexture());
					}
					else if (lua_isuserdata(L, -1))
					{
						fcyColor c = *static_cast<fcyColor*>(luaL_checkudata(L, -1, TYPENAME_COLOR));
						p->SetValue(key, c);
					}
					else
						return luaL_error(L, "PostEffect: invalid data type.");

					lua_pop(L, 1);  // s s t ... nil key
				}
			}

			if (!LAPP.PostEffect(rt, p, blend))
				return luaL_error(L, "PostEffect failed.");
			return 0;
		}
		static int PostEffectCapture(lua_State* L)LNOEXCEPT
		{
			if (!LAPP.PostEffectCapture())
				return luaL_error(L, "PostEffectCapture failed.");
			return 0;
		}
		static int PostEffectApply(lua_State* L)LNOEXCEPT
		{
			const char* name = luaL_checkstring(L, 1);
			BlendMode blend = TranslateBlendMode(L, 2);
			
			// 获取fx
			ResFX* p = LRES.FindFX(name);
			if (!p)
				return luaL_error(L, "PostEffectApply: can't find effect '%s'.", name);
			if (lua_istable(L, 3))
			{
				// 设置table上的参数到fx
				lua_pushnil(L);  // s s t ... nil
				while (0 != lua_next(L, 3))
				{
					// s s t ... nil key value
					const char* key = luaL_checkstring(L, -2);
					if (lua_isnumber(L, -1))
						p->SetValue(key, (float)lua_tonumber(L, -1));
					else if (lua_isstring(L, -1))
					{
						ResTexture* pTex = LRES.FindTexture(lua_tostring(L, -1));
						if (!pTex)
							return luaL_error(L, "PostEffectApply: can't find texture '%s'.", lua_tostring(L, -1));
						p->SetValue(key, pTex->GetTexture());
					}
					else if (lua_isuserdata(L, -1))
					{
						fcyColor c = *static_cast<fcyColor*>(luaL_checkudata(L, -1, TYPENAME_COLOR));
						p->SetValue(key, c);
					}
					else
						return luaL_error(L, "PostEffectApply: invalid data type.");

					lua_pop(L, 1);  // s s t ... nil key
				}
			}

			if (!LAPP.PostEffectApply(p, blend))
				return luaL_error(L, "PostEffectApply failed.");
			return 0;
		}
		
		// 声音控制函数
		static int PlaySound(lua_State* L)LNOEXCEPT
		{
			const char* s = luaL_checkstring(L, 1);
			ResSound* p = LRES.FindSound(s);
			if (!p)
				return luaL_error(L, "sound '%s' not found.", s);
			p->Play((float)luaL_checknumber(L, 2) * LRES.GetGlobalSoundEffectVolume(), (float)luaL_optnumber(L, 3, 0.));
			return 0;
		}
		static int StopSound(lua_State* L)LNOEXCEPT
		{
			const char* s = luaL_checkstring(L, 1);
			ResSound* p = LRES.FindSound(s);
			if (!p)
				return luaL_error(L, "sound '%s' not found.", s);
			p->Stop();
			return 0;
		}
		static int PauseSound(lua_State* L)LNOEXCEPT
		{
			const char* s = luaL_checkstring(L, 1);
			ResSound* p = LRES.FindSound(s);
			if (!p)
				return luaL_error(L, "sound '%s' not found.", s);
			p->Pause();
			return 0;
		}
		static int ResumeSound(lua_State* L)LNOEXCEPT
		{
			const char* s = luaL_checkstring(L, 1);
			ResSound* p = LRES.FindSound(s);
			if (!p)
				return luaL_error(L, "sound '%s' not found.", s);
			p->Resume();
			return 0;
		}
		static int GetSoundState(lua_State* L)LNOEXCEPT
		{
			const char* s = luaL_checkstring(L, 1);
			ResSound* p = LRES.FindSound(s);
			if (!p)
				return luaL_error(L, "sound '%s' not found.", s);
			if (p->IsPlaying())
				lua_pushstring(L, "playing");
			else if (p->IsStopped())
				lua_pushstring(L, "stopped");
			else
				lua_pushstring(L, "paused");
			return 1;
		}
		static int PlayMusic(lua_State* L)LNOEXCEPT
		{
			const char* s = luaL_checkstring(L, 1);
			ResMusic* p = LRES.FindMusic(s);
			if (!p)
				return luaL_error(L, "music '%s' not found.", s);
			p->Play((float)luaL_optnumber(L, 2, 1.) * LRES.GetGlobalMusicVolume(), luaL_optnumber(L, 3, 0.));
			return 0;
		}
		static int StopMusic(lua_State* L)LNOEXCEPT
		{
			const char* s = luaL_checkstring(L, 1);
			ResMusic* p = LRES.FindMusic(s);
			if (!p)
				return luaL_error(L, "music '%s' not found.", s);
			p->Stop();
			return 0;
		}
		static int PauseMusic(lua_State* L)LNOEXCEPT
		{
			const char* s = luaL_checkstring(L, 1);
			ResMusic* p = LRES.FindMusic(s);
			if (!p)
				return luaL_error(L, "music '%s' not found.", s);
			p->Pause();
			return 0;
		}
		static int ResumeMusic(lua_State* L)LNOEXCEPT
		{
			const char* s = luaL_checkstring(L, 1);
			ResMusic* p = LRES.FindMusic(s);
			if (!p)
				return luaL_error(L, "music '%s' not found.", s);
			p->Resume();
			return 0;
		}
		static int GetMusicState(lua_State* L)LNOEXCEPT
		{
			const char* s = luaL_checkstring(L, 1);
			ResMusic* p = LRES.FindMusic(s);
			if (!p)
				return luaL_error(L, "music '%s' not found.", s);
			if (p->IsPlaying())
				lua_pushstring(L, "playing");
			else if (p->IsStopped())
				lua_pushstring(L, "stopped");
			else
				lua_pushstring(L, "paused");
			return 1;
		}
		static int UpdateSound(lua_State* L)LNOEXCEPT
		{
			// 否决的方法
			return 0;
		}
		static int SetSEVolume(lua_State* L)LNOEXCEPT
		{
			float x = static_cast<float>(luaL_checknumber(L, 1));
			LRES.SetGlobalSoundEffectVolume(max(min(x, 1.f), 0.f));
			return 0;
		}
		static int SetBGMVolume(lua_State* L)LNOEXCEPT
		{
			if (lua_gettop(L) == 1)
			{
				float x = static_cast<float>(luaL_checknumber(L, 1));
				LRES.SetGlobalMusicVolume(max(min(x, 1.f), 0.f));
			}
			else
			{
				const char* s = luaL_checkstring(L, 1);
				float x = static_cast<float>(luaL_checknumber(L, 2));
				ResMusic* p = LRES.FindMusic(s);
				if (!p)
					return luaL_error(L, "music '%s' not found.", s);
				p->SetVolume(x * LRES.GetGlobalMusicVolume());
			}
			return 0;
		}

		// 输入控制函数
		static int GetKeyState(lua_State* L)LNOEXCEPT
		{
			lua_pushboolean(L, LAPP.GetKeyState(luaL_checkinteger(L, -1)));
			return 1;
		}
		static int GetLastKey(lua_State* L)LNOEXCEPT
		{
			lua_pushinteger(L, LAPP.GetLastKey());
			return 1;
		}
		static int GetLastChar(lua_State* L)LNOEXCEPT
		{
			return LAPP.GetLastChar(L);
		}
		static int GetMousePosition(lua_State* L)LNOEXCEPT
		{
			fcyVec2 tPos = LAPP.GetMousePosition();
			lua_pushnumber(L, tPos.x);
			lua_pushnumber(L, tPos.y);
			return 2;
		}
		static int GetMouseState(lua_State* L)LNOEXCEPT
		{
			lua_pushboolean(L, LAPP.GetMouseState(luaL_checkinteger(L, 1)));
			return 1;
		}

		// 杂项
		static int Snapshot(lua_State* L)LNOEXCEPT
		{
			LAPP.SnapShot(luaL_checkstring(L, 1));
			return 0;
		}
		static int Execute(lua_State* L)LNOEXCEPT
		{
			struct Detail_
			{
				LNOINLINE static bool Execute(const char* path, const char* args, const char* directory, bool bWait, bool bShow)LNOEXCEPT
				{
					wstring tPath, tArgs, tDirectory;

					try
					{
						tPath = fcyStringHelper::MultiByteToWideChar(path, CP_UTF8);
						tArgs = fcyStringHelper::MultiByteToWideChar(args, CP_UTF8);
						if (directory)
							tDirectory = fcyStringHelper::MultiByteToWideChar(directory, CP_UTF8);

						SHELLEXECUTEINFO tShellExecuteInfo;
						memset(&tShellExecuteInfo, 0, sizeof(SHELLEXECUTEINFO));

						tShellExecuteInfo.cbSize = sizeof(SHELLEXECUTEINFO);
						tShellExecuteInfo.fMask = bWait ? SEE_MASK_NOCLOSEPROCESS : 0;
						tShellExecuteInfo.lpVerb = L"open";
						tShellExecuteInfo.lpFile = tPath.c_str();
						tShellExecuteInfo.lpParameters = tArgs.c_str();
						tShellExecuteInfo.lpDirectory = directory ? tDirectory.c_str() : nullptr;
						tShellExecuteInfo.nShow = bShow ? SW_SHOWDEFAULT : SW_HIDE;
						
						if (FALSE == ShellExecuteEx(&tShellExecuteInfo))
							return false;

						if (bWait)
						{
							WaitForSingleObject(tShellExecuteInfo.hProcess, INFINITE);
							CloseHandle(tShellExecuteInfo.hProcess);
						}
						return true;
					}
					catch (const std::bad_alloc&)
					{
						return false;
					}
				}
			};

			const char* path = luaL_checkstring(L, 1);
			const char* args = luaL_optstring(L, 2, "");
			const char* directory = luaL_optstring(L, 3, NULL);
			bool bWait = true;
			bool bShow = true;
			if (lua_gettop(L) >= 4)
				bWait = lua_toboolean(L, 4) == 0 ? false : true;
			if (lua_gettop(L) >= 5)
				bShow = lua_toboolean(L, 5) == 0 ? false : true;
			
			lua_pushboolean(L, Detail_::Execute(path, args, directory, bWait, bShow));
			return 1;
		}

		// 内置数学库
		static int Sin(lua_State* L)LNOEXCEPT
		{
			lua_pushnumber(L, sin(luaL_checknumber(L, 1) * LDEGREE2RAD));
			return 1;
		}
		static int Cos(lua_State* L)LNOEXCEPT
		{
			lua_pushnumber(L, cos(luaL_checknumber(L, 1) * LDEGREE2RAD));
			return 1;
		}
		static int ASin(lua_State* L)LNOEXCEPT
		{
			lua_pushnumber(L, asin(luaL_checknumber(L, 1)) * LRAD2DEGREE);
			return 1;
		}
		static int ACos(lua_State* L)LNOEXCEPT
		{
			lua_pushnumber(L, acos(luaL_checknumber(L, 1)) * LRAD2DEGREE);
			return 1;
		}
		static int Tan(lua_State* L)LNOEXCEPT
		{
			lua_pushnumber(L, tan(luaL_checknumber(L, 1) * LDEGREE2RAD));
			return 1;
		}
		static int ATan(lua_State* L)LNOEXCEPT
		{
			lua_pushnumber(L, atan(luaL_checknumber(L, 1)) * LRAD2DEGREE);
			return 1;
		}
		static int ATan2(lua_State* L)LNOEXCEPT
		{
			lua_pushnumber(L, atan2(luaL_checknumber(L, 1), luaL_checknumber(L, 2)) * LRAD2DEGREE);
			return 1;
		}

		// 调试函数
		static int ObjTable(lua_State* L)LNOEXCEPT
		{
			return LPOOL.GetObjectTable(L);
		}

		// 对象构造函数
		static int NewColor(lua_State* L)LNOEXCEPT
		{
			fcyColor c;
			if (lua_gettop(L) == 1)
				c.argb = luaL_checkinteger(L, 1);
			else
			{
				c = fcyColor(
					luaL_checkinteger(L, 1),
					luaL_checkinteger(L, 2),
					luaL_checkinteger(L, 3),
					luaL_checkinteger(L, 4)
				);
			}
			*ColorWrapper::CreateAndPush(L) = c;
			return 1;
		}
		static int NewRand(lua_State* L)LNOEXCEPT
		{
			RandomizerWrapper::CreateAndPush(L);
			return 1;
		}
		static int BentLaserData(lua_State* L)LNOEXCEPT
		{
			BentLaserDataWrapper::CreateAndPush(L);
			return 1;
		}
	};

	luaL_Reg tFunctions[] =
	{
		// 框架函数
		{ "SetWindowed", &WrapperImplement::SetWindowed },
		{ "SetFPS", &WrapperImplement::SetFPS },
		{ "GetFPS", &WrapperImplement::GetFPS },
		{ "SetVsync", &WrapperImplement::SetVsync },
		{ "SetResolution", &WrapperImplement::SetResolution },
		{ "ChangeVideoMode", &WrapperImplement::ChangeVideoMode },
		{ "SetSplash", &WrapperImplement::SetSplash },
		{ "SetTitle", &WrapperImplement::SetTitle },
		{ "SystemLog", &WrapperImplement::SystemLog },
		{ "Print", &WrapperImplement::Print },
		{ "LoadPack", &WrapperImplement::LoadPack },
		{ "UnloadPack", &WrapperImplement::UnloadPack },
		{ "ExtractRes", &WrapperImplement::ExtractRes },
		{ "DoFile", &WrapperImplement::DoFile },
		{ "ShowSplashWindow", &WrapperImplement::ShowSplashWindow },
		// 对象控制函数
		{ "GetnObj", &WrapperImplement::GetnObj },
		{ "UpdateObjList", &WrapperImplement::UpdateObjList },
		{ "ObjFrame", &WrapperImplement::ObjFrame },
		{ "ObjRender", &WrapperImplement::ObjRender },
		{ "BoundCheck", &WrapperImplement::BoundCheck },
		{ "SetBound", &WrapperImplement::SetBound },
		{ "BoxCheck", &WrapperImplement::BoxCheck },
		{ "CollisionCheck", &WrapperImplement::CollisionCheck },
		{ "UpdateXY", &WrapperImplement::UpdateXY },
		{ "AfterFrame", &WrapperImplement::AfterFrame },
		{ "New", &WrapperImplement::New },
		{ "Del", &WrapperImplement::Del },
		{ "Kill", &WrapperImplement::Kill },
		{ "IsValid", &WrapperImplement::IsValid },
		{ "Angle", &WrapperImplement::Angle },
		{ "Dist", &WrapperImplement::Dist },
		{ "GetV", &WrapperImplement::GetV },
		{ "SetV", &WrapperImplement::SetV },
		{ "SetImgState", &WrapperImplement::SetImgState },
		{ "ResetPool", &WrapperImplement::ResetPool },
		{ "DefaultRenderFunc", &WrapperImplement::DefaultRenderFunc },
		{ "NextObject", &WrapperImplement::NextObject },
		{ "ObjList", &WrapperImplement::ObjList },
		{ "GetAttr", &WrapperImplement::ObjMetaIndex },
		{ "SetAttr", &WrapperImplement::ObjMetaNewIndex },
		{ "ParticleStop", &WrapperImplement::ParticleStop },
		{ "ParticleFire", &WrapperImplement::ParticleFire },
		{ "ParticleGetn", &WrapperImplement::ParticleGetn },
		{ "ParticleGetEmission", &WrapperImplement::ParticleGetEmission },
		{ "ParticleSetEmission", &WrapperImplement::ParticleSetEmission },
		// 资源控制函数
		{ "SetResourceStatus", &WrapperImplement::SetResourceStatus },
		{ "LoadTexture", &WrapperImplement::LoadTexture },
		{ "LoadImage", &WrapperImplement::LoadImage },
		{ "LoadAnimation", &WrapperImplement::LoadAnimation },
		{ "LoadPS", &WrapperImplement::LoadPS },
		{ "LoadSound", &WrapperImplement::LoadSound },
		{ "LoadMusic", &WrapperImplement::LoadMusic },
		{ "LoadFont", &WrapperImplement::LoadFont },
		{ "LoadTTF", &WrapperImplement::LoadTTF },
		{ "RegTTF", &WrapperImplement::RegTTF },
		{ "LoadFX", &WrapperImplement::LoadFX },
		{ "CreateRenderTarget", &WrapperImplement::CreateRenderTarget },
		{ "IsRenderTarget", &WrapperImplement::IsRenderTarget },
		{ "GetTextureSize", &WrapperImplement::GetTextureSize },
		{ "RemoveResource", &WrapperImplement::RemoveResource },
		{ "CheckRes", &WrapperImplement::CheckRes },
		{ "EnumRes", &WrapperImplement::EnumRes },
		{ "SetImageScale", &WrapperImplement::SetImageScale },
		{ "SetImageState", &WrapperImplement::SetImageState },
		{ "SetFontState", &WrapperImplement::SetFontState },
		{ "SetAnimationState", &WrapperImplement::SetAnimationState },
		{ "SetImageCenter", &WrapperImplement::SetImageCenter },
		{ "SetAnimationCenter", &WrapperImplement::SetAnimationCenter },
		// 绘图函数
		{ "BeginScene", &WrapperImplement::BeginScene },
		{ "EndScene", &WrapperImplement::EndScene },
		{ "RenderClear", &WrapperImplement::RenderClear },
		{ "SetViewport", &WrapperImplement::SetViewport },
		{ "SetOrtho", &WrapperImplement::SetOrtho },
		{ "SetPerspective", &WrapperImplement::SetPerspective },
		{ "Render", &WrapperImplement::Render },
		{ "RenderRect", &WrapperImplement::RenderRect },
		{ "Render4V", &WrapperImplement::Render4V },
		{ "RenderText", &WrapperImplement::RenderText },
		{ "RenderTexture", &WrapperImplement::RenderTexture },
		{ "RenderTTF", &WrapperImplement::RenderTTF },
		{ "SetFog", &WrapperImplement::SetFog },
		{ "PushRenderTarget", &WrapperImplement::PushRenderTarget },
		{ "PopRenderTarget", &WrapperImplement::PopRenderTarget },
		{ "PostEffect", &WrapperImplement::PostEffect },
		{ "PostEffectCapture", &WrapperImplement::PostEffectCapture },
		{ "PostEffectApply", &WrapperImplement::PostEffectApply },
		// 声音控制函数
		{ "PlaySound", &WrapperImplement::PlaySound },
		{ "StopSound", &WrapperImplement::StopSound },
		{ "PauseSound", &WrapperImplement::PauseSound },
		{ "ResumeSound", &WrapperImplement::ResumeSound },
		{ "GetSoundState", &WrapperImplement::GetSoundState },
		{ "PlayMusic", &WrapperImplement::PlayMusic },
		{ "StopMusic", &WrapperImplement::StopMusic },
		{ "PauseMusic", &WrapperImplement::PauseMusic },
		{ "ResumeMusic", &WrapperImplement::ResumeMusic },
		{ "GetMusicState", &WrapperImplement::GetMusicState },
		{ "UpdateSound", &WrapperImplement::UpdateSound },
		{ "SetSEVolume", &WrapperImplement::SetSEVolume },
		{ "SetBGMVolume", &WrapperImplement::SetBGMVolume },
		// 输入控制函数
		{ "GetKeyState", &WrapperImplement::GetKeyState },
		{ "GetLastKey", &WrapperImplement::GetLastKey },
		{ "GetLastChar", &WrapperImplement::GetLastChar },
		{ "GetMousePosition", &WrapperImplement::GetMousePosition },
		{ "GetMouseState", &WrapperImplement::GetMouseState },
		// 内置数学函数
		{ "sin", &WrapperImplement::Sin },
		{ "cos", &WrapperImplement::Cos },
		{ "asin", &WrapperImplement::ASin },
		{ "acos", &WrapperImplement::ACos },
		{ "tan", &WrapperImplement::Tan },
		{ "atan", &WrapperImplement::ATan },
		{ "atan2", &WrapperImplement::ATan2 },
		// 杂项
		{ "Snapshot", &WrapperImplement::Snapshot },
		{ "Execute", &WrapperImplement::Execute },
		// 调试函数
		{ "ObjTable", &WrapperImplement::ObjTable },
		// 对象构造函数
		{ "Color", &WrapperImplement::NewColor },
		{ "Rand", &WrapperImplement::NewRand },
		{ "BentLaserData", &WrapperImplement::BentLaserData },
		{ NULL, NULL }
	};

	luaL_register(L, "lstg", tFunctions);  // t
	lua_pop(L, 1);
}
#pragma endregion
