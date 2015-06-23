/// @file LogSystem.h
/// @brief ������־ϵͳ
/// @note ������־ϵͳ����Common.h��������˶�����Common.h
#pragma once
#include <fstream>

namespace LuaSTGPlus
{
	/// @brief ��־����
	enum class LogType
	{
		Information,
		Warning,
		Error
	};

	/// @brief ��־ϵͳ
	class LogSystem
	{
	public:
		/// @brief ��ȡ��־ϵͳʵ��
		static __declspec(noinline) LogSystem& GetInstance();
	private:
		std::fstream m_LogFile;
	public:
		/// @brief ��¼��־
		/// @param type ��־����
		/// @param info ��ʽ���ı�
		__declspec(noinline) void Log(LogType type, const wchar_t* info, ...)throw();
	public:
		LogSystem();
		~LogSystem();
	};
}
