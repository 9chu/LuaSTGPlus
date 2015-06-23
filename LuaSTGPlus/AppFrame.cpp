#include "AppFrame.h"
#include "Utility.h"
#include "LuaWrapper.h"

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

std::string AppFrame::loadFileAsString(fcStrW path)
{
	fcyRefPointer<fcyFileStream> pFile;
	pFile.DirectSet(new fcyFileStream(path, false));

	fLen tBytesReaded;
	std::string tRet;
	if (pFile->GetLength() > 0)
	{
		tRet.resize((fuInt)pFile->GetLength());
		pFile->ReadBytes((fData)tRet.data(), tRet.size(), &tBytesReaded);
		if (tBytesReaded != pFile->GetLength())
			throw fcyException("AppFrame::readFileToString", "fcyFileStream::ReadBytes failed.");
	}

	return std::move(tRet);
}

#pragma region 脚本接口
void AppFrame::SetWindowed(bool v)LNOEXCEPT
{
	if (m_iStatus == AppStatus::Initializing)
		m_OptionWindowed = v;
}

void AppFrame::SetFPS(fuInt v)LNOEXCEPT
{
	if (m_iStatus == AppStatus::Initializing)
		m_OptionFPSLimit = v;
}

void AppFrame::SetVsync(bool v)LNOEXCEPT
{
	if (m_iStatus == AppStatus::Initializing)
		m_OptionVsync = v;
}

void AppFrame::SetResolution(fuInt width, fuInt height)LNOEXCEPT
{
	if (m_iStatus == AppStatus::Initializing)
		m_OptionResolution.Set(width, height);
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
		m_OptionTitle = std::move(fcyStringHelper::MultiByteToWideChar(v));
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
			m_pRenderDev->GetBufferHeight());

		// 切换到新的视频选项
		if (FCYOK(m_pRenderDev->SetBufferSize(
			(fuInt)m_OptionResolution.x,
			(fuInt)m_OptionResolution.y,
			m_OptionWindowed,
			m_OptionVsync,
			F2DAALEVEL_NONE)))
		{
			LINFO("视频模式切换成功 (%dx%d Vsync:%b Windowed:%b) -> (%dx%d Vsync:%b Windowed:%b)",
				tOrgOptionResolution.x, tOrgOptionResolution.y, m_OptionVsyncOrg, tOrgOptionWindowed,
				m_OptionResolution.x, m_OptionResolution.y, m_OptionVsync, m_OptionWindowed);

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
	// ! TODO

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
	try
	{
		string tLaunchScript = loadFileAsString(LLAUNCH_SCRIPT);
		if (!SafeCallScript(tLaunchScript.c_str(), tLaunchScript.size(), "launch"))
			return false;
	}
	catch (const fcyException& e)
	{
		LERROR("装载初始化脚本失败 (异常信息'%m' 源'%m')", e.GetDesc(), e.GetSrc());
		return false;
	}

	//////////////////////////////////////// 初始化fancy2d引擎
	// ! TODO

	//////////////////////////////////////// 装载核心脚本
	// ! TODO

	// 调用GameInit

	m_iStatus = AppStatus::Initialized;
	LINFO("初始化成功完成");
	return true;
}

void AppFrame::Shutdown()LNOEXCEPT
{
	if (L)
	{
		lua_close(L);
		L = nullptr;
		LINFO("已卸载Lua虚拟机");
	}

	m_iStatus = AppStatus::Destroyed;
	LINFO("框架销毁");
}

void AppFrame::Run()LNOEXCEPT
{
	LASSERT(m_iStatus == AppStatus::Initialized);
	LINFO("开始执行游戏循环");

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
				L"脚本'%m'中产生未处理的运行时错误: %m",
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
#pragma endregion
