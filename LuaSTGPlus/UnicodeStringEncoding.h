#pragma once
#include "Global.h"

namespace LuaSTGPlus
{
	std::wstring Utf8ToUtf16(const char* p);
	std::string Utf16ToUtf8(const wchar_t* p);
	void Utf8ToUtf16(const char* p, std::wstring& ret);
	void Utf16ToUtf8(const wchar_t* p, std::string& ret);
}
