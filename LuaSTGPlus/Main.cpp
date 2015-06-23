#include "AppFrame.h"
#include "Utility.h"

#define INIT_FAILED_DESC L"�ܱ�Ǹ������ĳЩԭ����δ�ܳɹ���ʼ����\n���������γ�������ϵ���ߡ�\n\n�鿴��־�ļ�'%s'���Ի�ø�����Ϣ��"

using namespace std;
using namespace LuaSTGPlus;

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
#ifdef LDEBUG
	_CrtSetDbgFlag(_CrtSetDbgFlag(_CRTDBG_REPORT_FLAG) | _CRTDBG_LEAK_CHECK_DF);
	// _CrtSetBreakAlloc(5351);
#endif

	// ��ʼ��
	if (!LAPP.Init())
	{
		MessageBox(0, StringFormat(INIT_FAILED_DESC, LLOGFILE).c_str(), L"LuaSTGPlus��ʼ��ʧ��", MB_ICONERROR | MB_OK);
		return -1;
	}

	// ��Ϸѭ��
	LAPP.Run();

	// ����
	LAPP.Shutdown();
	return 0;
}
