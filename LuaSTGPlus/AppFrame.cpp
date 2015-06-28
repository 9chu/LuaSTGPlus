#include "AppFrame.h"
#include "Utility.h"
#include "LuaWrapper.h"

#include "D3D9.H"  // for SetFog

// 内置lua扩展
extern "C" int luaopen_lfs(lua_State *L);

using namespace std;
using namespace LuaSTGPlus;

LNOINLINE AppFrame& AppFrame::GetInstance()
{
	static AppFrame s_Instance;
	return s_Instance;
}

AppFrame::AppFrame()
{
}

AppFrame::~AppFrame()
{
	if (m_iStatus != AppStatus::NotInitialized && m_iStatus != AppStatus::Destroyed)
	{
		// 若没有销毁框架，则执行销毁
		Shutdown();
	}
}

#pragma region 脚本接口
void AppFrame::SetWindowed(bool v)LNOEXCEPT
{
	if (m_iStatus == AppStatus::Initializing)
		m_OptionWindowed = v;
	else if (m_iStatus == AppStatus::Running)
		LWARNING("试图在运行时更改窗口化模式");
}

void AppFrame::SetFPS(fuInt v)LNOEXCEPT
{
	if (m_iStatus == AppStatus::Initializing)
		m_OptionFPSLimit = v;
	else if (m_iStatus == AppStatus::Running)
		LWARNING("试图在运行时更改FPS限制");
}

void AppFrame::SetVsync(bool v)LNOEXCEPT
{
	if (m_iStatus == AppStatus::Initializing)
		m_OptionVsync = v;
	else if (m_iStatus == AppStatus::Running)
		LWARNING("试图在运行时更改垂直同步模式");
}

void AppFrame::SetResolution(fuInt width, fuInt height)LNOEXCEPT
{
	if (m_iStatus == AppStatus::Initializing)
		m_OptionResolution.Set((float)width, (float)height);
	else if (m_iStatus == AppStatus::Running)
		LWARNING("试图在运行时更改分辨率");
}

void AppFrame::SetSplash(bool v)LNOEXCEPT
{
	m_OptionSplash = v;
	if (m_pMainWindow)
		m_pMainWindow->HideMouse(m_OptionSplash);
}

LNOINLINE void AppFrame::SetTitle(const char* v)LNOEXCEPT
{
	try
	{
		m_OptionTitle = std::move(fcyStringHelper::MultiByteToWideChar(v, CP_UTF8));
		if (m_pMainWindow)
			m_pMainWindow->SetCaption(m_OptionTitle.c_str());
	}
	catch (const bad_alloc&)
	{
		LERROR("修改窗口标题时无法分配内存");
	}
}

LNOINLINE bool AppFrame::UpdateVideoParameters()LNOEXCEPT
{
	if (m_iStatus == AppStatus::Initialized)
	{
		// 获取原始视频选项
		bool tOrgOptionWindowed = m_pRenderDev->IsWindowed();
		fcyVec2 tOrgOptionResolution = fcyVec2((float)m_pRenderDev->GetBufferWidth(),
			(float)m_pRenderDev->GetBufferHeight());

		// 切换到新的视频选项
		if (FCYOK(m_pRenderDev->SetBufferSize(
			(fuInt)m_OptionResolution.x,
			(fuInt)m_OptionResolution.y,
			m_OptionWindowed,
			m_OptionVsync,
			F2DAALEVEL_NONE)))
		{
			LINFO("视频模式切换成功 (%dx%d Vsync:%b Windowed:%b) -> (%dx%d Vsync:%b Windowed:%b)",
				(int)tOrgOptionResolution.x, (int)tOrgOptionResolution.y, m_OptionVsyncOrg, tOrgOptionWindowed,
				(int)m_OptionResolution.x, (int)m_OptionResolution.y, m_OptionVsync, m_OptionWindowed);

			// 切换窗口大小
			m_pMainWindow->SetBorderType(m_OptionWindowed ? F2DWINBORDERTYPE_FIXED : F2DWINBORDERTYPE_NONE);
			m_pMainWindow->SetClientRect(
				fcyRect(10.f, 10.f, 10.f + m_OptionResolution.x, 10.f + m_OptionResolution.y)
			);
			m_pMainWindow->SetTopMost(!m_OptionWindowed);
			m_pMainWindow->MoveToCenter();

			m_OptionVsyncOrg = m_OptionVsync;
			return true;
		}
		else
		{
			LINFO("视频模式切换失败 (%dx%d Vsync:%b Windowed:%b) -> (%dx%d Vsync:%b Windowed:%b)",
				tOrgOptionResolution.x, tOrgOptionResolution.y, m_OptionVsyncOrg, tOrgOptionWindowed,
				m_OptionResolution.x, m_OptionResolution.y, m_OptionVsync, m_OptionWindowed);
		}
	}
	return false;
}

