#pragma once
#include "Global.h"

#include <thread>
#include <GdiPlus.h>

namespace LuaSTGPlus
{
	/// @brief ‘ÿ»Î¥∞ø⁄
	class SplashWindow
	{
	public:
		static Gdiplus::Image* LoadImageFromMemory(fcData data, size_t len);
		static Gdiplus::Image* LoadImageFromResource(UINT nID, LPCTSTR sTR);
	private:
		static LRESULT WINAPI WndProc(HWND hWnd, UINT wMsg, WPARAM wParam, LPARAM lParam);
	private:
		std::unique_ptr<std::thread> m_SplashWindowThread;

		HWND m_hHandle;
		Gdiplus::Image* m_bkImage = nullptr;
	private:
		void createWindow();
		void threadJob();
	public:
		void ShowSplashWindow(Gdiplus::Image* bkImage=nullptr);
		void HideSplashWindow();
	protected:
		SplashWindow& operator=(const SplashWindow&);
		SplashWindow(const SplashWindow&);
		SplashWindow(SplashWindow&&);
	public:
		SplashWindow();
		~SplashWindow();
	};
}
