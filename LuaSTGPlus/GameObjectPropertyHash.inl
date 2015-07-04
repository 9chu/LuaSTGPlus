#pragma once
#include <cstring>

namespace LuaSTGPlus
{
	enum class GameObjectProperty
	{
		X = 0,
		Y = 1,
		DX = 2,
		DY = 3,
		ROT = 4,
		OMIGA = 5,
		TIMER = 6,
		VX = 7,
		VY = 8,
		AX = 9,
		AY = 10,
		LAYER = 11,
		GROUP = 12,
		HIDE = 13,
		BOUND = 14,
		NAVI = 15,
		COLLI = 16,
		STATUS = 17,
		HSCALE = 18,
		VSCALE = 19,
		CLASS = 20,
		A = 21,
		B = 22,
		RECT = 23,
		IMG = 24,
		ANI = 25,
		_KEY_NOT_FOUND = -1
	};

	inline GameObjectProperty GameObjectPropertyHash(const char* key)
	{
		static const char* s_orgKeyList[] =
		{
			"x",
			"y",
			"dx",
			"dy",
			"rot",
			"omiga",
			"timer",
			"vx",
			"vy",
			"ax",
			"ay",
			"layer",
			"group",
			"hide",
			"bound",
			"navi",
			"colli",
			"status",
			"hscale",
			"vscale",
			"class",
			"a",
			"b",
			"rect",
			"img",
			"ani",
		};
		
		static const unsigned int s_bestIndices[] =
		{
			0, 1, 
		};
		
		static const unsigned int s_hashTable1[] =
		{
			191, 127, 
		};
		
		static const unsigned int s_hashTable2[] =
		{
			239, 14, 
		};
		
		static const unsigned int s_hashTableG[] =
		{
			0, 0, 0, 0, 15, 0, 0, 0, 0, 16, 
			13, 0, 19, 0, 9, 0, 12, 0, 0, 5, 
			4, 22, 8, 23, 13, 16, 22, 0, 0, 15, 
			22, 20, 8, 14, 19, 0, 14, 21, 6, 0, 
			20, 17, 0, 
		};
		
		unsigned int f1 = 0, f2 = 0, len = strlen(key);
		for (unsigned int i = 0; i < 2; ++i)
		{
			unsigned int idx = s_bestIndices[i];
			if (idx < len)
			{
				f1 = (f1 + s_hashTable1[i] * (unsigned int)key[idx]) % 43;
				f2 = (f2 + s_hashTable2[i] * (unsigned int)key[idx]) % 43;
			}
			else
				break;
		}
		
		unsigned int hash = (s_hashTableG[f1] + s_hashTableG[f2]) % 26;
		if (strcmp(s_orgKeyList[hash], key) == 0)
			return static_cast<GameObjectProperty>(hash);
		return GameObjectProperty::_KEY_NOT_FOUND;
	}
}