LNOINLINE void AppFrame::LoadScript(const char* path)LNOEXCEPT
{
	LINFO("装载脚本'%m'", path);
	if (!m_ResourceMgr.LoadFile(path, m_TempBuffer))
	{
		luaL_error(L, "can't load script '%s'", path);
		return;
	}
	if (luaL_loadbuffer(L, m_TempBuffer.data(), m_TempBuffer.size(), luaL_checkstring(L, 1)))
	{
		const char* tDetail = lua_tostring(L, -1);
		LERROR("编译脚本'%m'失败: %m", path, tDetail);
		luaL_error(L, "failed to compile '%s': %s", path, tDetail);
		return;
	}
	if (lua_pcall(L, 0, 0, 0))
	{
		const char* tDetail = lua_tostring(L, -1);
		LERROR("执行脚本'%m'失败", path);
		luaL_error(L, "failed to execute '%s':\n\t%s", path, tDetail);
		return;
	}
}

fBool AppFrame::GetKeyState(int VKCode)LNOEXCEPT
{
	if (VKCode > 0 && VKCode < _countof(m_KeyStateMap))
		return m_KeyStateMap[VKCode];
	return false;
}

LNOINLINE int AppFrame::GetLastChar(lua_State* L)LNOEXCEPT
{
	if (m_LastChar)
	{
		try
		{
			fCharW tBuf[2] = { m_LastChar, 0 };
			lua_pushstring(L,
				fcyStringHelper::WideCharToMultiByte(tBuf, CP_UTF8).c_str());
		}
		catch (const bad_alloc&)
		{
			LERROR("GetLastChar: 内存不足");
			return 0;
		}
	}
	else
		lua_pushstring(L, "");
	return 1;
}

bool AppFrame::BeginScene()LNOEXCEPT
{
	if (!m_bRenderStarted)
	{
		LERROR("不能在RenderFunc以外的地方执行渲染");
		return false;
	}

	if (m_GraphType == GraphicsType::Graph2D)
	{
		if (FCYFAILED(m_Graph2D->Begin()))
		{
			LERROR("执行f2dGraphics2D::Begin失败");
			return false;
		}
	}

	return true;
}

bool AppFrame::EndScene()LNOEXCEPT
{
	if (m_GraphType == GraphicsType::Graph2D)
	{
		if (FCYFAILED(m_Graph2D->End()))
		{
			LERROR("执行f2dGraphics2D::End失败");
			return false;
		}
	}

	return true;
}

