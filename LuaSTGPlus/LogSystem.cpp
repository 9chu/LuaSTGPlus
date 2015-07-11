#include "LogSystem.h"
#include "Utility.h"

using namespace std;
using namespace LuaSTGPlus;

LNOINLINE LogSystem& LogSystem::GetInstance()
{
	static LogSystem s_Instance;
	return s_Instance;
}

LogSystem::LogSystem()
	: m_LogFile(LLOGFILE, ios::out)
{
	if (!m_LogFile)
		LERROR("�޷�������־�ļ�'%s'", LLOGFILE);
}

LogSystem::~LogSystem()
{
}

LNOINLINE void LogSystem::Log(LogType type, const wchar_t* info, ...)throw()
{
	wstring tRet;

	try
	{
		switch (type)
		{
		case LogType::Error:
			tRet = L"[ERRO] ";
			break;
		case LogType::Warning:
			tRet = L"[WARN] ";
			break;
		default:
			tRet = L"[INFO] ";
			break;
		}

		va_list vargs;
		va_start(vargs, info);
		tRet += std::move(StringFormatV(info, vargs));
		va_end(vargs);
		tRet.push_back(L'\n');
	}
	catch (const bad_alloc&)
	{
		OutputDebugString(L"[ERRO] ��¼��־ʱ�����ڴ治�����");
		return;
	}
	
	OutputDebugString(tRet.c_str());
	try
	{
		if (m_LogFile)
		{
			m_LogFile << std::move(fcyStringHelper::WideCharToMultiByte(tRet, CP_UTF8));
			m_LogFile.flush();
		}	
	}
	catch (const bad_alloc&)
	{
		OutputDebugString(L"[ERRO] ��¼��־ʱ�����ڴ治�����");
		return;
	}
}
