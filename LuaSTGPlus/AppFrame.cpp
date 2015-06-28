#include "AppFrame.h"
#include "Utility.h"
#include "LuaWrapper.h"

#include "D3D9.H"  // for SetFog

// ����lua��չ
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
		// ��û�����ٿ�ܣ���ִ������
		Shutdown();
	}
}

#pragma region �ű��ӿ�
void AppFrame::SetWindowed(bool v)LNOEXCEPT
{
	if (m_iStatus == AppStatus::Initializing)
		m_OptionWindowed = v;
	else if (m_iStatus == AppStatus::Running)
		LWARNING("��ͼ������ʱ���Ĵ��ڻ�ģʽ");
}

void AppFrame::SetFPS(fuInt v)LNOEXCEPT
{
	if (m_iStatus == AppStatus::Initializing)
		m_OptionFPSLimit = v;
	else if (m_iStatus == AppStatus::Running)
		LWARNING("��ͼ������ʱ����FPS����");
}

void AppFrame::SetVsync(bool v)LNOEXCEPT
{
	if (m_iStatus == AppStatus::Initializing)
		m_OptionVsync = v;
	else if (m_iStatus == AppStatus::Running)
		LWARNING("��ͼ������ʱ���Ĵ�ֱͬ��ģʽ");
}

void AppFrame::SetResolution(fuInt width, fuInt height)LNOEXCEPT
{
	if (m_iStatus == AppStatus::Initializing)
		m_OptionResolution.Set((float)width, (float)height);
	else if (m_iStatus == AppStatus::Running)
		LWARNING("��ͼ������ʱ���ķֱ���");
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
		LERROR("�޸Ĵ��ڱ���ʱ�޷������ڴ�");
	}
}

LNOINLINE bool AppFrame::UpdateVideoParameters()LNOEXCEPT
{
	if (m_iStatus == AppStatus::Initialized)
	{
		// ��ȡԭʼ��Ƶѡ��
		bool tOrgOptionWindowed = m_pRenderDev->IsWindowed();
		fcyVec2 tOrgOptionResolution = fcyVec2((float)m_pRenderDev->GetBufferWidth(),
			(float)m_pRenderDev->GetBufferHeight());

		// �л����µ���Ƶѡ��
		if (FCYOK(m_pRenderDev->SetBufferSize(
			(fuInt)m_OptionResolution.x,
			(fuInt)m_OptionResolution.y,
			m_OptionWindowed,
			m_OptionVsync,
			F2DAALEVEL_NONE)))
		{
			LINFO("��Ƶģʽ�л��ɹ� (%dx%d Vsync:%b Windowed:%b) -> (%dx%d Vsync:%b Windowed:%b)",
				(int)tOrgOptionResolution.x, (int)tOrgOptionResolution.y, m_OptionVsyncOrg, tOrgOptionWindowed,
				(int)m_OptionResolution.x, (int)m_OptionResolution.y, m_OptionVsync, m_OptionWindowed);

			// �л����ڴ�С
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
			LINFO("��Ƶģʽ�л�ʧ�� (%dx%d Vsync:%b Windowed:%b) -> (%dx%d Vsync:%b Windowed:%b)",
				tOrgOptionResolution.x, tOrgOptionResolution.y, m_OptionVsyncOrg, tOrgOptionWindowed,
				m_OptionResolution.x, m_OptionResolution.y, m_OptionVsync, m_OptionWindowed);
		}
	}
	return false;
}

LNOINLINE void AppFrame::LoadScript(const char* path)LNOEXCEPT
{
	LINFO("װ�ؽű�'%m'", path);
	if (!m_ResourceMgr.LoadFile(path, m_TempBuffer))
	{
		luaL_error(L, "can't load script '%s'", path);
		return;
	}
	if (luaL_loadbuffer(L, m_TempBuffer.data(), m_TempBuffer.size(), luaL_checkstring(L, 1)))
	{
		const char* tDetail = lua_tostring(L, -1);
		LERROR("����ű�'%m'ʧ��: %m", path, tDetail);
		luaL_error(L, "failed to compile '%s': %s", path, tDetail);
		return;
	}
	if (lua_pcall(L, 0, 0, 0))
	{
		const char* tDetail = lua_tostring(L, -1);
		LERROR("ִ�нű�'%m'ʧ��", path);
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
			LERROR("GetLastChar: �ڴ治��");
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
		LERROR("������RenderFunc����ĵط�ִ����Ⱦ");
		return false;
	}

	if (m_GraphType == GraphicsType::Graph2D)
	{
		if (FCYFAILED(m_Graph2D->Begin()))
		{
			LERROR("ִ��f2dGraphics2D::Beginʧ��");
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
			LERROR("ִ��f2dGraphics2D::Endʧ��");
			return false;
		}
	}

	return true;
}

