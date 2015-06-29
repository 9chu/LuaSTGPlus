#include "LuaWrapper.h"
#include "AppFrame.h"

#define TYPENAME_COLOR "lstgColor"
#define TYPENAME_RANDGEN "lstgRand"

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
/// BuiltInFunctionWrapper
////////////////////////////////////////////////////////////////////////////////
#pragma region BuiltInFunctionWrapper
void BuiltInFunctionWrapper::Register(lua_State* L)LNOEXCEPT
{
	struct WrapperImplement
	{
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
			else
				luaL_error(L, "invalid blend mode '%s'.", s);
			return BlendMode::MulAlpha;
		}

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
			switch (lua_gettop(L))
			{
			case 2:
				if (!LRES.GetActivedPool()->LoadTexture(name, path))
					return luaL_error(L, "can't load texture from file '%s'.", path);
				break;
			case 3:
				if (!LRES.GetActivedPool()->LoadTexture(name, path, lua_toboolean(L, 3) == 0 ? false : true))
					return luaL_error(L, "can't load texture from file '%s'.", path);
				break;
			default:
				return luaL_error(L, "invalid argument count for 'LoadTexture'.");
				break;
			}
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
			return 0;
		}
		static int LoadMusic(lua_State* L)LNOEXCEPT
		{
			return 0;
		}
		static int LoadFont(lua_State* L)LNOEXCEPT
		{
			return 0;
		}
		static int LoadTTF(lua_State* L)LNOEXCEPT
		{
			return 0;
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
			return 0;
		}
		static int SetFontState2(lua_State* L)LNOEXCEPT
		{
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
				return luaL_error(L, "can't render '%m'", luaL_checkstring(L, 1));
			}
			return 0;
		}
		static int RenderText(lua_State* L)LNOEXCEPT
		{
			return 0;
		}
		static int RenderTexture(lua_State* L)LNOEXCEPT
		{
			return 0;
		}
		static int RenderTTF(lua_State* L)LNOEXCEPT
		{
			return 0;
		}
		static int RegTTF(lua_State* L)LNOEXCEPT
		{
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
		
		// 截图函数
		static int Snapshot(lua_State* L)LNOEXCEPT
		{
			return 0;
		}

		// 声音控制函数
		static int PlaySound(lua_State* L)LNOEXCEPT
		{
			return 0;
		}
		static int PlayMusic(lua_State* L)LNOEXCEPT
		{
			return 0;
		}
		static int StopMusic(lua_State* L)LNOEXCEPT
		{
			return 0;
		}
		static int PauseMusic(lua_State* L)LNOEXCEPT
		{
			return 0;
		}
		static int ResumeMusic(lua_State* L)LNOEXCEPT
		{
			return 0;
		}
		static int GetMusicState(lua_State* L)LNOEXCEPT
		{
			return 0;
		}
		static int UpdateSound(lua_State* L)LNOEXCEPT
		{
			return 0;
		}
		static int SetSEVolume(lua_State* L)LNOEXCEPT
		{
			return 0;
		}
		static int SetBGMVolume(lua_State* L)LNOEXCEPT
		{
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
		static int BentLaserData(lua_State* L)LNOEXCEPT
		{
			return 0;
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
	};

	luaL_Reg tFunctions[] =
	{
		// 框架函数
		{ "SetWindowed", &WrapperImplement::SetWindowed },
		{ "SetFPS", &WrapperImplement::SetFPS },
		{ "GetFPS", &WrapperImplement::GetFPS },
		{ "SetVsync", &WrapperImplement::SetVsync },
		{ "SetResolution", &WrapperImplement::SetResolution },
		{ "SetSplash", &WrapperImplement::SetSplash },
		{ "SetTitle", &WrapperImplement::SetTitle },
		{ "SystemLog", &WrapperImplement::SystemLog },
		{ "Print", &WrapperImplement::Print },
		{ "LoadPack", &WrapperImplement::LoadPack },
		{ "UnloadPack", &WrapperImplement::UnloadPack },
		{ "ExtractRes", &WrapperImplement::ExtractRes },
		{ "DoFile", &WrapperImplement::DoFile },
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
		{ "GetTextureSize", &WrapperImplement::GetTextureSize },
		{ "RemoveResource", &WrapperImplement::RemoveResource },
		{ "CheckRes", &WrapperImplement::CheckRes },
		{ "EnumRes", &WrapperImplement::EnumRes },
		{ "SetImageScale", &WrapperImplement::SetImageScale },
		{ "SetImageState", &WrapperImplement::SetImageState },
		{ "SetFontState", &WrapperImplement::SetFontState },
		{ "SetFontState2", &WrapperImplement::SetFontState2 },
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
		// 截图函数
		{ "Snapshot", &WrapperImplement::Snapshot },
		// 声音控制函数
		{ "PlaySound", &WrapperImplement::PlaySound },
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
		// 内置数学函数
		{ "sin", &WrapperImplement::Sin },
		{ "cos", &WrapperImplement::Cos },
		{ "asin", &WrapperImplement::ASin },
		{ "acos", &WrapperImplement::ACos },
		{ "tan", &WrapperImplement::Tan },
		{ "atan", &WrapperImplement::ATan },
		{ "atan2", &WrapperImplement::ATan2 },
		// 调试函数
		{ "ObjTable", &WrapperImplement::ObjTable },
		{ "BentLaserData", &WrapperImplement::BentLaserData },
		// 对象构造函数
		{ "Color", &WrapperImplement::NewColor },
		{ "Rand", &WrapperImplement::NewRand },
		{ NULL, NULL }
	};

	luaL_register(L, "lstg", tFunctions);  // t
	lua_pop(L, 1);
}
#pragma endregion