void AppFrame::SetFog(float start, float end, fcyColor color)
{
	if (m_Graph2D->IsInRender())
		m_Graph2D->Flush();

	// 从f2dRenderDevice中取出D3D设备
	IDirect3DDevice9* pDev = (IDirect3DDevice9*)m_pRenderDev->GetHandle();

	if (start != end)
	{
		pDev->SetRenderState(D3DRS_FOGENABLE, TRUE);
		pDev->SetRenderState(D3DRS_FOGCOLOR, color.argb);
		if (start == -1.0f)
		{
			pDev->SetRenderState(D3DRS_FOGTABLEMODE, D3DFOG_EXP);
			pDev->SetRenderState(D3DRS_FOGDENSITY, *(DWORD *)(&end));
		}
		else if (start == -2.0f)
		{
			pDev->SetRenderState(D3DRS_FOGTABLEMODE, D3DFOG_EXP2);
			pDev->SetRenderState(D3DRS_FOGDENSITY, *(DWORD *)(&end));
		}
		else
		{
			pDev->SetRenderState(D3DRS_FOGTABLEMODE, D3DFOG_LINEAR);
			pDev->SetRenderState(D3DRS_FOGSTART, *(DWORD *)(&start));
			pDev->SetRenderState(D3DRS_FOGEND, *(DWORD *)(&end));
		}
	}
	else
		pDev->SetRenderState(D3DRS_FOGENABLE, FALSE);
}

#pragma endregion

#pragma region 框架函数
bool AppFrame::Init()LNOEXCEPT
{
	LASSERT(m_iStatus == AppStatus::NotInitialized);

	LINFO("开始初始化 版本: %s", LVERSION);
	m_iStatus = AppStatus::Initializing;
	
	//////////////////////////////////////// Lua初始化部分
	LINFO("开始初始化Lua虚拟机 版本: %m", LVERSION_LUA);
	L = lua_open();
	if (!L)
	{
		LERROR("无法初始化Lua虚拟机");
		return false;
	}
	if (0 != luaJIT_setmode(L, 0, LUAJIT_MODE_ENGINE | LUAJIT_MODE_ON))
		LWARNING("无法启动JIT模式");

	luaL_openlibs(L);  // 内建库
	luaopen_lfs(L);  // 文件系统库
	ColorWrapper::Register(L);  // 颜色对象
	RandomizerWrapper::Register(L);  // 随机数发生器
	BuiltInFunctionWrapper::Register(L);  // 内建函数库
	
	// 为对象池分配空间
	LINFO("初始化对象池 上限=%u", LGOBJ_MAXCNT);
	try
	{
		m_GameObjectPool = make_unique<GameObjectPool>(L);
	}
	catch (const bad_alloc&)
	{
		LERROR("无法为对象池分配足够内存");
		return false;
	}

	// 设置命令行参数
	lua_newtable(L);
	for (int i = 0; i < __argc; ++i)
	{
		lua_pushnumber(L, i + 1);
		lua_pushstring(L, __argv[i]);
		lua_settable(L, -3);
	}
	lua_setglobal(L, "arg");

	//////////////////////////////////////// 装载初始化脚本
	LINFO("装载初始化脚本'%s'", LLAUNCH_SCRIPT);
	if (!m_ResourceMgr.LoadFile(LLAUNCH_SCRIPT, m_TempBuffer))
		return false;
	if (!SafeCallScript(m_TempBuffer.data(), m_TempBuffer.size(), "launch"))
		return false;

	//////////////////////////////////////// 初始化fancy2d引擎
	LINFO("初始化fancy2d 版本 %d.%d (分辨率: %dx%d 垂直同步: %b 窗口化: %b)",
		(F2DVERSION & 0xFFFF0000) >> 16, F2DVERSION & 0x0000FFFF,
		(int)m_OptionResolution.x, (int)m_OptionResolution.y, m_OptionVsync, m_OptionWindowed);
	struct : public f2dInitialErrListener
	{
		void OnErr(fuInt TimeTick, fcStr Src, fcStr Desc)
		{
			LERROR("初始化fancy2d失败 (异常信息'%m' 源'%m')", Desc, Src);
		}
	} tErrListener;

	if (FCYFAILED(CreateF2DEngineAndInit(
		F2DVERSION,
		fcyRect(0.f, 0.f, m_OptionResolution.x, m_OptionResolution.y),
		m_OptionTitle.c_str(),
		m_OptionWindowed,
		m_OptionVsync,
		F2DAALEVEL_NONE,
		this,
		&m_pEngine,
		&tErrListener
		)))
	{
		return false;
	}
	m_OptionVsyncOrg = m_OptionVsync;

	// 获取组件
	m_pMainWindow = m_pEngine->GetMainWindow();
	m_pRenderer = m_pEngine->GetRenderer();
	m_pRenderDev = m_pRenderer->GetDevice();
	m_pSoundSys = m_pEngine->GetSoundSys();

	// 创建渲染器
	if (FCYFAILED(m_pRenderDev->CreateGraphics2D(1024, 2048, &m_Graph2D)))
	{
		LERROR("无法创建渲染器 (fcyRenderDevice::CreateGraphics2D failed)");
		return false;
	}
	m_Graph2DLastBlendMode = BlendMode::AddAlpha;
	m_Graph2DBlendState = m_Graph2D->GetBlendState();
	m_Graph2DColorBlendState = m_Graph2D->GetColorBlendType();
	m_bRenderStarted = false;

	// 显示窗口（初始化时显示窗口，至少在加载的时候留个界面给用户）
	m_pMainWindow->MoveToCenter();
	m_pMainWindow->SetVisiable(true);

	m_LastChar = 0;
	m_LastKey = 0;
	memset(m_KeyStateMap, 0, sizeof(m_KeyStateMap));

	//////////////////////////////////////// 装载核心脚本并执行GameInit
	LINFO("装载核心脚本'%s'", LCORE_SCRIPT);
	if (!m_ResourceMgr.LoadFile(LCORE_SCRIPT, m_TempBuffer))
		return false;
	if (!SafeCallScript(m_TempBuffer.data(), m_TempBuffer.size(), "core.lua"))
		return false;
	if (!SafeCallGlobalFunction(LFUNC_GAMEINIT))
		return false;

	m_iStatus = AppStatus::Initialized;
	LINFO("初始化成功完成");
	return true;
}

