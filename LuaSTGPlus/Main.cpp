#include "AppFrame.h"
#include "Utility.h"

#define INIT_FAILED_DESC L"很抱歉，由于某些原因框架未能成功初始化。\n若该问题多次出现请联系作者。\n\n查看日志文件'%s'可以获得更多信息。"

using namespace std;
using namespace LuaSTGPlus;

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
#ifdef LDEBUG
	_CrtSetDbgFlag(_CrtSetDbgFlag(_CRTDBG_REPORT_FLAG) | _CRTDBG_LEAK_CHECK_DF);
	// _CrtSetBreakAlloc(5351);
#endif

	// 初始化
	if (!LAPP.Init())
	{
		MessageBox(0, StringFormat(INIT_FAILED_DESC, LLOGFILE).c_str(), L"LuaSTGPlus初始化失败", MB_ICONERROR | MB_OK);
		return -1;
	}

	// 游戏循环
	LAPP.Run();

	// 销毁
	LAPP.Shutdown();
	return 0;
}
