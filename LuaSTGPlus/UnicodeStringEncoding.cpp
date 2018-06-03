#include "UnicodeStringEncoding.h"

using namespace std;
using namespace LuaSTGPlus;

class Utf8Decoder
{
private:
	// 内部状态
	int      m_iState = 0;
	char32_t m_incpChar = 0;
public:
	bool operator() (uint8_t input, char32_t& output)
	{
		// 状态机
		if (m_iState == 0)
		{
			// 前导字节
			if (input >= 0xFCu && input <= 0xFDu)  // UCS4 1111110x 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx
			{
				m_incpChar = input & 0x01u;
				m_iState = 5;
			}
			else if (input >= 0xF8u && input <= 0xFBu) // UCS4 111110xx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx
			{
				m_incpChar = input & 0x03u;
				m_iState = 4;
			}
			else if (input >= 0xF0u && input <= 0xF7u) // UCS4 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
			{
				m_incpChar = input & 0x07u;
				m_iState = 3;
			}
			else if (input >= 0xE0u && input <= 0xEFu) // 0800~FFFF: 1110xxxx 10xxxxxx 10xxxxxx
			{
				m_incpChar = input & 0x0Fu;
				m_iState = 2;
			}
			else if (input >= 0xC0u && input <= 0xDFu) // 0080~07FF: 110xxxxx 10xxxxxx
			{
				m_incpChar = input & 0x1Fu;
				m_iState = 1;
			}
			else if (/*input >= 0x00u &&*/ input <= 0x7Fu) // 0000~007F: 0xxxxxxx
			{
				m_incpChar = input;
				m_iState = 0;
			}
			else
			{
				m_incpChar = (char32_t)'?';
				m_iState = 0;
			}
		}
		else
		{
			if (input >= 0x80u && input <= 0xBFu)
			{
				m_incpChar = (m_incpChar << 6) | (input & 0x3Fu);
				--m_iState;
			}
			else
			{
				m_incpChar = (char32_t)'?';
				m_iState = 0;
			}
		}

		// 输出
		if (m_iState == 0)
		{
			output = m_incpChar;
			return true;
		}
		else
			return false;
	}
};

class Utf16Decoder
{
private:
	// 内部状态
	uint8_t  m_cBuf[2];
	int      m_iState = 0;
	char16_t m_leadChar = 0;
public:
	bool operator() (uint8_t input, char32_t& output)
	{
		// 状态机
		switch (m_iState)
		{
		case 0:
			m_cBuf[0] = input;
			m_iState = 1;
			return false;
		case 1:
			m_cBuf[1] = input;
			*(uint8_t*)&m_leadChar = m_cBuf[0];
			*((uint8_t*)&m_leadChar + 1) = m_cBuf[1];
			// 检查是否为代理位
			if (m_leadChar >= 0xD800u && m_leadChar < 0xDC00)
			{
				m_leadChar -= 0xD800u;
				m_iState = 2;
				return false;
			}
			else
			{
				output = static_cast<char32_t>(m_leadChar);
				m_iState = 0;
				return true;
			}
		case 2:
			m_cBuf[0] = input;
			m_iState = 3;
			return false;
		case 3:
			m_cBuf[1] = input;
			{
				char16_t tempChar;
				*(uint8_t*)&tempChar = m_cBuf[0];
				*((uint8_t*)&tempChar + 1) = m_cBuf[1];
				// 组合并输出
				if (tempChar >= 0xDC00u && tempChar < 0xE000)
				{
					output = static_cast<char32_t>((m_leadChar << 16) | (tempChar - 0xDC00u));
					m_iState = 0;
					return true;
				}
				else
				{
					output = (char32_t)'?';
					m_iState = 0;
					return true;
				}
			}
		default:
			output = (char32_t)'?';
			m_iState = 0;
			return true;
		}
	}
};

