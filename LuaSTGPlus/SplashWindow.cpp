#include "SplashWindow.h"

#include "resource.h"

#define CLASSNAME TEXT("LUASTGPLUS_SPLASHWINDOW")

using namespace std;
using namespace Gdiplus;
using namespace LuaSTGPlus;

Gdiplus::Image* SplashWindow::LoadImageFromMemory(fcData data, size_t len)
{
	HGLOBAL hMem = GlobalAlloc(GMEM_FIXED, len);
	fData pData = (fData)GlobalLock(hMem);
	memcpy(pData, data, len);
	IStream* pstm;
	CreateStreamOnHGlobal(hMem, FALSE, &pstm);
	Image* ret = Gdiplus::Image::FromStream(pstm);
	GlobalUnlock(hMem);
	pstm->Release();
	return ret;
}

Gdiplus::Image* SplashWindow::LoadImageFromResource(UINT nID, LPCTSTR sTR)
{
	HRSRC hRsrc = ::FindResource(GetModuleHandle(NULL), MAKEINTRESOURCE(nID), sTR);
	if (!hRsrc)
		return NULL;
	DWORD len = SizeofResource(GetModuleHandle(NULL), hRsrc);
	BYTE* lpRsrc = (BYTE*)LoadResource(GetModuleHandle(NULL), hRsrc);
	if (!lpRsrc)
		return NULL;
	Image* ret = LoadImageFromMemory(lpRsrc, len);
	FreeResource(lpRsrc);
	return ret;
}

LRESULT WINAPI SplashWindow::WndProc(HWND hWnd, UINT wMsg, WPARAM wParam, LPARAM lParam)
{
	int scrWidth, scrHeight;
	RECT rect;
	PAINTSTRUCT ptStr;
	HDC hDC;
	SplashWindow* pThis = (SplashWindow*)(LONG_PTR)GetWindowLongPtr(hWnd, GWLP_USERDATA);

	switch (wMsg)
	{
	case WM_CREATE:
		// 获得屏幕尺寸
		scrWidth = GetSystemMetrics(SM_CXSCREEN);
		scrHeight = GetSystemMetrics(SM_CYSCREEN);
		// 获取窗体尺寸
		GetWindowRect(hWnd, &rect);
		rect.left = (scrWidth - rect.right) / 2;
		rect.top = (scrHeight - rect.bottom) / 2;
		// 设置不显示任务栏图标
		SetWindowLong(hWnd, GWL_EXSTYLE, WS_EX_TOOLWINDOW);
		// 设置窗体位置
		SetWindowPos(hWnd, HWND_TOPMOST, rect.left, rect.top, rect.right, rect.bottom, SWP_SHOWWINDOW);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	case WM_PAINT:
		hDC = BeginPaint(hWnd, &ptStr);
		if (pThis)
		{
			Graphics g(hDC);
			Image* p = pThis->m_bkImage;
			g.DrawImage(p, 0, 0, p->GetWidth(), p->GetHeight());
		}
		EndPaint(hWnd, &ptStr);
		break;
	}
	return DefWindowProc(hWnd, wMsg, wParam, lParam);
}

SplashWindow::SplashWindow()
{
}

SplashWindow::~SplashWindow()
{
	HideSplashWindow();
}

void SplashWindow::createWindow()
{
	struct Wrapper {
		
	};

	WNDCLASS wc = { 0 };
	wc.style = 0;
	wc.lpfnWndProc = WndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = GetModuleHandle(NULL);
	wc.hIcon = NULL;
	wc.hCursor = LoadCursor(NULL, IDC_WAIT);
	wc.hbrBackground = (HBRUSH)GetSysColorBrush(COLOR_WINDOW);
	wc.lpszMenuName = NULL;
	wc.lpszClassName = CLASSNAME;

	RegisterClass(&wc);

	m_hHandle = CreateWindowEx(0,
		CLASSNAME,
		TEXT("Loading..."),
		WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_POPUP,
		0,
		0,
		m_bkImage->GetWidth(),
		m_bkImage->GetHeight(),
		NULL,
		NULL,
		GetModuleHandle(NULL),
		0);
	SetWindowLongPtr(m_hHandle, GWLP_USERDATA, (LONG)(LONG_PTR)this);
}

void SplashWindow::threadJob()
{
	createWindow();

	// 另一个消息循环
	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	UnregisterClass(CLASSNAME, GetModuleHandle(NULL));
}

void SplashWindow::ShowSplashWindow(Gdiplus::Image* bkImage)
{
	struct Wrapper {
		static void ThreadJob(SplashWindow* p)
		{
			p->threadJob();
		}
	};

	if (!m_SplashWindowThread)
	{
		if (bkImage)
			m_bkImage = bkImage->Clone();
		else
			m_bkImage = LoadImageFromResource(IDB_SPLASH, L"PNG");
		
		if (m_bkImage)
			m_SplashWindowThread = make_unique<std::thread>(Wrapper::ThreadJob, this);
	}
}

void SplashWindow::HideSplashWindow()
{
	if (m_SplashWindowThread)
	{
		SendMessage(m_hHandle, WM_CLOSE, 0, 0);
		if (m_SplashWindowThread->joinable())
			m_SplashWindowThread->join();
		delete m_bkImage;
		m_hHandle = nullptr;
		m_bkImage = nullptr;
		m_SplashWindowThread = nullptr;
	}
}