void AppFrame::Shutdown()LNOEXCEPT
{
	m_GameObjectPool = nullptr;
	LINFO("已清空对象池");

	m_ResourceMgr.ClearAllResource();
	LINFO("已清空所有资源");

	m_Graph2D = nullptr;
	m_pEngine = nullptr;
	m_pMainWindow = nullptr;
	m_pRenderer = nullptr;
	m_pRenderDev = nullptr;
	m_pSoundSys = nullptr;
	LINFO("已卸载fancy2d");

	if (L)
	{
		lua_close(L);
		L = nullptr;
		LINFO("已卸载Lua虚拟机");
	}
	m_ResourceMgr.UnloadAllPack();
	LINFO("已卸载所有资源包");

	m_iStatus = AppStatus::Destroyed;
	LINFO("框架销毁");
}

void AppFrame::Run()LNOEXCEPT
{
	LASSERT(m_iStatus == AppStatus::Initialized);
	LINFO("开始执行游戏循环");

	m_pEngine->Run(F2DENGTHREADMODE_MULTITHREAD, m_OptionFPSLimit);

	LINFO("退出游戏循环");
}

bool AppFrame::SafeCallScript(const char* source, size_t len, const char* desc)LNOEXCEPT
{
	if (0 != luaL_loadbuffer(L, source, len, desc))
	{
		try
		{
			wstring tErrorInfo = StringFormat(
				L"脚本'%m'编译失败: %m",
				desc,
				lua_tostring(L, -1)
			);

			LERROR("脚本错误：%s", tErrorInfo.c_str());
			MessageBox(
				m_pMainWindow ? (HWND)m_pMainWindow->GetHandle() : 0,
				tErrorInfo.c_str(),
				L"LuaSTGPlus脚本错误",
				MB_ICONERROR | MB_OK
			);
		}
		catch (const bad_alloc&)
		{
			LERROR("尝试写出脚本错误时发生内存不足错误");
		}
		
		lua_pop(L, 1);
		return false;
	}

	if (0 != lua_pcall(L, 0, 0, 0))
	{
		try
		{
			wstring tErrorInfo = StringFormat(
				L"脚本'%m'中产生未处理的运行时错误:\n\t%m",
				desc,
				lua_tostring(L, -1)
			);

			LERROR("脚本错误：%s", tErrorInfo.c_str());
			MessageBox(
				m_pMainWindow ? (HWND)m_pMainWindow->GetHandle() : 0,
				tErrorInfo.c_str(),
				L"LuaSTGPlus脚本错误",
				MB_ICONERROR | MB_OK
			);
		}
		catch (const bad_alloc&)
		{
			LERROR("尝试写出脚本错误时发生内存不足错误");
		}

		lua_pop(L, 1);
		return false;
	}

	return true;
}