void AppFrame::SetFog(float start, float end, fcyColor color)
{
	if (m_Graph2D->IsInRender())
		m_Graph2D->Flush();

	// ��f2dRenderDevice��ȡ��D3D�豸
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

#pragma region ��ܺ���
bool AppFrame::Init()LNOEXCEPT
{
	LASSERT(m_iStatus == AppStatus::NotInitialized);

	LINFO("��ʼ��ʼ�� �汾: %s", LVERSION);
	m_iStatus = AppStatus::Initializing;
	
	//////////////////////////////////////// Lua��ʼ������
	LINFO("��ʼ��ʼ��Lua����� �汾: %m", LVERSION_LUA);
	L = lua_open();
	if (!L)
	{
		LERROR("�޷���ʼ��Lua�����");
		return false;
	}
	if (0 != luaJIT_setmode(L, 0, LUAJIT_MODE_ENGINE | LUAJIT_MODE_ON))
		LWARNING("�޷�����JITģʽ");

	luaL_openlibs(L);  // �ڽ���
	luaopen_lfs(L);  // �ļ�ϵͳ��
	ColorWrapper::Register(L);  // ��ɫ����
	RandomizerWrapper::Register(L);  // �����������
	BuiltInFunctionWrapper::Register(L);  // �ڽ�������
	
	// Ϊ����ط���ռ�
	LINFO("��ʼ������� ����=%u", LGOBJ_MAXCNT);
	try
	{
		m_GameObjectPool = make_unique<GameObjectPool>(L);
	}
	catch (const bad_alloc&)
	{
		LERROR("�޷�Ϊ����ط����㹻�ڴ�");
		return false;
	}

	// ���������в���
	lua_newtable(L);
	for (int i = 0; i < __argc; ++i)
	{
		lua_pushnumber(L, i + 1);
		lua_pushstring(L, __argv[i]);
		lua_settable(L, -3);
	}
	lua_setglobal(L, "arg");

	//////////////////////////////////////// װ�س�ʼ���ű�
	LINFO("װ�س�ʼ���ű�'%s'", LLAUNCH_SCRIPT);
	if (!m_ResourceMgr.LoadFile(LLAUNCH_SCRIPT, m_TempBuffer))
		return false;
	if (!SafeCallScript(m_TempBuffer.data(), m_TempBuffer.size(), "launch"))
		return false;

	//////////////////////////////////////// ��ʼ��fancy2d����
	LINFO("��ʼ��fancy2d �汾 %d.%d (�ֱ���: %dx%d ��ֱͬ��: %b ���ڻ�: %b)",
		(F2DVERSION & 0xFFFF0000) >> 16, F2DVERSION & 0x0000FFFF,
		(int)m_OptionResolution.x, (int)m_OptionResolution.y, m_OptionVsync, m_OptionWindowed);
	struct : public f2dInitialErrListener
	{
		void OnErr(fuInt TimeTick, fcStr Src, fcStr Desc)
		{
			LERROR("��ʼ��fancy2dʧ�� (�쳣��Ϣ'%m' Դ'%m')", Desc, Src);
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

	// ��ȡ���
	m_pMainWindow = m_pEngine->GetMainWindow();
	m_pRenderer = m_pEngine->GetRenderer();
	m_pRenderDev = m_pRenderer->GetDevice();
	m_pSoundSys = m_pEngine->GetSoundSys();

	// ������Ⱦ��
	if (FCYFAILED(m_pRenderDev->CreateGraphics2D(1024, 2048, &m_Graph2D)))
	{
		LERROR("�޷�������Ⱦ�� (fcyRenderDevice::CreateGraphics2D failed)");
		return false;
	}
	m_Graph2DLastBlendMode = BlendMode::AddAlpha;
	m_Graph2DBlendState = m_Graph2D->GetBlendState();
	m_Graph2DColorBlendState = m_Graph2D->GetColorBlendType();
	m_bRenderStarted = false;

	// ��ʾ���ڣ���ʼ��ʱ��ʾ���ڣ������ڼ��ص�ʱ������������û���
	m_pMainWindow->MoveToCenter();
	m_pMainWindow->SetVisiable(true);

	m_LastChar = 0;
	m_LastKey = 0;
	memset(m_KeyStateMap, 0, sizeof(m_KeyStateMap));

	//////////////////////////////////////// װ�غ��Ľű���ִ��GameInit
	LINFO("װ�غ��Ľű�'%s'", LCORE_SCRIPT);
	if (!m_ResourceMgr.LoadFile(LCORE_SCRIPT, m_TempBuffer))
		return false;
	if (!SafeCallScript(m_TempBuffer.data(), m_TempBuffer.size(), "core.lua"))
		return false;
	if (!SafeCallGlobalFunction(LFUNC_GAMEINIT))
		return false;

	m_iStatus = AppStatus::Initialized;
	LINFO("��ʼ���ɹ����");
	return true;
}

void AppFrame::Shutdown()LNOEXCEPT
{
	m_GameObjectPool = nullptr;
	LINFO("����ն����");

	m_ResourceMgr.ClearAllResource();
	LINFO("�����������Դ");

	m_Graph2D = nullptr;
	m_pEngine = nullptr;
	m_pMainWindow = nullptr;
	m_pRenderer = nullptr;
	m_pRenderDev = nullptr;
	m_pSoundSys = nullptr;
	LINFO("��ж��fancy2d");

	if (L)
	{
		lua_close(L);
		L = nullptr;
		LINFO("��ж��Lua�����");
	}
	m_ResourceMgr.UnloadAllPack();
	LINFO("��ж��������Դ��");

	m_iStatus = AppStatus::Destroyed;
	LINFO("�������");
}

void AppFrame::Run()LNOEXCEPT
{
	LASSERT(m_iStatus == AppStatus::Initialized);
	LINFO("��ʼִ����Ϸѭ��");

	m_pEngine->Run(F2DENGTHREADMODE_MULTITHREAD, m_OptionFPSLimit);

	LINFO("�˳���Ϸѭ��");
}

bool AppFrame::SafeCallScript(const char* source, size_t len, const char* desc)LNOEXCEPT
{
	if (0 != luaL_loadbuffer(L, source, len, desc))
	{
		try
		{
			wstring tErrorInfo = StringFormat(
				L"�ű�'%m'����ʧ��: %m",
				desc,
				lua_tostring(L, -1)
			);

			LERROR("�ű�����%s", tErrorInfo.c_str());
			MessageBox(
				m_pMainWindow ? (HWND)m_pMainWindow->GetHandle() : 0,
				tErrorInfo.c_str(),
				L"LuaSTGPlus�ű�����",
				MB_ICONERROR | MB_OK
			);
		}
		catch (const bad_alloc&)
		{
			LERROR("����д���ű�����ʱ�����ڴ治�����");
		}
		
		lua_pop(L, 1);
		return false;
	}

	if (0 != lua_pcall(L, 0, 0, 0))
	{
		try
		{
			wstring tErrorInfo = StringFormat(
				L"�ű�'%m'�в���δ���������ʱ����:\n\t%m",
				desc,
				lua_tostring(L, -1)
			);

			LERROR("�ű�����%s", tErrorInfo.c_str());
			MessageBox(
				m_pMainWindow ? (HWND)m_pMainWindow->GetHandle() : 0,
				tErrorInfo.c_str(),
				L"LuaSTGPlus�ű�����",
				MB_ICONERROR | MB_OK
			);
		}
		catch (const bad_alloc&)
		{
			LERROR("����д���ű�����ʱ�����ڴ治�����");
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
				L"ִ�к���'%m'ʱ����δ���������ʱ����:\n\t%m",
				name,
				lua_tostring(L, -1)
			);

			LERROR("�ű�����%s", tErrorInfo.c_str());
			MessageBox(
				m_pMainWindow ? (HWND)m_pMainWindow->GetHandle() : 0,
				tErrorInfo.c_str(),
				L"LuaSTGPlus�ű�����",
				MB_ICONERROR | MB_OK
			);
		}
		catch (const bad_alloc&)
		{
			LERROR("����д���ű�����ʱ�����ڴ治�����");
		}

		lua_pop(L, 1);
		return false;
	}

	return true;
}
#pragma endregion

#pragma region ��Ϸѭ��
fBool AppFrame::OnUpdate(fDouble ElapsedTime, f2dFPSController* pFPSController, f2dMsgPump* pMsgPump)
{
	m_fFPS = (float)pFPSController->GetFPS();

	// ������Ϣ
	f2dMsg tMsg;
	while (FCYOK(pMsgPump->GetMsg(&tMsg)))
	{
		switch (tMsg.Type)
		{
		case F2DMSG_WINDOW_ONCLOSE:
			return false;  // �رմ���ʱ����ѭ��
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

	// ִ��֡����
	if (!SafeCallGlobalFunction(LFUNC_FRAME, 0, 1))
		return false;
	bool tAbort = lua_toboolean(L, -1) == 0 ? false : true;
	lua_pop(L, 1);

	return !tAbort;
}

fBool AppFrame::OnRender(fDouble ElapsedTime, f2dFPSController* pFPSController)
{
	m_pRenderDev->Clear();

	// ִ����Ⱦ����
	m_bRenderStarted = true;
	if (!SafeCallGlobalFunction(LFUNC_RENDER, 0, 0))
		m_pEngine->Abort();
	m_bRenderStarted = false;

	return true;
}
#pragma endregion
