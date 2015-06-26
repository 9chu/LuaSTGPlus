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
		LAYER = 9,
		GROUP = 10,
		HIDE = 11,
		BOUND = 12,
		NAVI = 13,
		COLLI = 14,
		STATUS = 15,
		HSCALE = 16,
		VSCALE = 17,
		CLASS = 18,
		A = 19,
		B = 20,
		RECT = 21,
		IMG = 22,
		ANI = 23,
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
			161, 95, 
		};
		
		static const unsigned int s_hashTable2[] =
		{
			138, 90, 
		};
		
		static const unsigned int s_hashTableG[] =
		{
			0, 0, 0, 0, 0, 0, 0, 1, 3, 20, 
			0, 1, 18, 0, 0, 3, 0, 20, 8, 0, 
			15, 12, 10, 0, 11, 22, 0, 0, 1, 12, 
			2, 0, 6, 18, 19, 1, 21, 4, 12, 17, 
			0, 
		};
		
		unsigned int f1 = 0, f2 = 0, len = strlen(key);
		for (unsigned int i = 0; i < 2; ++i)
		{
			unsigned int idx = s_bestIndices[i];
			if (idx < len)
			{
				f1 = (f1 + s_hashTable1[i] * (unsigned int)key[idx]) % 41;
				f2 = (f2 + s_hashTable2[i] * (unsigned int)key[idx]) % 41;
			}
			else
				break;
		}
		
		unsigned int hash = (s_hashTableG[f1] + s_hashTableG[f2]) % 24;
		if (strcmp(s_orgKeyList[hash], key) == 0)
			return static_cast<GameObjectProperty>(hash);
		return GameObjectProperty::_KEY_NOT_FOUND;
	}
}
