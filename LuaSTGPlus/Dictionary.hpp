#pragma once
#include "Global.h"

namespace LuaSTGPlus
{
	/// @brief MPQHash±í
	class MPQHashCryptTable
	{
	public:
		static const MPQHashCryptTable& GetInstance()
		{
			static MPQHashCryptTable s_Table;
			return s_Table;
		}
	private:
		uint32_t m_CryptTable[0x500];
	public:
		uint32_t operator[](uint32_t idx)const
		{
			return m_CryptTable[idx];
		}
	public:
		MPQHashCryptTable()
		{
			uint32_t seed = 0x00100001;
			for (uint32_t index1 = 0; index1 < 0x100; index1++)
			{
				for (uint32_t index2 = index1, i = 0; i < 5; i++, index2 += 0x100)
				{
					unsigned long temp1, temp2;
					seed = (seed * 125 + 3) % 0x2AAAAB;
					temp1 = (seed & 0xFFFF) << 0x10;
					seed = (seed * 125 + 3) % 0x2AAAAB;
					temp2 = (seed & 0xFFFF);
					m_CryptTable[index2] = (temp1 | temp2);
				}
			}
		}
	};

	/// @brief MPQ HashËã·¨
	template <uint32_t dwHashType>
	uint32_t MPQHash(const char* key)
	{
		uint32_t seed1 = 0x7FED7FED, seed2 = 0xEEEEEEEE;
		uint32_t ch;
		while (*key != 0)
		{
			ch = *key++;
			seed1 = MPQHashCryptTable::GetInstance()[(dwHashType << 8) + ch] ^ (seed1 + seed2);
			seed2 = ch + seed1 + seed2 + (seed2 << 5) + 3;
		}
		return seed1;
	}

	/// @brief ×Öµä¼ü
	struct DictionaryKey
	{
		uint32_t HashKey;
		uint32_t Hash1, Hash2;
#ifdef LDEBUG
		std::string Key;
#endif

		bool operator==(const DictionaryKey& right)const
		{
#ifdef LDEBUG
			if (HashKey == right.HashKey && Hash1 == right.Hash1 &&
				Hash2 == right.Hash2)
			{
				LASSERT(Key == right.Key);
				return true;
			}
			return false;
#else
			return (HashKey == right.HashKey && Hash1 == right.Hash1 &&
				Hash2 == right.Hash2);
#endif
		}

		DictionaryKey()
			: HashKey(0), Hash1(0), Hash2(0) {}
		DictionaryKey(fcStr KeyStr)
		{
			HashKey = MPQHash<0>(KeyStr);
			Hash1 = MPQHash<1>(KeyStr);
			Hash2 = MPQHash<2>(KeyStr);
#ifdef LDEBUG
			Key = KeyStr;
#endif
		}
		explicit DictionaryKey(const std::string& KeyStr)
		{
			HashKey = MPQHash<0>(KeyStr.c_str());
			Hash1 = MPQHash<1>(KeyStr.c_str());
			Hash2 = MPQHash<2>(KeyStr.c_str());
#ifdef LDEBUG
			Key = KeyStr;
#endif
		}
		DictionaryKey(const DictionaryKey& org)
			: HashKey(org.HashKey), Hash1(org.Hash1), Hash2(org.Hash2)
		{
#ifdef LDEBUG
			Key = org.Key;
#endif
		}
		DictionaryKey(DictionaryKey&& org)
			: HashKey(org.HashKey), Hash1(org.Hash1), Hash2(org.Hash2)
		{
#ifdef LDEBUG
			Key = std::move(org.Key);
#endif
		}
	};

	struct DictionaryKeyHasher
	{
		size_t operator()(const DictionaryKey& k)const
		{
			return k.HashKey;
		}
	};

	template <typename Value>
	using Dictionary = std::unordered_map<DictionaryKey, Value, DictionaryKeyHasher>;
}
