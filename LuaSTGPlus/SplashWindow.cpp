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
	SplashWindow* pThis = (SplashWindow*)(LONG_PTR)GetWindowLongPtr(hWnd, GWLP_USERDATA);

	switch (wMsg)
	{
	case WM_CREATE:
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	case WM_PAINT:
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

	// 设置层叠属性
	SetWindowLong(m_hHandle, GWL_EXSTYLE, WS_EX_TOOLWINDOW | WS_EX_LAYERED);

	// 更新窗口数据
	HDC dcDest = GetDC(m_hHandle);
	HDC dcSrc;
	dcSrc = CreateCompatibleDC(dcDest);
	HBITMAP bmpDest;
	bmpDest = CreateCompatibleBitmap(dcDest, m_bkImage->GetWidth(), m_bkImage->GetHeight());
	SelectObject(dcSrc, bmpDest);

	Graphics g(dcSrc);
	g.DrawImage(m_bkImage, 0, 0, m_bkImage->GetWidth(), m_bkImage->GetHeight());

	BLENDFUNCTION tBlend;
	tBlend.BlendOp = 0;
	tBlend.BlendFlags = 0;
	tBlend.AlphaFormat = 1;
	tBlend.SourceConstantAlpha = 255;

	POINT tZero = { 0, 0 };
	SIZE tSize = { static_cast<LONG>(m_bkImage->GetWidth()), static_cast<LONG>(m_bkImage->GetHeight()) };

	UpdateLayeredWindow(m_hHandle, dcDest, &tZero, &tSize, dcSrc, &tZero, 0, &tBlend, ULW_ALPHA);

	DeleteObject(bmpDest);
	DeleteDC(dcSrc);
	g.ReleaseHDC(dcSrc);
	ReleaseDC(m_hHandle, dcDest);

	// 显示窗口
	int scrWidth, scrHeight;
	RECT rect;
	// 获得屏幕尺寸
	scrWidth = GetSystemMetrics(SM_CXSCREEN);
	scrHeight = GetSystemMetrics(SM_CYSCREEN);
	// 获取窗体尺寸
	GetWindowRect(m_hHandle, &rect);
	rect.left = (scrWidth - rect.right) / 2;
	rect.top = (scrHeight - rect.bottom) / 2;
	// 设置窗体位置
	SetWindowPos(m_hHandle, 0, rect.left, rect.top, rect.right, rect.bottom, SWP_SHOWWINDOW);
	SetWindowPos(m_hHandle, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);
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

void SplashWindow::ShowSplashWindow(Gdiplus::Image* bkImage)LNOEXCEPT
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

void SplashWindow::HideSplashWindow()LNOEXCEPT
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
