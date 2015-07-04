#pragma once
#include "Global.h"

namespace LuaSTGPlus
{
	template <typename T, size_t MaxSize>
	class CirularQueue
	{
	private:
		std::array<T, MaxSize> m_Data;
		size_t m_Front = 0;
		size_t m_Rear = 0;
		size_t m_Count = 0;
	public:
		T& operator[](size_t idx)
		{
			LASSERT(idx < m_Count);
			return m_Data[(idx + m_Front) % MaxSize];
		}
		bool IsEmpty() { return m_Front == m_Rear; }
		bool IsFull() { return (m_Front == (m_Rear + 1) % MaxSize); }
		size_t Size() { return m_Count; }
		size_t Max() { return MaxSize - 1; }
		bool Push(T val)
		{
			if (IsFull())
				return false;
			else
			{
				m_Data[m_Rear] = val;
				m_Rear = (m_Rear + 1) % MaxSize;
				++m_Count;
				return true;
			}
		}
		bool Pop(T& out)
		{
			if (IsEmpty())
				return false;
			else
			{
				out = m_Data[m_Front];
				m_Front = (m_Front + 1) % MaxSize;
				--m_Count;
				return true;
			}
		}
		T& Front()
		{
			LASSERT(!IsEmpty());
			return m_Data[m_Front];
		}
		T& Back()
		{
			LASSERT(!IsEmpty());
			if (m_Rear == 0)
				return m_Data[MaxSize - 1];
			else
				return m_Data[m_Rear - 1];
		}
	};
}
