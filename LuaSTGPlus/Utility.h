/// @file Utility.h
/// @brief 实用工具
#pragma once
#include "Global.h"

namespace LuaSTGPlus
{
	/// @brief 域
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

	/// @brief 计时域
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

	/// @brief 字符串格式化
	/// @param Format 字符串格式，不支持精度
	std::string StringFormat(const char* Format, ...)LNOEXCEPT;

	/// @brief 字符串格式化 va_list版本
	std::string StringFormatV(const char* Format, va_list vaptr)LNOEXCEPT;

	/// @brief 字符串格式化 宽字符
	/// @param Format 字符串格式，不支持精度
	std::wstring StringFormat(const wchar_t* Format, ...)LNOEXCEPT;

	/// @brief 字符串格式化 宽字符、va_list版本
	std::wstring StringFormatV(const wchar_t* Format, va_list vaptr)LNOEXCEPT;

	/// @brief RC4加解密实现
	class RC4
	{
	private:
		uint8_t S[256];
	public:
		void operator()(const uint8_t* input, size_t inputlen, uint8_t* output)
        {
            uint8_t Scpy[256];
            memcpy(Scpy, S, sizeof(S));

            for (size_t i = 0, j = 0; i < inputlen; i++)
            {
                // S盒置换
                size_t i2 = (i + 1) % 256;
                j = (j + Scpy[i2]) % 256;
                std::swap(Scpy[i2], Scpy[j]);
                uint8_t n = Scpy[(Scpy[i2] + Scpy[j]) % 256];

                // 加解密
                *(output + i) = *(input + i) ^ n;
            }
        }
	public:
		RC4(const uint8_t* password, size_t len)
		{
			len = min(len, 256U);

			// 初始化S盒
			for (int i = 0; i < 256; ++i)
				S[i] = i;

			// S盒初始置换
			for (size_t i = 0, j = 0; i < 256; i++)
			{
				j = (j + S[i] + password[i % len]) % 256;
				std::swap(S[i], S[j]);
			}
		}
	};
}
