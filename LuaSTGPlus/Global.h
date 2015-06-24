/// @file Global.h
/// @brief 全局定义文件
#pragma once

// C
#include <cassert>
#include <cstdlib>
#include <cstdio>
#include <cmath>

// STL
#include <algorithm>
#include <string>
#include <vector>
#include <map>
#include <set>
#include <unordered_map>
#include <unordered_set>

// CRTDBG
#include <crtdbg.h>

// fancy2d
#include <fcyIO/fcyStream.h>
#include <fcyIO/fcyBinaryHelper.h>
#include <fcyParser/fcyPathParser.h>
#include <fcyMisc/fcyStringHelper.h>
#include <fcyMisc/fcyRandom.h>
#include <f2d.h>

// Zlib
#include <zlib.h>
#include <unzip.h>

// luajit
#include <lua.hpp>

// 日志系统
#include "LogSystem.h"

// 一些全局范围的宏
#define LVERSION L"luaSTGPlus-0.1"
#define LVERSION_LUA LUAJIT_VERSION

#define LLOGFILE L"log.txt"
#define LLAUNCH_SCRIPT L"launch"
#define LCORE_SCRIPT L"core.lua"

#define LFUNC_GAMEINIT "GameInit"

#define LNOEXCEPT throw()
#define LNOINLINE __declspec(noinline)
#define LNOUSE(x) static_cast<void>(x)
#ifdef _DEBUG
#define LDEBUG
#endif

#define LAPP (LuaSTGPlus::AppFrame::GetInstance())
#define LLOGGER (LuaSTGPlus::LogSystem::GetInstance())

#define LWIDE_(x) L ## x
#define LWIDE(x) LWIDE_(x)
#define LERROR(info, ...) LLOGGER.Log(LuaSTGPlus::LogType::Error, L##info, __VA_ARGS__)
#define LWARNING(info, ...) LLOGGER.Log(LuaSTGPlus::LogType::Warning, L##info, __VA_ARGS__)
#define LINFO(info, ...) LLOGGER.Log(LuaSTGPlus::LogType::Information, L##info, __VA_ARGS__)

#ifdef LDEBUG
#define LASSERT(cond) \
	if (!(cond)) \
	{ \
		LERROR("调试断言失败 于文件 '%s' 函数 '%s' 行 %d: %s", LWIDE(__FILE__), LWIDE(__FUNCTION__), __LINE__, L#cond); \
		_wassert(L#cond, LWIDE(__FILE__), __LINE__); \
	}
#else
#define LASSERT(cond)
#endif

namespace LuaSTGPlus
{
	class AppFrame;
}