class Utf8Encoder
{
private:
	// 内部状态
	uint8_t m_cBuf[6];
public:
	bool operator() (char32_t input, uint8_t*& output, size_t& size)
	{
		int c;
		uint32_t uinput = (uint32_t)input;

		if (uinput < 0x80u)
			c = 0;
		else if (uinput < 0x800u)
			c = 1;
		else if (uinput < 0x10000u)
			c = 2;
		else if (uinput < 0x200000u)
			c = 3;
		else if (uinput < 0x4000000u)
			c = 4;
		else if (uinput < 0x80000000u)
			c = 5;
		else
		{
			m_cBuf[0] = (uint8_t)'?';
			output = m_cBuf;
			size = 1;
			return true;
		}

		if (c == 0)
			m_cBuf[0] = uinput;
		else
		{
			// 填充字节
			for (int i = c; i > 0; --i)
			{
				m_cBuf[i] = (uinput & 0x3Fu) | 0x80u;
				uinput >>= 6;
			}

			// 填充首字
			m_cBuf[0] = (0xFEu << (6 - c)) | uinput;
		}

		// 写出
		output = m_cBuf;
		size = c + 1;
		return true;
	}
};

class Utf16Encoder
{
private:
	// 内部状态
	char16_t m_cBuf[2];
public:
	bool operator() (char32_t input, uint8_t*& output, size_t& size)
	{
		// 检查是否需要代理位
		if (input >= 0x10000u)
		{
			input -= 0x10000u;
			m_cBuf[0] = static_cast<char16_t>((input >> 10) | 0xD800u);    // 前导字节
			m_cBuf[1] = static_cast<char16_t>((input & 0x3FFu) | 0xDC00u); // 后尾字节
			output = reinterpret_cast<uint8_t*>(m_cBuf);
			size = 2 * sizeof(char16_t);
			return true;
		}
		else
		{
			m_cBuf[0] = static_cast<char16_t>(input);
			output = reinterpret_cast<uint8_t*>(m_cBuf);
			size = 1 * sizeof(char16_t);
			return true;
		}
	}
};

template<typename decoder_t, typename encoder_t, typename char_t = char, typename orgchar_t>
void Convert(const orgchar_t* nullTerminatedString, std::basic_string<char_t>& ret)
{
	decoder_t decoder;
	encoder_t encoder;

	ret.clear();
	char32_t ucs4;
	bool de_finished = true;
	uint8_t* buf;
	size_t size = 0;
	bool en_finished = true;

	const char* p = (const char*)nullTerminatedString;
	while (*nullTerminatedString != 0)
	{
		if (decoder(*p, ucs4))
		{
			de_finished = true;
			if (encoder(ucs4, buf, size))
			{
				en_finished = true;

				if (buf)
				{
					if (size % sizeof(char_t) != 0)
						ret.push_back((char_t)'?');
					else
						ret.append(reinterpret_cast<char_t*>(buf), size / sizeof(char_t));
				}
			}
			else
				en_finished = false;
		}
		else
			de_finished = false;

		++p;
		if ((size_t)p - (size_t)nullTerminatedString == sizeof(orgchar_t))
			++nullTerminatedString;
	}

	if (!de_finished)
		ret.push_back((char_t)'?');
	else if (!en_finished)
		ret.push_back((char_t)'?');
}

std::wstring LuaSTGPlus::Utf8ToUtf16(const char* p)
{
	wstring tRet;
	tRet.reserve(128);
	Convert<Utf8Decoder, Utf16Encoder, wchar_t>(p, tRet);
	return tRet;
}

std::string LuaSTGPlus::Utf16ToUtf8(const wchar_t* p)
{
	string tRet;
	tRet.reserve(128);
	Convert<Utf16Decoder, Utf8Encoder>(p, tRet);
	return tRet;
}

void LuaSTGPlus::Utf8ToUtf16(const char* p, std::wstring& ret)
{
	ret.clear();
	Convert<Utf8Decoder, Utf16Encoder, wchar_t>(p, ret);
}

void LuaSTGPlus::Utf16ToUtf8(const wchar_t* p, std::string& ret)
{
	ret.clear();
	Convert<Utf16Decoder, Utf8Encoder>(p, ret);
}