bool AppFrame::SafeCallGlobalFunction(const char* name, int argc, int retc)LNOEXCEPT
{
	lua_getglobal(L, name);
	if (0 != lua_pcall(L, argc, retc, 0))
	{
		try
		{
			wstring tErrorInfo = StringFormat(
				L"执行函数'%m'时产生未处理的运行时错误:\n\t%m",
				name,
				lua_tostring(L, -1)
			);

			LERROR("脚本错误：%s", tErrorInfo.c_str());
			MessageBox(
				m_pMainWindow ? (HWND)m_pMainWindow->GetHandle() : 0,
				tErrorInfo.c_str(),
				L"LuaSTGPlus脚本错误",
				MB_ICONERROR | MB_OK
			);
		}
		catch (const bad_alloc&)
		{
			LERROR("尝试写出脚本错误时发生内存不足错误");
		}

		lua_pop(L, 1);
		return false;
	}

	return true;
}
#pragma endregion

#pragma region 游戏循环
fBool AppFrame::OnUpdate(fDouble ElapsedTime, f2dFPSController* pFPSController, f2dMsgPump* pMsgPump)
{
	m_fFPS = (float)pFPSController->GetFPS();

	// 处理消息
	f2dMsg tMsg;
	while (FCYOK(pMsgPump->GetMsg(&tMsg)))
	{
		switch (tMsg.Type)
		{
		case F2DMSG_WINDOW_ONCLOSE:
			return false;  // 关闭窗口时结束循环
		case F2DMSG_WINDOW_ONGETFOCUS:
			if (!SafeCallGlobalFunction(LFUNC_GAINFOCUS))
				return false;
		case F2DMSG_WINDOW_ONLOSTFOCUS:
			if (!SafeCallGlobalFunction(LFUNC_LOSEFOCUS))
				return false;
		case F2DMSG_WINDOW_ONCHARINPUT:
			m_LastChar = (fCharW)tMsg.Param1;
			break;
		case F2DMSG_WINDOW_ONKEYDOWN:
			m_LastKey = (fInt)tMsg.Param1;
			if (0 < tMsg.Param1 && tMsg.Param1 < _countof(m_KeyStateMap))
				m_KeyStateMap[tMsg.Param1] = true;
			break;
		case F2DMSG_WINDOW_ONKEYUP:
			if (m_LastKey == tMsg.Param1)
				m_LastKey = 0;
			if (0 < tMsg.Param1 && tMsg.Param1 < _countof(m_KeyStateMap))
				m_KeyStateMap[tMsg.Param1] = false;
			break;
		default:
			break;
		}
	}

	// 执行帧函数
	if (!SafeCallGlobalFunction(LFUNC_FRAME, 0, 1))
		return false;
	bool tAbort = lua_toboolean(L, -1) == 0 ? false : true;
	lua_pop(L, 1);

	return !tAbort;
}

fBool AppFrame::OnRender(fDouble ElapsedTime, f2dFPSController* pFPSController)
{
	m_pRenderDev->Clear();

	// 执行渲染函数
	m_bRenderStarted = true;
	if (!SafeCallGlobalFunction(LFUNC_RENDER, 0, 0))
		m_pEngine->Abort();
	m_bRenderStarted = false;

	return true;
}
#pragma endregion
