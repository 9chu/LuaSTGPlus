/// @file Global.h
/// @brief ȫ�ֶ����ļ�
#pragma once
/*
                           _ooOoo_
                          o8888888o
                          88" . "88
                          (| -_- |)
                          O\  =  /O
                       ____/`---'\____
                     .'  \\|     |//  `.
                    /  \\|||  :  |||//  \
                   /  _||||| -:- |||||-  \
                   |   | \\\  -  /// |   |
                   | \_|  ''\---/''  |   |
                   \  .-\__  `-`  ___/-. /
                 ___`. .'  /--.--\  `. . __
              ."" '<  `.___\_<|>_/___.'  >'"".
             | | :  `- \`.;`\ _ /`;.`/ - ` : | |
             \  \ `-.   \_ __\ /__ _/   .-` /  /
        ======`-.____`-.___\_____/___.-`____.-'======
                           `=---='
        ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
                 ���汣��   ����BUG   �����޸�
*/

// C
#include <cassert>
#include <cstdlib>
#include <cstdio>
#include <cmath>

// STL
#include <algorithm>
#include <string>
#include <vector>
#include <memory>
#include <map>
#include <set>
#include <array>
#include <limits>
#include <type_traits>
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

// ��־ϵͳ
#include "LogSystem.h"

// һЩȫ�ַ�Χ�ĺ�
#define LVERSION L"luaSTGPlus-0.1"
#define LVERSION_LUA LUAJIT_VERSION

// ȫ���ļ�
#define LLOGFILE L"log.txt"
#define LLAUNCH_SCRIPT L"launch"
#define LCORE_SCRIPT L"core.lua"

// ȫ�ֻص���������
#define LFUNC_GAMEINIT "GameInit"
#define LFUNC_FRAME "FrameFunc"
#define LFUNC_RENDER "RenderFunc"
#define LFUNC_LOSEFOCUS "FocusLoseFunc"
#define LFUNC_GAINFOCUS "FocusGainFunc"

// �������Ϣ
#define LGOBJ_MAXCNT 32768
#define LGOBJ_DEFAULTGROUP 0
#define LGOBJ_GROUPCNT 16

// CLASS�д�ŵĻص��������±�
#define LGOBJ_CC_INIT 1
#define LGOBJ_CC_DEL 2
#define LGOBJ_CC_FRAME 3
#define LGOBJ_CC_RENDER 4
#define LGOBJ_CC_COLLI 5
#define LGOBJ_CC_KILL 6

// ��ѧ����
#define LRAD2DEGREE (180.0/3.141592653589793) // ���ȵ��Ƕ�
#define LDEGREE2RAD (1.0/LRAD2DEGREE) // �Ƕȵ�����

#define LNOEXCEPT throw()
#define LNOINLINE __declspec(noinline)
#define LNOUSE(x) static_cast<void>(x)
#ifdef _DEBUG
#define LDEBUG
#endif

#define LSHOWRESLOADINFO

#define LAPP (LuaSTGPlus::AppFrame::GetInstance())
#define LLOGGER (LuaSTGPlus::LogSystem::GetInstance())
#define LPOOL (LAPP.GetGameObjectPool())
#define LRES (LAPP.GetResourceMgr())

#define LWIDE_(x) L ## x
#define LWIDE(x) LWIDE_(x)
#define LERROR(info, ...) LLOGGER.Log(LuaSTGPlus::LogType::Error, L##info, __VA_ARGS__)
#define LWARNING(info, ...) LLOGGER.Log(LuaSTGPlus::LogType::Warning, L##info, __VA_ARGS__)
#define LINFO(info, ...) LLOGGER.Log(LuaSTGPlus::LogType::Information, L##info, __VA_ARGS__)

#ifdef LDEBUG
#define LASSERT(cond) \
	if (!(cond)) \
	{ \
		LERROR("���Զ���ʧ�� ���ļ� '%s' ���� '%s' �� %d: %s", LWIDE(__FILE__), LWIDE(__FUNCTION__), __LINE__, L#cond); \
		_wassert(L#cond, LWIDE(__FILE__), __LINE__); \
	}
#else
#define LASSERT(cond)
#endif

namespace LuaSTGPlus
{
	class AppFrame;
}
