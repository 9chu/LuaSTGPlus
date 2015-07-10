/// @file Utility.h
/// @brief ʵ�ù���
#pragma once
#include "Global.h"

namespace LuaSTGPlus
{
	/// @brief ��
	class Scope
	{
	private:
		std::function<void()> m_WhatToDo;
	private:
		Scope& operator=(const Scope&);
		Scope(const Scope&);
	public:
		Scope(std::function<void()> exitJob)
			: m_WhatToDo(exitJob) {}
		~Scope()
		{
			m_WhatToDo();
		}
	};

	/// @brief ��ʱ��
	class TimerScope
	{
	private:
		fcyStopWatch m_StopWatch;
		float& m_Out;
	public:
		TimerScope(float& Out)
			: m_Out(Out)
		{
		}
		~TimerScope()
		{
			m_Out = static_cast<float>(m_StopWatch.GetElpased());
		}
	};

	/// @brief �ַ�����ʽ��
	/// @param Format �ַ�����ʽ����֧�־���
	std::string StringFormat(const char* Format, ...)LNOEXCEPT;

	/// @brief �ַ�����ʽ�� va_list�汾
	std::string StringFormatV(const char* Format, va_list vaptr)LNOEXCEPT;

	/// @brief �ַ�����ʽ�� ���ַ�
	/// @param Format �ַ�����ʽ����֧�־���
	std::wstring StringFormat(const wchar_t* Format, ...)LNOEXCEPT;

	/// @brief �ַ�����ʽ�� ���ַ���va_list�汾
	std::wstring StringFormatV(const wchar_t* Format, va_list vaptr)LNOEXCEPT;
}
